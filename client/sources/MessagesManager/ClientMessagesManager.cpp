#include "ClientMessagesManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

char messages[10][256];

int messagesNumber = 0;

struct ThreadParameter {
    ClientMessagesManager *client;
    int socket;
};

// thread de read messages teria que estar talvez dentro do communication manager?
void* ClientMessagesManager::readMessagesThread(SocketFD socketFd) {

    int writeResult;

    while(true) {

        bzero(messages[messagesNumber], 256);
        writeResult = read(socketFd, messages[messagesNumber], 256);
        if (writeResult < 0) {
            printf("ERROR reading from socket\n");
        } else if (writeResult > 0) {
            system("clear");

            int index = 0;

            messagesNumber++;

            for(index = 0; index < messagesNumber; index++) {
                // Chamar display message do Client UI?
                printf("Message %d: %s\n", index, messages[index]);
            }
        }
    }
}

void* ClientMessagesManager::writeMessagesThread(SocketFD socketFd) {
    string message;
    int writeResult;

    int bufferSize = 256;
    char finalMessageBuffer[bufferSize];
    while(true) {
        message.clear();
        std::cout << " > ";
        std::getline(std::cin, message);

        string stringToSend = this->userName + ": " + message;

        bzero(finalMessageBuffer, bufferSize);
        strcpy(finalMessageBuffer, stringToSend.c_str());

        writeResult = write(socketFd, finalMessageBuffer, bufferSize);
        if (writeResult < 0)
            printf("ERROR writing to socket\n");
    }
}

int ClientMessagesManager::writeUserFirstMessage(SocketFD socketFd) {

    char buffer[256];
    int writeResult;

    strcpy(buffer, this->userName.c_str());
    strcat(buffer, " entrou...");

    writeResult = write(socketFd, buffer, 256);

    return writeResult;
}

int ClientMessagesManager::startClient(SocketConnectionInfo connectionInfo, UserInfo userInfo) {

    SocketFD socketConnectionResult;
    pthread_t mySocketReading, mySocketWriting;
    int fstMessageWriteResult;

    this->userName = userInfo.name;
    socketConnectionResult = this->connectClient(connectionInfo);

    if(socketConnectionResult < 0) {
        printf("Error connecting socket");
        return socketConnectionResult;
    }
    printf("Connection successful");
    // Send first message

    if (this->writeUserFirstMessage(socketConnectionResult) < 0 ) {
        printf("Error sending first Message");
        return WRITING_FST_MESSAGE_ERROR;
    }

    ThreadParameter *tp = new ThreadParameter();
    tp->client = this;
    tp->socket = socketConnectionResult;

    pthread_create(&mySocketReading, NULL, ClientMessagesManager::staticReadMessagesThread, tp);
    pthread_create(&mySocketWriting, NULL, ClientMessagesManager::staticWriteMessagesThread, tp);

    pthread_join(mySocketReading, NULL);
    pthread_join(mySocketWriting, NULL);

    // Pegar interrupcao do ctrl - c pra fechar o socket e finalizar a conexao (talves uma outra thread?)
    //close(sockfd);
    return 0;
}

int ClientMessagesManager::connectClient(SocketConnectionInfo connectionInfo) {

    SocketFD sockFd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(connectionInfo.ipAddress.c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return INVALID_HOST_ERROR;
    }

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("ERROR opening socket\n");
        return SOCKET_CREATION_ERROR;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(connectionInfo.port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockFd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        return SOCKET_CONNECTION_ERROR;
    }

    return sockFd;
}

void * ClientMessagesManager::staticReadMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->readMessagesThread(t->socket);
    return NULL;
}

void * ClientMessagesManager::staticWriteMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->writeMessagesThread(t->socket);
    return NULL;
}


