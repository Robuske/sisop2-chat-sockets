#include "GroupsManager/ServerGroupsManager.h"
#include "Persistency/ServerPersistency.h"
#include "ServerCommunicationManager.h"
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <netdb.h>

enum eLogLevel { Info, Debug, Error } typedef LogLevel;
void log(LogLevel logLevel, const string& msg) {
    switch (logLevel) {
        case Info:
            std::cout << "INFO:: " << msg << std::endl;
            break;

        case Debug:
            std::cout << "DEBUG:: " << msg << std::endl;
            break;

        case Error:
            perror(msg.c_str());
            break;
    }
}


// MARK: - Static methods
void *ServerCommunicationManager::staticHandleNewClientConnectionThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleNewClientConnection(t);
    return nullptr;
}

void *ServerCommunicationManager::staticNewClientConnectionKeepAliveThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->newClientConnectionKeepAlive(t);
    return nullptr;
}


void ServerCommunicationManager::closeSocketConnection(SocketFD socketFileDescriptor) {
    int closeReturn = close(socketFileDescriptor);
    if (closeReturn < 0) {
        throw ERROR_SOCKET_CLOSE;
    }
}

void ServerCommunicationManager::terminateClientConnection(SocketFD socketFileDescriptor, string username, ServerGroupsManager* groupsManager) {
    this->closeSocketConnection(socketFileDescriptor);
    groupsManager->handleUserDisconnection(socketFileDescriptor, username);
}

void ServerCommunicationManager::resetContinuousBufferFor(SocketFD socket) {
    continuousBufferAccessControl[socket].lock();
    continuousBuffers[socket].clear();
    continuousBufferAccessControl[socket].unlock();
}

Packet ServerCommunicationManager::readPacketFromSocket(SocketFD communicationSocket) {
    try {
        // Critical section
        continuousBufferAccessControl[communicationSocket].lock();
        ContinuousBuffer continuousBuffer = continuousBuffers[communicationSocket];
        Packet packet = continuousBufferRead(communicationSocket, continuousBuffer);
        continuousBufferAccessControl[communicationSocket].unlock();
        return packet;

    } catch (int readOperationResult) {
        continuousBufferAccessControl[communicationSocket].unlock();

        if (readOperationResult == 0) {
            throw ERROR_CLIENT_DISCONNECTED;

        } else {
            throw readOperationResult;
        }
    }
}

void ServerCommunicationManager::sendMessageToClients(Message message, const std::list<UserConnection>& userConnections) {
    for (const UserConnection& userConnection:userConnections) {
        Packet packet = message.asPacket();
        int readWriteOperationResult = write(userConnection.socket, &packet, sizeof(Packet));
        if (readWriteOperationResult < 0) {
            throw ERROR_SOCKET_WRITE;
        }
    }
}

void ServerCommunicationManager::handleNewClientConnectionErrors(int errorCode, SocketFD communicationSocket, const string& username, ServerGroupsManager* groupsManager) {
    if (errorCode == ERROR_CLIENT_DISCONNECTED) {
        try {
            this->terminateClientConnection(communicationSocket, username, groupsManager);
        } catch (int errorCode) {
            if (errorCode == ERROR_SOCKET_CLOSE) {
                string errorPrefix =
                        "Error(" + std::to_string(errno) + ") closing socket(" +
                        std::to_string(communicationSocket) + ")";
                log(Error, errorPrefix);
            }
        }
    } else if (errorCode == ERROR_MAX_USER_CONNECTIONS_REACHED) {
        try {
            this->closeSocketConnection(communicationSocket);
        } catch (int errorCode) {
            if (errorCode == ERROR_SOCKET_CLOSE) {
                string errorPrefix =
                        "Error(" + std::to_string(errno) + ") closing socket(" +
                        std::to_string(communicationSocket) + ")";
                log(Error, errorPrefix);
            }
        }
    } else if (errno == EBADF && shouldTerminateSocketConnection(communicationSocket)) {
        // Quando fechamos o socket por timeout vai dar erro de bad file descriptor(errno=9)
        // Neste caso, não queremos printar o erro
    } else {
        string errorPrefix =
                "Error(" + std::to_string(errno) + ") from socket(" + std::to_string(communicationSocket) + ")";
        log(Error, errorPrefix);
    }
}

void *ServerCommunicationManager::handleNewClientConnection(HandleNewClientArguments *args) {
    SocketFD communicationSocket = args->newClientSocket;

    Packet packet{};

    while(true) {
        try {
            packet = readPacketFromSocket(communicationSocket);
            std::cout << "PACKET TEXT "<<packet.text << std::endl;
            updateLastPongForSocket(communicationSocket);
            Message message = Message(packet);
            if (packet.type == TypeConnection) {
//                args->groupsManager->handleUserConnection(message.username,
//                                                          communicationSocket,
//                                                          message.groupName);
                this->startTestElection();
            } else if (packet.type == TypeMessage) {
                args->groupsManager->sendMessage(message);
            } else if (packet.type == TypeElection) {

                this->electionManager.didReceiveElectionMessage(packet.text);
            } else if (packet.type == TypeElected) {
                this->electionManager.didReceiveElectedMessage(packet.text);
            }

        } catch (int errorCode) {
            handleNewClientConnectionErrors(errorCode,
                                            communicationSocket,
                                            packet.username,
                                            args->groupsManager);
            break;
        }
    }

    return nullptr;
}

void ServerCommunicationManager::startTestElection() {
    this->electionManager.setupElection();
    Message firstElectionMessage = this->electionManager.getFirstCandidateDefaultMessage();
    std::cout<<"Começou a eleicao "<< firstElectionMessage.text << std::endl;
    auto writeResult = this->electionManager.sendMessageForCurrentElection(firstElectionMessage);
    std::cout<<"Write election message result" << writeResult << std::endl;
}


void ServerCommunicationManager::updateLastPingForSocket(SocketFD socket) {
    pingAccessControl[socket].lock();
    socketsLastPing[socket] = now();
    pingAccessControl[socket].unlock();
}

void ServerCommunicationManager::updateLastPongForSocket(SocketFD socket) {
    pongAccessControl[socket].lock();
    socketsLastPong[socket] = now();
    pongAccessControl[socket].unlock();
}

bool ServerCommunicationManager::shouldTerminateSocketConnection(SocketFD socket) {
    // TODO: Esses ifs estao aqui pq na primeira execucao o ping é 0 ainda e temos um pong da msg de conexao.
    //  Talvez tenha uma forma melhor de resolver, mas é o que temos for now.
    std::time_t lastPing = socketsLastPing[socket];
    if (lastPing <= 0) {
        return false;
    }
    std::time_t lastPong = socketsLastPong[socket];
    if (lastPing <= 0) {
        return false;
    }

    return (abs(lastPing - lastPong) > TIMEOUT);
}

void *ServerCommunicationManager::newClientConnectionKeepAlive(HandleNewClientArguments *args) {
    UserConnection userConnection;
    userConnection.socket = args->newClientSocket;
    std::list<UserConnection> singleUserConnectionList;
    singleUserConnectionList.push_back(userConnection);

    // Ensures ping is reset when repeating the socket
    updateLastPingForSocket(userConnection.socket);

    while (true) {
        sleep(TIMEOUT);
        try {
            string username = args->groupsManager->getUsernameForSocket(userConnection.socket);
            if (username.empty()) {
                // Client desconectou no intervalo do timeout.
                std::cout << "Socket " + std::to_string(userConnection.socket) + " already left" << std::endl;
                break;
            }

            if (shouldTerminateSocketConnection(userConnection.socket)) {
                terminateClientConnection(userConnection.socket, username, args->groupsManager);
                break;
            } else {
                std::cout << "Pinging socket " << std::to_string(userConnection.socket) << std::endl;
                updateLastPingForSocket(userConnection.socket);
                Message keepAliveMessage = Message::keepAliveWithUsername(username);
                sendMessageToClients(keepAliveMessage, singleUserConnectionList);
            }
        } catch (int zapError) {
            string errorPrefix = "Error(" + std::to_string(errno) + ", " + std::to_string(zapError) + ") from socket(" + std::to_string(userConnection.socket) + ")";
            log(Error, errorPrefix);
            break;
        }
    }

    return nullptr;
}

SocketFD ServerCommunicationManager::setupServerSocket(unsigned short port) {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(connectionSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        return ERROR_SOCKET_BINDING;

    //    The backlog argument defines the maximum length to which the queue of
    //    pending connections for sockfd may grow.  If a connection request
    //    arrives when the queue is full, the client may receive an error with
    //    an indication of ECONNREFUSED or, if the underlying protocol supports
    //    retransmission, the request may be ignored so that a later reattempt
    //    at connection succeeds.
    int backlog = 100;
    listen(connectionSocketFD, backlog);

    return connectionSocketFD;
}

int ServerCommunicationManager::startServer(int loadMessageCount, int myID, int coordinatorID,  unsigned short port) {
    ServerGroupsManager groupsManager = ServerGroupsManager(loadMessageCount, this);

    SocketFD communicationSocketFD, connectionSocketFDResult;
    connectionSocketFDResult = this->setupServerSocket(port);
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

    this->electionManager.port = port;

    this->electionManager.loadAvailableServersConnections();
    this->electionManager.setMyID(myID);
    this->electionManager.setElected(coordinatorID);

    struct sockaddr_in clientAddress;
    socklen_t clientSocketLength;
    while(true) {
        clientSocketLength = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &clientAddress, &clientSocketLength)) == -1)
            return ERROR_SOCKET_ACCEPT_CONNECTION;

        resetContinuousBufferFor(communicationSocketFD);

        struct HandleNewClientArguments args;
        args.newClientSocket = communicationSocketFD;
        args.communicationManager = this;
        args.groupsManager = &groupsManager;

        // Não estamos usando o id da thread depois, só estamos passando um valor porque usar nullptr no primeiro parâmetro da um warning
        pthread_t keepAliveThread, connectionThread;
        // TODO: Descomentar a linha abaixo
        pthread_create(&keepAliveThread, nullptr, ServerCommunicationManager::staticNewClientConnectionKeepAliveThread, &args);
        pthread_create(&connectionThread, nullptr, ServerCommunicationManager::staticHandleNewClientConnectionThread, &args);
    }

    return 0;
}
