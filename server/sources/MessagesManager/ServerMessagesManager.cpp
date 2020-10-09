#include "ServerMessagesManager.h"


int ServerMessagesManager::loadInitialMessages(const string& groupName, std::list<Message>& messages, int messagesCount) {

    this->lockAccessForGroup(groupName);
    int numberOfMessagesRead = this->persistency.readMessages(groupName, messagesCount, messages);
    this->unlockAccessForGroup(groupName);
    return numberOfMessagesRead;
}

int ServerMessagesManager::writeMessage(const Message& message) {

    this->lockAccessForGroup(message.groupName);
    int writeMessageResult = this->persistency.saveMessage(message);
    this->unlockAccessForGroup(message.groupName);
    return writeMessageResult;
}

/**
 * Function: ServerMessagesManager::unlockAccessForGroup
 * Unlock the corresponding group mutex allowing other threads to write/read in/from the group files
 * @param[in] groupName
 * @param[out] void
 */

void ServerMessagesManager::unlockAccessForGroup(const string &groupName) {
    this->groupsAccessControl[groupName].unlock();
}

/**
 * Function: ServerMessagesManager::lockAccessForGroup
 * Lock the corresponding group mutex blocking other threads from writing/reading the group files
 * @param[in] groupName
 * @param[out] void
 */


void ServerMessagesManager::lockAccessForGroup(const string &groupName) {

    // Acessing a member of a map that doest not exist creates a default one with group name
    // There if we are trying to access a mutex of a group that doesn't exist an entry will
    // be automaticly created in the persistency access control map
    this->groupsAccessControl[groupName].lock();

    // The code bellow will not be necessary
    // however let's keep it in here until the pull request is approved =)

    //bool groupFound = false;

//    for (auto& groupAccess : persistencyAccessControl) {
//        if(groupAccess.first == groupName) {
//            groupAccess.second.lock();
//            groupFound = true;
//            break;
//        }
//    }
//
//    if(!groupFound) {
//        this->persistencyAccessControl.emplace(groupName, new std::mutex);
//        this->persistencyAccessControl[groupName].lock();
//    }
}

