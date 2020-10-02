#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "SharedDefinitions.h"
#include "CommunicationManager/ServerCommunicationManager.h"
#include <list>

struct UserConnection {
    string username;
    SocketFD socket;
};

struct Group {
    string name;
    std::list<UserConnection> clients;
};

class ServerGroupsManager {
private:
    int numberOfMessagesToLoadWhenUserJoined;
    ServerCommunicationManager *communicationManager;
    std::list<Group> groups;

public:
    void handleUserConnection(string username, SocketFD socket, string group);
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
    void sendMessage(Message message);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
