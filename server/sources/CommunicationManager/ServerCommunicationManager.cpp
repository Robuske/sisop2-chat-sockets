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

#define PORT 4000

std::list<SocketFD> clients;

void *handleNewClientConnection(void *sock) {

    int readWriteOperationResult, socketToWriteIndex = 0;
    SocketFD communicationSocket = *(int*) sock;
    clients.push_back(communicationSocket);

    struct PacketHeader* packetHeader =  (PacketHeader*) malloc(sizeof(PacketHeader));
    struct Message* message = (Message*) malloc(sizeof(Message));

    while(true) {

        bzero(packetHeader, sizeof(PacketHeader));

        readWriteOperationResult = read(communicationSocket, packetHeader, sizeof(PacketHeader));
        if (readWriteOperationResult < 0) {
            string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") reading from socket";
            perror(errorPrefix.c_str());
        }


        readWriteOperationResult = read(communicationSocket, message, packetHeader->length);

        if (readWriteOperationResult < 0) {
            string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") reading from socket";
            perror(errorPrefix.c_str());
        }

        /* Write message to all connected clients */
        for(std::list<SocketFD>::iterator client = std::begin(clients); client != std::end(clients); ++client) {
            int socketToWrite = *client;
            readWriteOperationResult = write(socketToWrite, message->text.c_str(), message->text.length());
            if (readWriteOperationResult < 0) {
                string errorPrefix = "Error(" + std::to_string(readWriteOperationResult) + ") writing int socket(" + std::to_string(socketToWrite) +")";
                perror(errorPrefix.c_str());
            }
        }

        socketToWriteIndex = 0;
    }
}


SocketFD ServerCommunicationManager::connectServer() {

    SocketFD connectionSocketFD;
    struct sockaddr_in serv_addr;

    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return SOCKET_CREATION_ERROR;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(connectionSocketFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        return SOCKET_BINDING_ERROR;

    listen(connectionSocketFD, 5);

    return connectionSocketFD;
}

int ServerCommunicationManager::startServer(int loadMessageCount) {

    pthread_t clientConnections[10];

    SocketFD communicationSocketFD, connectionSocketFDResult;
    socklen_t clilen;

    struct sockaddr_in cli_addr;

    connectionSocketFDResult = this->connectServer();
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

    int threadIndex = 0;

    while(true) {
        clilen = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &cli_addr, &clilen)) == -1)
            return ACCEPT_SOCKET_CONNECTION_ERROR;

        pthread_create(&clientConnections[threadIndex], NULL, handleNewClientConnection, &communicationSocketFD);
    }


   // close(sockfd);// Disconecting the server
    return 0;
}
