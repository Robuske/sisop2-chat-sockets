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

#define PORT 4000

// Handle new socket connection thread

int group1Sockets[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};


void *handleNewClientConnection(void *sock) {

    int operationsResult;
    char buffer[256];
    SocketFD communicationSocket = *(int*) sock;

    while(true) {

        bzero(buffer, 256);
        /* read from the socket */
        operationsResult = read(communicationSocket, buffer, 256);
        if (operationsResult < 0)
            printf("ERROR reading from socket");

        printf("Here is the message: %s\n", buffer);

        /* write in the socket */

        int socketsIndex = 0;

        while (group1Sockets[socketsIndex] != -1){
            operationsResult = write(group1Sockets[socketsIndex],buffer, 256);
            if (operationsResult < 0) {
                printf("ERROR writing to socket");
            }

            socketsIndex++;
        }

        socketsIndex = 0;
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
        group1Sockets[threadIndex] = communicationSocketFD;
        threadIndex++;
    }


   // close(sockfd);// Disconecting the server
    return 0;
}
