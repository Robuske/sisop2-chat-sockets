#ifndef SISOP2_T1_SERVERMESSAGESMANAGER_H
#define SISOP2_T1_SERVERMESSAGESMANAGER_H

#include "Message/Message.h"
#include "Persistency/ServerPersistency.h"
#include "SharedDefinitions.h"
#include <iostream>
#include <list>

class ServerMessagesManager {

public:
    int loadInitialMessages(const string& groupName, std::list<Message>& messages, int messagesCount);
    int writeMessage(const Message& message);

private:
    ServerPersistency persistency;

};


#endif //SISOP2_T1_SERVERMESSAGESMANAGER_H
