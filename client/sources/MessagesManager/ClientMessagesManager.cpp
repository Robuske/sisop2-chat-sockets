#include "ClientMessagesManager.h"
#include <pthread.h>
#include <iostream>
#include <list>

std::list<Message> messages;

struct ThreadParameter {
    ClientMessagesManager *client;
};

void  ClientMessagesManager::sendKeepAliveMessage() {
    Message keepAliveMessage = Message::keepAliveWithUsername(userInfo.username);
    int writeResult = communicationManager.writeSocketMessage(keepAliveMessage);
    if (writeResult < 0) {
        string errorPrefix = "Error(" + std::to_string(writeResult) + ") writing keep alive message to socket";
        perror(errorPrefix.c_str());
    }
}

void* ClientMessagesManager::readMessagesThread() {
    // TODO: DEBUG: For testing. We should always send keep alive.
    bool shouldPrintKeepAlive = false;
    bool shouldSendKeepAlive = true;
    while (true) {
        try {
            Message message = communicationManager.readSocketMessage();
            if (message.packetType == TypeKeepAlive) {
                if (shouldSendKeepAlive) {
                    sendKeepAliveMessage();
                }

                if (shouldPrintKeepAlive) {
                    messages.push_back(message);
                    this->clientUI.displayMessages(messages, userInfo);

                }
            } else {

                messages.push_back(message);

                this->clientUI.displayMessages(messages, userInfo);

                if (message.packetType == TypeMaxConnectionsReached) {
                    string errorPrefix = "Error(" + std::to_string(ERROR_MAX_CONNECTIONS_PER_USERNAME_REACHED) + ") reading from socket";
                    perror(errorPrefix.c_str());
                    exit(EXIT_FAILURE);
                }
            }

        } catch (int error) {
            string errorPrefix = "Error(" + std::to_string(error) + ") reading from socket";
            perror(errorPrefix.c_str());
            exit(EXIT_FAILURE);
        }
    }
}

void* ClientMessagesManager::writeMessagesThread() {
    string messageString;
    int writeResult;

    Message connectionMessage = Message(TypeConnection, now(), userInfo.groupName, userInfo.username, "");

    // TODO: Throw or handle error
    communicationManager.writeConnectionMessageToSocket(connectionMessage);

    while(true) {
        messageString.clear();
        std::getline(std::cin, messageString);
        if (messageString.length() >= MESSAGE_SIZE) {
            this->clientUI.displayMessageSizeError(messageString.length());
        } else {
            Message message = Message(TypeMessage, now(), userInfo.groupName, userInfo.username, messageString);
            writeResult = communicationManager.writeSocketMessage(message);
            if (writeResult < 0) {
                string errorPrefix = "Error(" + std::to_string(writeResult) + ") writing to socket";
                perror(errorPrefix.c_str());
            }
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


