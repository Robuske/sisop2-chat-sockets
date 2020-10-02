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
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
    void handleUserConnection(const string& username, SocketFD socket, const string& group);
    void sendMessage(const Message& message);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
