#include "ServerMessagesManager.h"

int ServerMessagesManager::loadInitialMessages(const string& groupName, std::list<Message>& messages, int messagesCount) {
    /// MARK: Critical session access
    this->groupsAccessControl.lockAccessForGroup(groupName);
    int numberOfMessagesRead = this->persistency.readMessages(groupName, messagesCount, messages);
    this->groupsAccessControl.unlockAccessForGroup(groupName);
    return numberOfMessagesRead;
}

int ServerMessagesManager::writeMessage(const Message& message) {
    /// MARK: Critical session access
    this->groupsAccessControl.lockAccessForGroup(message.groupName);
    int writeMessageResult = this->persistency.saveMessage(message);
    this->groupsAccessControl.unlockAccessForGroup(message.groupName);
    return writeMessageResult;
}

