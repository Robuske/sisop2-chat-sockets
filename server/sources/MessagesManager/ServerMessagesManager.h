#ifndef SISOP2_T1_SERVERMESSAGESMANAGER_H
#define SISOP2_T1_SERVERMESSAGESMANAGER_H

#include "Persistency/ServerPersistency.h"
#include "SharedDefinitions.h"
#include <iostream>

class ServerMessagesManager {

public:
    int loadInitialMessages(const string& groupName, Message* messages, int messagesCount);
    int writeMessage(const Message& message);

private:
    ServerPersistency persistency;

};


#endif //SISOP2_T1_SERVERMESSAGESMANAGER_H
