#include "ServerCommunicationManager.h"
#include "SharedDefinitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "GroupsManager/ServerGroupsManager.h"

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
    return NULL;
}

// MARK: - Instance methods
// TODO: Send disconnection message to all remaining client
void ServerCommunicationManager::terminateClientConnection(SocketFD socketFileDescriptor, string username) {
    // TODO: Do we need to close the socket here?
    // close(socketFileDescriptor);
    clients.remove(socketFileDescriptor);

    int readWriteOperationResult;
    string disconnectionMessage = username + " desconectou!";
    // TODO: We should notify only the clients of a particular group, not all clients
    // BUSKE: Pq a gente nao tem o groupsManager na instancia?
    // Estamos apenas passando como argumento na staticHandleNewClientConnection
    // Doug falou que tu explicou algo pra ele.
    // this->groupsManager->handleUserDisconnection(socketFileDescriptor);
    // This for was moved to handleUserDisconnection.
    for(std::list<SocketFD>::iterator client = std::begin(clients); client != std::end(clients); ++client) {
        int socketToWrite = *client;
        readWriteOperationResult = write(socketToWrite, disconnectionMessage.c_str(), disconnectionMessage.length());
        if (readWriteOperationResult < 0) {
            string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") writing into socket(" + std::to_string(socketToWrite) +")";
            perror(errorPrefix.c_str());
        }
    }
}

// TODO: This will die in favor of using throw where it's being used
bool ServerCommunicationManager::handleReadResult(int readResult, int socket) {
    bool isEndOfFile = (readResult == 0);
    if (isEndOfFile) {
        string message = "Read result(" + std::to_string(readResult) + ") reading from socket(" + std::to_string(socket) + ")";
        log(Info, message);
        return false;
    } else if (readResult < 0) {
        string errorPrefix = "Read result(" + std::to_string(readResult) + ") reading from socket(" + std::to_string(socket) + ")";
        log(Error, errorPrefix);
        return false;
    }

    return true;
}

// TODO: Make this global?
// TODO: Move this somewhere?
#define ERROR_TERMINATE_CONNECTION -7
#define ERROR_CLIENT_DISCONNECTED -134

// TODO: readPacketHeaderFromSocket and readPacketFromSocket can be refactored, the only difference is the type of what we're reading.
// I tried the code below but the server was crashing when a user connects:
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

PacketHeader ServerCommunicationManager::readPacketHeaderFromSocket(SocketFD communicationSocket) {
    PacketHeader packetHeader;
    int readOperationResult = read(communicationSocket, &packetHeader, sizeof(PacketHeader));
    if (readOperationResult == 0) {
        throw ERROR_CLIENT_DISCONNECTED;
    } else if (readOperationResult < 0) {
        throw readOperationResult;
    } else {
        return packetHeader;
    }
}

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

// TODO: Change `string message` to be a `Message message`
void ServerCommunicationManager::sendMessageToClients(const string& message, const std::list<UserConnection>& userConnections) {
    for (const UserConnection& userConnection:userConnections) {
        int readWriteOperationResult = write(userConnection.socket, message.c_str(), message.length());
        if (readWriteOperationResult < 0) {
            // TODO: Create constant
            throw -321;
        }
    }
}

void *ServerCommunicationManager::handleNewClientConnection(HandleNewClientArguments *args) {
    SocketFD communicationSocket = args->newClientSocket;
    // TODO: Add client to correct list in a group
    clients.push_back(communicationSocket);

    Packet packet;
    bool shouldContinue = true;
    while(shouldContinue) {
        try {
            PacketHeader packetHeader = readPacketHeaderFromSocket(communicationSocket);
            packet = readPacketFromSocket(communicationSocket, packetHeader.length);
            if (packetHeader.type == TypeConnection) {
                args->groupsManager->handleUserConnection(packet.payload.username,
                                                          communicationSocket,
                                                          packet.payload.group);
            } else if (packetHeader.type == TypeMessage) {
                args->groupsManager->sendMessage(packet.payload);
            }
        } catch (int errorCode) {
            string errorPrefix =
                    "Error(" + std::to_string(errorCode) + ") from socket(" + std::to_string(communicationSocket) + ")";
            log(Error, errorPrefix);
            if (errorCode == ERROR_CLIENT_DISCONNECTED) {
                terminateClientConnection(communicationSocket, packet.payload.username);
            };
            break;
        }
    }

    return NULL;
}

SocketFD ServerCommunicationManager::setupServerSocket() {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddress.sin_zero), 8);

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

    // TODO: Change this to std::list
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
        pthread_create(&clientConnections[threadIndex], NULL, ServerCommunicationManager::staticHandleNewClientConnection, &args);
    }

    return 0;
}
