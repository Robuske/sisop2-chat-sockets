#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "CommunicationManager/ServerCommunicationManager.h"
#include "MessagesManager/ServerMessagesManager.h"
#include "GroupsAccessControl/GroupsAccessControl.h"
#include "SharedDefinitions.h"

struct Group {
    string name;
    std::list<UserConnection> clients;
};

class ServerGroupsManager {
private:
    int numberOfMessagesToLoadWhenUserJoined;
    std::list<Group> groups;
    ServerMessagesManager messagesManager;

    // Mutual exclusion control
    GroupsAccessControl groupsListAccessControl;
    GroupsAccessControl allGroupsAccessControl;

    ServerCommunicationManager *communicationManager;

    bool checkForUsersMaxConnections(const string &username);
    void sendMessagesToSpecificUser(UserConnection userConnection, std::list<Message> messages);
    void loadInitialMessagesForNewUserConnection(UserConnection userConnection, const string& groupName);
    void handleUserConnectionLimitReached(const string &username, const string &groupName, const UserConnection &userConnection);

public:
    void sendMessage(const Message& message);
    bool isConnectionValid(const UserConnection& userConnection);
    void handleUserDisconnection(UserConnection userConnection);
    void handleUserConnection(UserConnection userConnection, const string &groupName);
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
