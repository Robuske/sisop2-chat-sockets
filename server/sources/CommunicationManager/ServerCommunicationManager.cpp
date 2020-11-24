#include "GroupsManager/ServerGroupsManager.h"
#include "Persistency/ServerPersistency.h"
#include "ServerCommunicationManager.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <netdb.h>
#define ERROR_SEND_BACKUP_MESSAGE -36

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

void *ServerCommunicationManager::staticHandleNewServerConnectionThread(void *newClientArguments) {
    auto* t = static_cast<ThreadArguments*>(newClientArguments);
    t->communicationManager->handleNewServerConnectionThread(t);
    return nullptr;
}

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
    if (!this->electionManager.isCoordinator()) {
        return;
    }

    for (const UserConnection& userConnection:userConnections) {
        Packet packet = message.asPacket();
        packet.recipient = userConnection.origin;
        int readWriteOperationResult = write(userConnection.frontSocket, &packet, sizeof(Packet));
        if (readWriteOperationResult < 0) {
            throw ERROR_SOCKET_WRITE;
        }
    }
}

void ServerCommunicationManager::handleNewClientConnectionErrors(int errorCode, SocketFD frontCommunicationSocket, const string &username) {
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

void *ServerCommunicationManager::handleNewServerConnectionThread(ThreadArguments *args) {
    SocketFD communicationSocket = args->socket;

    Packet packet{};

    while(true) {
        try {
            packet = readPacketFromSocket(communicationSocket);
            std::cout << "PACKET TEXT "<< packet.text << std::endl;
            Message message = Message(packet);

            switch(packet.type) {
                case TypeConnection:
                    break;

                case TypeMessage:
                    break;

                case TypeElection:
                    this->electionManager.didReceiveElectionMessage(packet.text);
                    break;

                case TypeElected:
                    this->electionManager.didReceiveElectedMessage(packet.text);
                    this->setupMainConnection();
                    return nullptr;

            }

        } catch (int errorCode) {

            if(errorCode == ERROR_FRONT_DISCONNECTED) {
                std::cout << "CONNECTION CLOSED" << std::endl;
            } else {
                std::cout << "CONNECTION CLOSED WITH ERROR " << errorCode << std::endl;
                perror("READ ERROR");
                exit(EXIT_FAILURE);
            }

            break;
        }
    }

    return nullptr;
}

void *ServerCommunicationManager::handleNewFrontConnectionThread(ThreadArguments *args) {
    SocketFD frontCommunicationSocket = args->socket;

    Packet packet{};
    Client origin{};
    UserConnection userConnection{};

    struct KeepAliveThreadArguments keepAliveThreadArguments;
    keepAliveThreadArguments.communicationManager = this;
    pthread_t keepAliveThread;
    while (true) {
        try {
            packet = readPacketFromSocket(frontCommunicationSocket);
            logPacket(packet);

            origin = packet.sender;
            updateLastPongForClient(origin);
            Message message = Message(packet);
            userConnection.username = message.username;
            userConnection.origin = origin;
            userConnection.frontSocket = frontCommunicationSocket;

            if (this->electionManager.isCoordinator()) {
                this->forwardPacketToBackups(packet);
            }

            switch (packet.type) {
                case TypeConnection:
                    keepAliveThreadArguments.userConnection = userConnection;

                    this->groupsManager->handleUserConnection(userConnection, message.groupName);
                    // Handles user connection first to not create a keep alive thread if the connection is refused
                    pthread_create(&keepAliveThread, nullptr, ServerCommunicationManager::staticNewClientConnectionKeepAliveThread, &keepAliveThreadArguments);
                    break;

                case TypeDisconnection:
                    this->terminateClientConnection(userConnection, groupsManager);
                    break;

                case TypeMessage:
                    this->groupsManager->sendMessage(message);
                    break;

                default:
                    break;
            }

        } catch (int errorCode) {
            std::cout << "Error: " << errorCode << " while reading from socket: " << frontCommunicationSocket << std::endl;

            if (errorCode == ERROR_MAX_USER_CONNECTIONS_REACHED || errorCode == ERROR_GROUP_NOT_FOUND) {
                // The connection was denied or is trying to remove a connection that was already removed
                continue;
            }

            if (errorCode == ERROR_SEND_BACKUP_MESSAGE) {
                // Should we handle it differently?
                exit(EXIT_FAILURE);
            }

            // FIXME: Support booth client and front errors
            handleNewClientConnectionErrors(errorCode,
                                            frontCommunicationSocket,
                                            packet.username);
            break;
        }
    }

    return nullptr;
}

void ServerCommunicationManager::forwardPacketToBackups(Packet packet) {
    int result = 0;
    for (const SocketFD &backupServer: this->backupServers) {
        result = write(backupServer, &packet, sizeof(Packet));
        if (result < 0) {
            throw ERROR_SEND_BACKUP_MESSAGE;
        }
    }
}

void ServerCommunicationManager::startElection() {
    this->electionManager.setupElection();
    Message firstElectionMessage = this->electionManager.getFirstCandidateDefaultMessage();
    std::cout << "Começou a eleição " << firstElectionMessage.text << std::endl;
    this->electionManager.sendMessageForCurrentElection(firstElectionMessage);
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
            std::cout << "Timestamp " << dateStringFromTimestamp(now()) << std::endl;
            bool isConnectionValid = this->groupsManager->isConnectionValid(userConnection);
            if (!isConnectionValid) {
                // Client desconectou no intervalo do timeout.
                std::cout << "Client " << userConnection.username << ":" << userConnection.origin.frontID << "-" << userConnection.origin.clientSocket << " already left" << std::endl;
                break;
            }

            if (shouldTerminateClientConnection(userConnection.origin)) {
                std::cout << "Client " << userConnection.username << ":" << userConnection.origin.frontID << "-" << userConnection.origin.clientSocket << " timed out" << std::endl;
                terminateClientConnection(userConnection, this->groupsManager);
                break;
            } else {
                std::cout << "Pinging client " << userConnection.username << ":" << userConnection.origin.frontID << "-" << userConnection.origin.clientSocket << std::endl;
                updateLastPingForClient(userConnection.origin);
                Message keepAliveMessage = Message::keepAliveWithUsername(userConnection.username, userConnection.origin, clientNotSet);
                sendMessageToClients(keepAliveMessage, singleUserConnectionList);
            }
        } catch (int zapError) {
            string errorPrefix = "Error(" + std::to_string(errno) + ", " + std::to_string(zapError) + ") from client(" + userConnection.username + ":" + std::to_string(userConnection.origin.frontID) + "-" + std::to_string(userConnection.origin.clientSocket) + ")";
            log(Error, errorPrefix);
            break;
        }
    }

    return nullptr;
}


int ServerCommunicationManager::performConnectionTo(const SocketConnectionInfo& connectionInfo) {

    SocketFD socketFD;
    struct sockaddr_in socketAddress{};
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

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(connectionInfo.port);
    socketAddress.sin_addr = *((struct in_addr *)front->h_addr);

    int connectionResult = connect(socketFD, (struct sockaddr *) &socketAddress, sizeof(socketAddress));
    if (connectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(connectionResult) + ") connecting";
        perror(errorPrefix.c_str());
        return ERROR_SOCKET_CONNECTION;
    }

    return socketFD;
}


SocketFD ServerCommunicationManager::openServerToServerPort(unsigned short port) {
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

int ServerCommunicationManager::setupServerToServerConnection(SocketConnectionInfo connectionInfo) {

    SocketFD communicationSocketFD, connectionSocketFDResult;
    connectionSocketFDResult = this->openServerToServerPort(connectionInfo.port);
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

    struct sockaddr_in clientAddress;
    socklen_t clientSocketLength;
    while (true) {
        clientSocketLength = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &clientAddress, &clientSocketLength)) == -1)
            return ERROR_SOCKET_ACCEPT_CONNECTION;

        resetContinuousBufferFor(communicationSocketFD);

        struct ThreadArguments args;
        args.socket = communicationSocketFD;
        args.communicationManager = this;

        if (this->electionManager.isCoordinator()) {
            this->backupServers.push_back(communicationSocketFD);
        } else {
            // Não estamos usando o id da thread depois, só estamos passando um valor porque usar nullptr no primeiro parâmetro da um warning
            pthread_t keepAliveThread, connectionThread;

            // pthread_create(&keepAliveThread, nullptr, ServerCommunicationManager::staticNewClientConnectionKeepAliveThread, &args);
            pthread_create(&connectionThread, nullptr, ServerCommunicationManager::staticHandleNewServerConnectionThread, &args);
        }
    }
}

void ServerCommunicationManager::setupFronts() {
    std::list<SocketConnectionInfo> connections;
    // TODO: Load fronts from config file
    SocketConnectionInfo hardCodedConnectionInfo;
    hardCodedConnectionInfo.ipAddress = "localhost";
    hardCodedConnectionInfo.port = PORT_FRONT_SERVER;
    connections.push_back(hardCodedConnectionInfo);

    SocketFD communicationSocket;
    for (const SocketConnectionInfo &connectionInfo: connections) {
        communicationSocket = performConnectionTo(connectionInfo);
        if (communicationSocket <= 0) {
            string errorPrefix = "Error(" + std::to_string(communicationSocket) + ") connecting server to: " + connectionInfo.ipAddress + ":" + std::to_string(connectionInfo.port);
            perror(errorPrefix.c_str());
            throw communicationSocket;
        }

        std::cout << "Successful connection to" << connectionInfo.ipAddress << ":" << connectionInfo.port << std::endl;
        std::cout << "\tCommunicationSocket: " << communicationSocket << std::endl;

        struct ThreadArguments args;
        args.socket = communicationSocket;
        args.communicationManager = this;

        pthread_t connectionThread;
        pthread_create(&connectionThread, nullptr, ServerCommunicationManager::staticHandleNewFrontConnectionThread, &args);
    }
}

void ServerCommunicationManager::setupAsBackup() {
    std::cout << "Setup as backup: " << this->electionManager.serverID << std::endl;
    SocketConnectionInfo coordinatorConnectionInfo = this->electionManager.loadCoordinatorConnectionInfo();

    SocketFD communicationSocket = performConnectionTo(coordinatorConnectionInfo);
    if (communicationSocket <= 0) {
        string errorPrefix = "Error(" + std::to_string(communicationSocket) + ") connecting server to: " + coordinatorConnectionInfo.ipAddress + ":" + std::to_string(coordinatorConnectionInfo.port);
        perror(errorPrefix.c_str());
        throw communicationSocket;
    }

    std::cout << "Successful connection to " << coordinatorConnectionInfo.ipAddress << ":" << coordinatorConnectionInfo.port << std::endl;
    std::cout << "\tCommunicationSocket: " << communicationSocket << std::endl;

    struct ThreadArguments args;
    args.socket = communicationSocket;
    args.communicationManager = this;

    pthread_t connectionThread;
    pthread_create(&connectionThread, nullptr, ServerCommunicationManager::staticHandleNewFrontConnectionThread, &args);
}

void ServerCommunicationManager::setupMainConnection() {

    if(this->electionManager.isCoordinator()) {
        std::cout << "WILL CONNECT TO FRONT" << std::endl;
        setupFronts();
    } else {
        // connect to coordinator
        std::cout << "WILL CONNECT TO COORDINATOR" << std::endl;
        setupAsBackup();
    }
}

int ServerCommunicationManager::startServer(int loadMessageCount, int serverID) {
    groupsManager = new ServerGroupsManager(loadMessageCount, this);
    this->electionManager.setupElectionManager(serverID, this);

    this->setupMainConnection();

    SocketConnectionInfo serverConnectionInfo = this->electionManager.searchConnectionInfoForServerID(serverID);
    auto serverConnectionResult = this->setupServerToServerConnection(serverConnectionInfo);
    return serverConnectionResult;
}
