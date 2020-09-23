#include "SharedDefinitions.h"
#include "ClientUI.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

#define PORT 4000

pthread_t mySocketReading, mySocketWriting;

char messages[10][256];

int messagesNumber = 0;

char userName[30];

void *readingMessagesThread(void *my_socket) {

    char buffer[256];
    int n;
    int sockfd = *(int*) my_socket;


    while(1) {

        bzero(messages[messagesNumber], 256);
        n = read(sockfd, messages[messagesNumber], 256);
        if (n < 0) {
            printf("ERROR reading from socket\n");
        } else if (n>0) {
            system("clear");

            int index = 0;

            messagesNumber++;

            for(index = 0; index < messagesNumber; index++) {
                printf("Message - %d) %s\n", index, messages[index]);
            }

        }
    }

}


void *writingMessagesThread(void *my_socket) {

    char buffer[256];
    char message[256];
    int n;
    int sockfd = *(int*) my_socket;

    while(1) {
        printf("Enter the message: ");
        bzero(buffer, 256);
        fgets(buffer, 256, stdin);

        strcpy(message, userName);
        strcat(message, " - ");
        strcat(message, buffer);


        n = write(sockfd, buffer, 256);
        if (n < 0)
            printf("ERROR writing to socket\n");
    }
}

int ClientUI::startClient(string userName, string groupName, string serverIPAddress, unsigned short port) {
    std::cout << "Cliente iniciado" << std::endl;
    std::cout << userName << "@" << serverIPAddress << ":" << port << "/" << groupName << "/db.txt.chupa" << std::endl;

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    server = gethostbyname(serverIPAddress.c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        printf("ERROR opening socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        printf("ERROR connecting\n");

    strcpy(buffer, userName.c_str());
    strcat(buffer, " entrou...");

    n = write(sockfd, buffer, 256);
    if (n < 0) {
        printf("ERROR writing to socket");
    }

    pthread_create(&mySocketReading, NULL, readingMessagesThread, &sockfd);
    pthread_create(&mySocketWriting, NULL, writingMessagesThread, &sockfd);



    pthread_join(mySocketReading, NULL);
    pthread_join(mySocketWriting, NULL);



    //close(sockfd);
    return 0;
}
