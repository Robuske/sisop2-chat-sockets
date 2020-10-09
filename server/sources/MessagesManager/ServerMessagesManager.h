#ifndef SISOP2_T1_SERVERMESSAGESMANAGER_H
#define SISOP2_T1_SERVERMESSAGESMANAGER_H

#include "Message/Message.h"
#include "Persistency/ServerPersistency.h"
#include "SharedDefinitions.h"
#include <iostream>
#include <list>
#include <map>
#include <mutex>

/**
 * Struct: GroupConcurrentAccessControl
 * Responsible to manage the group users access to the group files
 * @attributes [groupName, groupMutex]
 */

typedef std::map<string , std::mutex> GroupAccessControl;

class ServerMessagesManager {

public:
    int loadInitialMessages(const string& groupName, std::list<Message>& messages, int messagesCount);
    int writeMessage(const Message& message);
    void lockAccessForGroup(const string& groupName);
    void unlockAccessForGroup(const string& groupName);

private:
    ServerPersistency persistency;
    GroupAccessControl groupsAccessControl;
};


#endif //SISOP2_T1_SERVERMESSAGESMANAGER_H
