#include "GroupsManager/ServerGroupsManager.h"
#include "Persistency/ServerPersistency.h"
#include "ServerCommunicationManager.h"
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctime>

std::time_t now() {
    std::time_t now = std::time(nullptr);
    return now;
}

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
void *ServerCommunicationManager::staticHandleNewClientConnection(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleNewClientConnection(t);
    return nullptr;
}

void *ServerCommunicationManager::staticNewClientConnectionKeepAlive(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->newClientConnectionKeepAlive(t);
    return nullptr;
}

// MARK: - Instance methods

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

// TODO: readPacketHeaderFromSocket and readPacketFromSocket can be refactored, the only difference is the type of what we're reading.
//  Maybe we can move this to shared so the client can also use this code
//  I tried the code below but the server was crashing when a user connects:
//void readSocket(SocketFD socket, size_t length, void* dst) {
//    int readOperationResult = read(socket, dst, length);
//    if (readOperationResult == 0) {
//        throw ERROR_CLIENT_DISCONNECTED;
//    } else if (readOperationResult < 0) {
//        throw readOperationResult;
//    }
//}
//
//PacketHeader ServerCommunicationManager::readPacketHeaderFromSocket(SocketFD communicationSocket) {
//    PacketHeader packetHeader;
//    readSocket(communicationSocket, sizeof(PacketHeader), &packetHeader);
//}
//
//Packet ServerCommunicationManager::readPacketFromSocket(SocketFD communicationSocket, int packetSize) {
//    Packet packet;
//    readSocket(communicationSocket, packetSize, &packet);
//}

//PacketHeader ServerCommunicationManager::readPacketHeaderFromSocket(SocketFD communicationSocket) {
//    PacketHeader packetHeader;
//    int readOperationResult = read(communicationSocket, &packetHeader, sizeof(PacketHeader));
//    if (readOperationResult == 0) {
//        throw ERROR_CLIENT_DISCONNECTED;
//    } else if (readOperationResult < 0) {
//        throw readOperationResult;
//    } else {
//        return packetHeader;
//    }
//}

Packet ServerCommunicationManager::readPacketFromSocket(SocketFD communicationSocket, int packetSize) {
    Packet packet;
    int readOperationResult = read(communicationSocket, &packet, packetSize);
    if (readOperationResult == 0) {
        throw ERROR_CLIENT_DISCONNECTED;
    } else if (readOperationResult < 0) {
        throw readOperationResult;
    } else {
        return packet;
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

typedef std::map<SocketFD, std::time_t> KeepAlive;
KeepAlive socketsLastPing;
KeepAlive socketsLastPong;

typedef std::map<SocketFD, std::mutex> KeepAliveAccessControl;
KeepAliveAccessControl pingAccessControl;
KeepAliveAccessControl pongAccessControl;

void updateLastPingForSocket(SocketFD socket) {
    pingAccessControl[socket].lock();
    socketsLastPing[socket] = now();
    pingAccessControl[socket].unlock();
}

void updateLastPongForSocket(SocketFD socket) {
    pongAccessControl[socket].lock();
    socketsLastPong[socket] = now();
    pongAccessControl[socket].unlock();
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

    Packet packet;
    while(true) {
        try {
            packet = readPacketFromSocket(communicationSocket, sizeof(Packet));
            updateLastPongForSocket(communicationSocket);
            Message message = Message(packet);
            if (packet.type == TypeConnection) {
                args->groupsManager->handleUserConnection(message.username,
                                                          communicationSocket,
                                                          message.groupName);
            } else if (packet.type == TypeMessage) {
                args->groupsManager->sendMessage(message);
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
    Message keepAliveMessage = Message(TypeKeepAlive);
    while (true) {
        sleep(TIMEOUT);
        try {
            if (shouldTerminateSocketConnection(userConnection.socket)) {
                // TODO: Get the username
                string username = "WHERE WE CAN GET THE USERNAME FROM?";
                terminateClientConnection(userConnection.socket, username, args->groupsManager);
                break;
            } else {
                std::cout << "Pinging socket " << std::to_string(userConnection.socket) << std::endl;
                updateLastPingForSocket(userConnection.socket);
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

SocketFD ServerCommunicationManager::setupServerSocket() {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
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

int ServerCommunicationManager::startServer(int loadMessageCount) {
    ServerGroupsManager groupsManager = ServerGroupsManager(loadMessageCount, this);

    SocketFD communicationSocketFD, connectionSocketFDResult;
    connectionSocketFDResult = this->setupServerSocket();
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

    // TODO: Server não deveria ter isso, só o GroupsManager deveria controlar isso, mas no momento estamos usando pra desconectar users
    pthread_t clientConnections[10];
    int threadIndex = 0;
    struct sockaddr_in clientAddress;
    socklen_t clientSocketLength;
    while(true) {
        clientSocketLength = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &clientAddress, &clientSocketLength)) == -1)
            return ERROR_SOCKET_ACCEPT_CONNECTION;

        struct HandleNewClientArguments args;
        args.newClientSocket = communicationSocketFD;
        args.communicationManager = this;
        args.groupsManager = &groupsManager;
        pthread_create(&clientConnections[threadIndex], nullptr, ServerCommunicationManager::staticNewClientConnectionKeepAlive, &args);
        pthread_create(&clientConnections[threadIndex], nullptr, ServerCommunicationManager::staticHandleNewClientConnection, &args);
    }

    return 0;
}
