#include "ServerMessagesManager.h"


int ServerMessagesManager::loadInitialMessages(const string& groupName, std::list<Message>& messages, int messagesCount) {
    int numberOfMessagesRead = this->persistency.readMessages(groupName, messagesCount, messages);
    return numberOfMessagesRead;
}

int ServerMessagesManager::writeMessage(const Message& message) {
    return this->persistency.saveMessage(message);
}
