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

#define PORT 4000

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
    clients.remove(socketFileDescriptor);

    int readWriteOperationResult;
    string disconnectionMessage = username + " desconectou!";

    /* Write message to all connected clients */
    for(std::list<SocketFD>::iterator client = std::begin(clients); client != std::end(clients); ++client) {
        int socketToWrite = *client;
        readWriteOperationResult = write(socketToWrite, disconnectionMessage.c_str(), disconnectionMessage.length());
        if (readWriteOperationResult < 0) {
            string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") writing into socket(" + std::to_string(socketToWrite) +")";
            perror(errorPrefix.c_str());
        }
    }
}

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

PacketHeader ServerCommunicationManager::readPacketHeaderFromSocket(SocketFD communicationSocket) {
    PacketHeader packetHeader;
    int readWriteOperationResult = read(communicationSocket, &packetHeader, sizeof(PacketHeader));
    bool readResult = handleReadResult(readWriteOperationResult, communicationSocket);
    if (readResult) {
        return packetHeader;
    } else {
        // TODO: Create error constant
        throw -123;
    }
}

Packet ServerCommunicationManager::readPacketFromSocket(SocketFD communicationSocket, int packetSize) {
    Packet packet;
    int readOperationResult = read(communicationSocket, &packet, packetSize);
    bool shouldContinue = handleReadResult(readOperationResult, communicationSocket);
    if (shouldContinue) {
        return packet;
    } else {
        // TODO: Create error constant
        throw -123;
    }
}

void ServerCommunicationManager::sendMessageToClients(const string& message, const std::list<UserConnection>& userConnections) {
    for (const UserConnection& userConnection:userConnections) {
        int readWriteOperationResult = write(userConnection.socket, message.c_str(), message.length());
        if (readWriteOperationResult < 0) {
            throw -321;
        }
    }
}

void *ServerCommunicationManager::handleNewClientConnection(HandleNewClientArguments *args) {
    SocketFD communicationSocket = args->newClientSocket;
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
            string errorPrefix = "Error(" + std::to_string(errorCode) + ") from socket(" + std::to_string(communicationSocket) +")";
            log(Error, errorPrefix);
            terminateClientConnection(communicationSocket, packet.payload.username);
            break;
        }
    }

    return NULL;
}

SocketFD ServerCommunicationManager::setupServerSocket() {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return SOCKET_CREATION_ERROR;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddress.sin_zero), 8);

    if (bind(connectionSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        return SOCKET_BINDING_ERROR;

    listen(connectionSocketFD, 5);

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
    struct sockaddr_in cli_addr;
    socklen_t clientSocketLength;
    while(true) {
        clientSocketLength = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &cli_addr, &clientSocketLength)) == -1)
            return ACCEPT_SOCKET_CONNECTION_ERROR;

        struct HandleNewClientArguments args;
        args.newClientSocket = communicationSocketFD;
        args.communicationManager = this;
        args.groupsManager = &groupsManager;
        pthread_create(&clientConnections[threadIndex], NULL, ServerCommunicationManager::staticHandleNewClientConnection, &args);
    }

    return 0;
}
