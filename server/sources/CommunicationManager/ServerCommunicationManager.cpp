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


void *handleNewClientConnecton(void *arg) {

    int n;
    char buffer[256];
    int newSockFD = *(int*) arg;

    while(1) {

        bzero(buffer, 256);
        /* read from the socket */
        n = read(newSockFD, buffer, 256);
        if (n < 0)
            printf("ERROR reading from socket");

        printf("Here is the message: %s\n", buffer);

        /* write in the socket */

        int socketsIndex = 0;

        while (group1Sockets[socketsIndex] != -1){
            n = write(group1Sockets[socketsIndex],buffer, 256);
            if (n < 0) {
                printf("ERROR writing to socket");
            }

            socketsIndex++;
        }

        socketsIndex = 0;

        //close(newSockFD);
    }

}

int ServerCommunicationManager::startServer(int loadMessageCount) {

    pthread_t clientConnections[10];

    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    // Inicianto o soket com ([tipo de conexao(domain) - nesse caso remota], [type - TCP], [protocolo de comunicacao - 0 padrao])

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket");

    serv_addr.sin_family = AF_INET; // network conection
    serv_addr.sin_port = htons(PORT); // host to network big endian little endian conversion network port
    serv_addr.sin_addr.s_addr = INADDR_ANY; // ip address - 127.0.0.1
    bzero(&(serv_addr.sin_zero), 8);     // i dont know


    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        printf("ERROR on binding"); // for binding we use the socket (endpoint) we've just created with the socket address abstraction (used for local and internet connections)

    listen(sockfd, 5);

    int threadIndex = 0;

    while(1) {

        clilen = sizeof(struct sockaddr_in);
        if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
            printf("ERROR on accept");

        pthread_create(&clientConnections[threadIndex], NULL, handleNewClientConnecton, &newsockfd);
        group1Sockets[threadIndex] = newsockfd;
        threadIndex++;

    }

    close(sockfd);// Disconecting the server
    return 0;
}
