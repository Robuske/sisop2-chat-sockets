#include "ClientMessagesManager.h"
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

        readResult = communicationManager.readSocketMessage(messages[messagesNumber]);
        if (readResult < 0) {
            string errorPrefix = "Error(" + std::to_string(readResult) + ") reading from socket";
            perror(errorPrefix.c_str());
            return nullptr; // TODO: Provavelmente isso tudo vai mudar bastante, mas no momento não faz sentido manter a leitura

        } else if (readResult > 0) {
            system("clear");

            int index = 0;

            messagesNumber++;

            for (index = 0; index < messagesNumber; index++) {
                // Chamar display message do Client UI?
                std::cout << "Message " << index << ": " << messages[index] << std::endl;
            }
        }
    }
}

void* ClientMessagesManager::writeMessagesThread() {
    string messageString;
    int writeResult;

    Message connectionMessage;
    connectionMessage.username = userInfo.username;
    connectionMessage.group = userInfo.groupName;
    communicationManager.writeConnectionMessageToSocket(&connectionMessage);

    while(true) {
        messageString.clear();
        std::cout << " > ";
        std::getline(std::cin, messageString);

        struct Message message;
        message.text = messageString;
        message.username = userInfo.username;
        message.timestamp = 1234;
        message.group = userInfo.groupName;

        writeResult = communicationManager.writeSocketMessage(&message);
        if (writeResult < 0) {
            string errorPrefix = "Error(" + std::to_string(writeResult) + ") writing to socket";
            perror(errorPrefix.c_str());
        }
    }
}

int ClientMessagesManager::startClient(const SocketConnectionInfo& connectionInfo, UserInfo userInfo) {

    int socketConnectionResult;
    pthread_t mySocketReading, mySocketWriting;

    this->userInfo = userInfo;

    socketConnectionResult = communicationManager.connectClient(connectionInfo);
    if (socketConnectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(socketConnectionResult) + ") connecting client";
        perror(errorPrefix.c_str());
        return socketConnectionResult;
    }

    std::cout << "Connection successful";

    auto *tp = new ThreadParameter();
    tp->client = this;

    pthread_create(&mySocketReading, nullptr, ClientMessagesManager::staticReadMessagesThread, tp);
    pthread_create(&mySocketWriting, nullptr, ClientMessagesManager::staticWriteMessagesThread, tp);

    pthread_join(mySocketReading, nullptr);
    pthread_join(mySocketWriting, nullptr);

    // Pegar interrupcao do ctrl - c pra fechar o socket e finalizar a conexao (talves uma outra thread?)

    return 0;
}

void * ClientMessagesManager::staticReadMessagesThread(void *threadParm) {
    auto* t = static_cast<ThreadParameter*>(threadParm);
    t->client->readMessagesThread();
    return nullptr;
}

void * ClientMessagesManager::staticWriteMessagesThread(void *threadParm) {
    auto* t = static_cast<ThreadParameter*>(threadParm);
    t->client->writeMessagesThread();
    return nullptr;
}


