#include "ClientMessagesManager.h"
#include <pthread.h>
#include <iostream>
#include <list>

std::list<Message> messages;

// TODO: Can we remove this?
int messagesNumber = 0;

struct ThreadParameter {
    ClientMessagesManager *client;
};

void  ClientMessagesManager::sendKeepAliveMessage() {
    Message keepAliveMessage = Message(TypeKeepAlive);
    int writeResult = communicationManager.writeSocketMessage(keepAliveMessage);
    if (writeResult < 0) {
        string errorPrefix = "Error(" + std::to_string(writeResult) + ") writing keep alive message to socket";
        perror(errorPrefix.c_str());
    }
}

// thread de read messages teria que estar talvez dentro do communication manager?
void* ClientMessagesManager::readMessagesThread() {

    while(true) {
        try {
            Message message = communicationManager.readSocketMessage();
            if (message.packetType == TypeKeepAlive) {
                sendKeepAliveMessage();
            }
            system("clear");
            std::cout << "Grupo: " << userInfo.groupName << std::endl;
            int index = 1;
            messages.push_back(message);
            for (Message message:messages) {
                std::cout << "Message " << index++ << " [" << message.username << "]" <<": " << message.text << std::endl;
            }

        } catch (int error) {
            string errorPrefix = "Error(" + std::to_string(error) + ") reading from socket";
            perror(errorPrefix.c_str());
            return nullptr; // TODO: Provavelmente isso tudo vai mudar bastante, mas no momento não faz sentido manter a leitura
        }
    }
}

void* ClientMessagesManager::writeMessagesThread() {
    string messageString;
    int writeResult;

    Message connectionMessage = Message(TypeConnection, 12345, userInfo.groupName, userInfo.username, "");

    // TODO: Throw or handle error
    communicationManager.writeConnectionMessageToSocket(connectionMessage);

    while(true) {
        messageString.clear();
        std::cout << " > ";
        std::getline(std::cin, messageString);

//        // TODO: Timestamp
//        message.timestamp = 1234;
//        message.group = userInfo.groupName;
        Message message = Message(TypeMessage, 654321, userInfo.groupName, userInfo.username, messageString);

        writeResult = communicationManager.writeSocketMessage(message);
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

    std::cout << "Connection successful" << std::endl;

    auto *tp = new ThreadParameter();
    tp->client = this;

    pthread_create(&mySocketReading, nullptr, ClientMessagesManager::staticReadMessagesThread, tp);
    pthread_create(&mySocketWriting, nullptr, ClientMessagesManager::staticWriteMessagesThread, tp);

    pthread_join(mySocketReading, nullptr);
    pthread_join(mySocketWriting, nullptr);

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


