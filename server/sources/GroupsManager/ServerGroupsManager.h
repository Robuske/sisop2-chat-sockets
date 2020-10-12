#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "CommunicationManager/ServerCommunicationManager.h"
#include "MessagesManager/ServerMessagesManager.h"
#include "GroupsAccessControl/GroupsAccessControl.h"
#include "SharedDefinitions.h"

struct UserConnection {
    string username;
    SocketFD socket;

    bool operator==(const struct UserConnection& a) const {
        return ( a.socket == this->socket && a.username == this->username );
    }
};

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
    string getUsernameForSocket(SocketFD socketFd);
    void handleUserDisconnection(SocketFD socket, const string& username);
    void handleUserConnection(const string& username, SocketFD socket, const string& group);
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
