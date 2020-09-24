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

struct ThreadParameter {
    ClientUI *client;
    int socket;
};

pthread_t mySocketReading, mySocketWriting;

char messages[10][256];

int messagesNumber = 0;

void* ClientUI::readingMessagesThread(int my_socket) {

//    char buffer[256];
    int n;
    int sockfd = my_socket;


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
                printf("Message %d: %s\n", index, messages[index]);
            }

        }
    }
}

void* ClientUI::writingMessagesThread(int my_socket) {
    string message;
    int n;
    int sockfd = my_socket;

    int bufferSize = 256;
    char finalMessageBuffer[bufferSize];
    while(1) {
        message.clear();
        std::cout << " > ";
        std::getline(std::cin, message);

        string stringToSend = this->userName + ": " + message;

        bzero(finalMessageBuffer, bufferSize);
        strcpy(finalMessageBuffer, stringToSend.c_str());

        n = write(sockfd, finalMessageBuffer, bufferSize);
        if (n < 0)
            printf("ERROR writing to socket\n");
    }
}

int ClientUI::startClient(string userName, string groupName, string serverIPAddress, unsigned short port) {
    std::cout << "Cliente iniciado" << std::endl;
    std::cout << userName << "@" << serverIPAddress << ":" << port << "/" << groupName << "/db.txt.chupa" << std::endl;

    this->userName = userName;
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

    ThreadParameter *tp = new ThreadParameter();
    tp->client = this;
    tp->socket = sockfd;

    pthread_create(&mySocketReading, NULL, ClientUI::staticReadingMessagesThread, tp);
    pthread_create(&mySocketWriting, NULL, ClientUI::staticWritingMessagesThread, tp);

    pthread_join(mySocketReading, NULL);
    pthread_join(mySocketWriting, NULL);

    //close(sockfd);
    return 0;
}

void * ClientUI::staticReadingMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->readingMessagesThread(t->socket);
    return NULL;
}

void * ClientUI::staticWritingMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->writingMessagesThread(t->socket);
    return NULL;
}
