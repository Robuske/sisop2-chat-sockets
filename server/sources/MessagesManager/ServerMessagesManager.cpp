#include "ServerMessagesManager.h"


int ServerMessagesManager::loadInitialMessages(const string& groupName, Message* messages, int messagesCount) {
    int numberOfMessagesRead = this->persistency.readMessages(groupName, messagesCount, messages);
    // TODO: Não entendi o que era pra isso fazer \/
//    if(numberOfMessagesRead > 0) {
//        std::sort(messages, messages + (numberOfMessagesRead-1));
//    }

    return numberOfMessagesRead;
}

int ServerMessagesManager::writeMessage(const Message& message) {
    return this->persistency.saveMessage(message);
}
