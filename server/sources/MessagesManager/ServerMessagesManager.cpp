#include "ServerMessagesManager.h"


int ServerMessagesManager::loadInitialMessages(string groupName, Message* messages, int messagesCount) {
    int numberOfMessagesRead = this->persistency.readMessages(groupName, messagesCount, messages);
    if(numberOfMessagesRead > 0) {
        std::sort(messages, messages + (numberOfMessagesRead-1));
    }
    return CODE_SUCCESS;
}
