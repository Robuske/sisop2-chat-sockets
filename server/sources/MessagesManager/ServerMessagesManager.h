#ifndef SISOP2_T1_SERVERMESSAGESMANAGER_H
#define SISOP2_T1_SERVERMESSAGESMANAGER_H

#include "Persistency/ServerPersistency.h"
#include "SharedDefinitions.h"
#include <iostream>
#include <algorithm>

class ServerMessagesManager {

public:
    int loadInitialMessages(string groupName, Message* messages, int messagesCount);

private:
    ServerPersistency persistency;

};


#endif //SISOP2_T1_SERVERMESSAGESMANAGER_H
