#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "SharedDefinitions.h"
#include "CommunicationManager/ServerCommunicationManager.h"
#include <list>

class ServerGroupsManager {
private:
    int numberOfMessagesToLoadWhenUserJoined;
    ServerCommunicationManager *communicationManager;

public:
    void handleUserConnection(string username, SocketFD socket, string group);
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
