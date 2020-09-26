#include "ClientMessagesManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>

char messages[10][256];

int messagesNumber = 0;

struct ThreadParameter {
    ClientMessagesManager *client;
};

// thread de read messages teria que estar talvez dentro do communication manager?
void* ClientMessagesManager::readMessagesThread() {

    int readResult;

    while(true) {

        bzero(messages[messagesNumber], 256);
        readResult = communicationManager.readSocketMessage(messages[messagesNumber]);
        if (readResult < 0) {
            printf("ERROR reading from socket\n");
        } else if (readResult > 0) {
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

void* ClientMessagesManager::writeMessagesThread() {
    string message;
    int writeResult;

    int bufferSize = 10;
    char finalMessageBuffer[bufferSize];
    while(true) {
        message.clear();
        std::cout << " > ";
        std::getline(std::cin, message);

       // bzero(finalMessageBuffer, bufferSize);
       // strcpy(finalMessageBuffer, stringToSend.c_str());

        struct Message _message;
        _message.text = message;
        _message.username = this->userName;
        _message.timestamp = 0;
        _message.group = "oi buske";

        writeResult = communicationManager.writeSocketMessage(&_message);
        if (writeResult < 0)
            printf("ERROR writing to socket\n");
    }
}

int ClientMessagesManager::startClient(SocketConnectionInfo connectionInfo, UserInfo userInfo) {

    int socketConnectionResult;
    pthread_t mySocketReading, mySocketWriting;

    this->userName = userInfo.name;
    socketConnectionResult = communicationManager.connectClient(connectionInfo);

    if(socketConnectionResult < 0) {
        printf("Error connecting socket");
        return socketConnectionResult;
    }
    printf("Connection successful");

    ThreadParameter *tp = new ThreadParameter();
    tp->client = this;

    pthread_create(&mySocketReading, NULL, ClientMessagesManager::staticReadMessagesThread, tp);
    pthread_create(&mySocketWriting, NULL, ClientMessagesManager::staticWriteMessagesThread, tp);

    pthread_join(mySocketReading, NULL);
    pthread_join(mySocketWriting, NULL);

    // Pegar interrupcao do ctrl - c pra fechar o socket e finalizar a conexao (talves uma outra thread?)
    //close(sockfd);
    return 0;
}

void * ClientMessagesManager::staticReadMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->readMessagesThread();
    return NULL;
}

void * ClientMessagesManager::staticWriteMessagesThread(void *threadParm) {
    ThreadParameter* t = static_cast<ThreadParameter*>(threadParm);
    t->client->writeMessagesThread();
    return NULL;
}


