#include "GroupsManager/ServerGroupsManager.h"
#include "Persistency/ServerPersistency.h"
#include "ServerCommunicationManager.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

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
void *ServerCommunicationManager::staticHandleNewFrontConnectionThread(void *newClientArguments) {
    auto* t = static_cast<ThreadArguments*>(newClientArguments);
    t->communicationManager->handleNewFrontConnectionThread(t);
    return nullptr;
}

void *ServerCommunicationManager::staticNewClientConnectionKeepAliveThread(void *newClientArguments) {
    auto* t = static_cast<KeepAliveThreadArguments*>(newClientArguments);
    t->communicationManager->newClientConnectionKeepAlive(t);
    return nullptr;
}

// MARK: - Instance methods

void ServerCommunicationManager::terminateClientConnection(UserConnection userConnection, ServerGroupsManager *groupsManager) {
    groupsManager->handleUserDisconnection(userConnection);
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
            throw ERROR_FRONT_DISCONNECTED;

        } else {
            throw readOperationResult;
        }
    }
}

void ServerCommunicationManager::sendMessageToClients(Message message, const std::list<UserConnection>& userConnections) {
    for (const UserConnection& userConnection:userConnections) {
        Packet packet = message.asPacket();
        packet.recipient = userConnection.origin;
        int readWriteOperationResult = write(userConnection.frontSocket, &packet, sizeof(Packet));
        if (readWriteOperationResult < 0) {
            throw ERROR_SOCKET_WRITE;
        }
    }
}

void ServerCommunicationManager::handleNewClientConnectionErrors(int errorCode, SocketFD communicationSocket, const string& username, ServerGroupsManager* groupsManager) {
        // TODO: Handle errors somehow D=
        exit(EXIT_FAILURE);
//    if (errorCode == ERROR_FRONT_DISCONNECTED) {
//        exit(EXIT_FAILURE);
//        try {
//            this->terminateClientConnection(communicationSocket, groupsManager);
//        } catch (int errorCode) {
//            if (errorCode == ERROR_SOCKET_CLOSE) {
//                string errorPrefix =
//                        "Error(" + std::to_string(errno) + ") closing socket(" +
//                        std::to_string(communicationSocket) + ")";
//                log(Error, errorPrefix);
//            }
//        }
//    } else if (errorCode == ERROR_MAX_USER_CONNECTIONS_REACHED) {
//        try {
//            this->terminateClientConnection(communicationSocket, groupsManager);
//            this->closeSocketConnection(communicationSocket);
//        } catch (int errorCode) {
//            if (errorCode == ERROR_SOCKET_CLOSE) {
//                string errorPrefix =
//                        "Error(" + std::to_string(errno) + ") closing socket(" + std::to_string(communicationSocket) + ")";
//                log(Error, errorPrefix);
//            }
//        }
//    } else if (errno == EBADF && shouldTerminateClientConnection(communicationSocket)) {
//        // Quando fechamos o socket por timeout vai dar erro de bad file descriptor(errno=9)
//        // Neste caso, não queremos printar o erro
//    } else {
//        string errorPrefix = "Error(" + std::to_string(errno) + ") from socket(" + std::to_string(communicationSocket) + ")";
//        log(Error, errorPrefix);
//    }
}

void *ServerCommunicationManager::handleNewFrontConnectionThread(ThreadArguments *args) {
    SocketFD frontCommunicationSocket = args->socket;

    Packet packet{};
    Client origin{};
    UserConnection userConnection{};

    struct KeepAliveThreadArguments keepAliveThreadArguments;
    keepAliveThreadArguments.communicationManager = this;
    pthread_t keepAliveThread;
    while(true) {
        try {
            packet = readPacketFromSocket(frontCommunicationSocket);
            origin = packet.sender;
            updateLastPongForClient(origin);
            Message message = Message(packet);
            if (packet.type == TypeConnection) {
                userConnection.username = message.username;
                userConnection.origin = origin;
                userConnection.frontSocket = frontCommunicationSocket;
                keepAliveThreadArguments.userConnection = userConnection;

                pthread_create(&keepAliveThread, nullptr, ServerCommunicationManager::staticNewClientConnectionKeepAliveThread, &keepAliveThreadArguments);
                this->groupsManager->handleUserConnection(userConnection, message.groupName);

            } else if (packet.type == TypeMessage) {
                this->groupsManager->sendMessage(message);
            }
        } catch (int errorCode) {
            // FIXME: Support booth client and front errors
            std::cout << "Error: " << errorCode << "while reading from socket: " << frontCommunicationSocket << std::endl;
//            handleNewClientConnectionErrors(errorCode,
//                                            frontCommunicationSocket,
//                                            packet.username,
//                                            args->groupsManager);
            break;
        }
    }

    return nullptr;
}

void ServerCommunicationManager::updateLastPingForClient(Client client) {
    pingAccessControl[client].lock();
    clientsLastPing[client] = now();
    pingAccessControl[client].unlock();
}

void ServerCommunicationManager::updateLastPongForClient(Client client) {
    pongAccessControl[client].lock();
    clientsLastPong[client] = now();
    pongAccessControl[client].unlock();
}

bool ServerCommunicationManager::shouldTerminateClientConnection(Client client) {
    // TODO: Esses ifs estao aqui pq na primeira execucao o ping é 0 ainda e temos um pong da msg de conexao.
    //  Talvez tenha uma forma melhor de resolver, mas é o que temos for now.
    std::time_t lastPing = clientsLastPing[client];
    if (lastPing <= 0) {
        return false;
    }
    std::time_t lastPong = clientsLastPong[client];
    if (lastPing <= 0) {
        return false;
    }

    return (abs(lastPing - lastPong) > TIMEOUT);
}

void *ServerCommunicationManager::newClientConnectionKeepAlive(KeepAliveThreadArguments *args) {
    UserConnection userConnection = args->userConnection;
    std::list<UserConnection> singleUserConnectionList;
    singleUserConnectionList.push_back(userConnection);

    // Ensures ping is reset when repeating the socket
    updateLastPingForClient(userConnection.origin);

    while (true) {
        sleep(TIMEOUT);
        try {
            bool isConnectionValid = this->groupsManager->isConnectionValid(userConnection);
            if (!isConnectionValid) {
                // Client desconectou no intervalo do timeout.
                std::cout << "Client " << userConnection.origin.frontID << "-" << userConnection.origin.clientSocket << " already left" << std::endl;
                break;
            }

            if (shouldTerminateClientConnection(userConnection.origin)) {
                terminateClientConnection(userConnection, this->groupsManager);
                break;
            } else {
                std::cout << "Pinging client " << userConnection.origin.frontID << "-" << userConnection.origin.clientSocket << std::endl;
                updateLastPingForClient(userConnection.origin);
                Message keepAliveMessage = Message::keepAliveWithUsername(userConnection.username, userConnection.origin, clientNotSet);
                sendMessageToClients(keepAliveMessage, singleUserConnectionList);
            }
        } catch (int zapError) {
            string errorPrefix = "Error(" + std::to_string(errno) + ", " + std::to_string(zapError) + ") from client(" + std::to_string(userConnection.origin.frontID) + "-" + std::to_string(userConnection.origin.clientSocket) + ")";
            log(Error, errorPrefix);
            break;
        }
    }

    return nullptr;
}

int ServerCommunicationManager::connectToFront(const SocketConnectionInfo& connectionInfo) {

    SocketFD socketFD;
    struct sockaddr_in front_addr{};
    struct hostent *front;

    front = gethostbyname(connectionInfo.ipAddress.c_str());
    if (front == nullptr) {
        string errorPrefix = "Error no such host '" + connectionInfo.ipAddress + "'";
        perror(errorPrefix.c_str());
        return ERROR_INVALID_HOST;
    }

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        string errorPrefix = "Error(" + std::to_string(socketFD) + ") opening socket";
        perror(errorPrefix.c_str());
        return ERROR_SOCKET_CREATION;
    }

    front_addr.sin_family = AF_INET;
    front_addr.sin_port = htons(connectionInfo.port);
    front_addr.sin_addr = *((struct in_addr *)front->h_addr);

    int connectionResult = connect(socketFD, (struct sockaddr *) &front_addr, sizeof(front_addr));
    if (connectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(connectionResult) + ") connecting";
        perror(errorPrefix.c_str());
        return ERROR_SOCKET_CONNECTION;
    }

    return socketFD;
}

int ServerCommunicationManager::startServer(int loadMessageCount) {
    groupsManager = new ServerGroupsManager(loadMessageCount, this);

    std::list<SocketConnectionInfo> connections;
    // TODO: Load fronts from config file
    SocketConnectionInfo hardCodedconnectionInfo;
    hardCodedconnectionInfo.ipAddress = "localhost";
    hardCodedconnectionInfo.port = PORT_FRONT_SERVER;
    connections.push_back(hardCodedconnectionInfo);

    SocketFD communicationSocket;

    for (const SocketConnectionInfo &connectionInfo: connections) {
        communicationSocket = connectToFront(connectionInfo);
        if (communicationSocket <= 0) {
            string errorPrefix = "Error(" + std::to_string(communicationSocket) + ") connecting server to:\nfront: " + connectionInfo.ipAddress + ":" + std::to_string(connectionInfo.port);
            perror(errorPrefix.c_str());
            return communicationSocket;
        }

        std::cout << "Successful connection to:" << std::endl;
        std::cout << "front: " << connectionInfo.ipAddress << ":" << connectionInfo.port << std::endl;
        std::cout << "communicationSocket: " << communicationSocket << std::endl;

        struct ThreadArguments args;
        args.socket = communicationSocket;
        args.communicationManager = this;

        pthread_t connectionThread;

        pthread_create(&connectionThread, nullptr, ServerCommunicationManager::staticHandleNewFrontConnectionThread, &args);
    }

    // TODO: Probably would be more correct to wait all threads? Not sure
    while (true);

    return 0;
}
