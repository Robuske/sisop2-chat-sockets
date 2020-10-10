#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "CommunicationManager/ServerCommunicationManager.h"
#include "MessagesManager/ServerMessagesManager.h"
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
    void sendMessagesToSpecificUser(UserConnection userConnection, std::list<Message> messages, int messagesCount);
    void loadInitialMessagesForNewUserConnection(UserConnection userConnection, const string& groupName);
    int numberOfMessagesToLoadWhenUserJoined;
    ServerCommunicationManager *communicationManager;
    ServerMessagesManager messagesManager;
    std::list<Group> groups;

public:
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
    void handleUserConnection(const string& username, SocketFD socket, const string& group);
    void sendMessage(const Message& message);

    void handleUserDisconnection(SocketFD socket, const string& username);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
