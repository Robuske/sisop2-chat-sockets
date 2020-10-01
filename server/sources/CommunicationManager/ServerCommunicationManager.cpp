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
#include <iostream>
#include <list>
#include "GroupsManager/ServerGroupsManager.h"

#define PORT 4000

std::list<SocketFD> clients;

enum eLogLevel { Info, Debug, Error } typedef LogLevel;
void log(LogLevel logLevel, string msg) {
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

// TODO: Send disconnection message to all remaining client
void terminateClientConnection(SocketFD socketFileDescriptor, string username) {
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

bool handleReadResult(int readResult, int socket) {
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

void *handleNewClientConnection(void *sock) {
    int readWriteOperationResult, socketToWriteIndex = 0;
    struct HandleNewClientArguments args = *(HandleNewClientArguments*)sock;
    SocketFD communicationSocket = *(int*) sock;
    communicationSocket = args.newClientSocket;
    clients.push_back(communicationSocket);

    struct PacketHeader* packetHeader =  (PacketHeader*) malloc(sizeof(PacketHeader));
    struct Message* message = (Message*) malloc(sizeof(Message));

    // TODO: Handle group creation
//    GroupManager.handleGroupCreation()


    bool shouldContinue = true;
    while(shouldContinue) {
        // Read header
        bzero(packetHeader, sizeof(PacketHeader));
        readWriteOperationResult = read(communicationSocket, packetHeader, sizeof(PacketHeader));
        shouldContinue = handleReadResult(readWriteOperationResult, communicationSocket);
        if (!shouldContinue) {
            terminateClientConnection(communicationSocket, message->username);
            break;
        }

        // Read content
        bzero(message, sizeof(Message));
        readWriteOperationResult = read(communicationSocket, message, packetHeader->length);
        shouldContinue = handleReadResult(readWriteOperationResult, communicationSocket);
        if (!shouldContinue) {
            terminateClientConnection(communicationSocket, message->username);
            break;
        }

//        GroupManager.writeToGroup();

        // Write message to all connected clients
        for(std::list<SocketFD>::iterator client = std::begin(clients); client != std::end(clients); ++client) {
            int socketToWrite = *client;
            readWriteOperationResult = write(socketToWrite, message->text.c_str(), message->text.length());
            if (readWriteOperationResult < 0) {
                string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") writing into socket(" + std::to_string(socketToWrite) +")";
                perror(errorPrefix.c_str());
            }
        }
    }
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

struct HandleNewClientArguments {
    ServerCommunicationManager *communicationManager;
    ServerGroupsManager *groupsManager;
    SocketFD newClientSocket;
};

int ServerCommunicationManager::startServer(int loadMessageCount) {
    ServerGroupsManager groupsManager = ServerGroupsManager(loadMessageCount, this);

    // TODO: Change this to std::list
    pthread_t clientConnections[10];

    SocketFD communicationSocketFD, connectionSocketFDResult;
    connectionSocketFDResult = this->setupServerSocket();
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

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
        pthread_create(&clientConnections[threadIndex], NULL, handleNewClientConnection, &args);
    }

    return 0;
}
