#ifndef SISOP2_T1_SERVERGROUPSMANAGER_H
#define SISOP2_T1_SERVERGROUPSMANAGER_H

#include "CommunicationManager/ServerCommunicationManager.h"
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
    void sendMessagesToSpecificUser(SocketFD socket, Message* messages, int messagesCount);
    int numberOfMessagesToLoadWhenUserJoined;
    ServerCommunicationManager *communicationManager;
    std::list<Group> groups;

public:
    ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager);
    void handleUserConnection(const string& username, SocketFD socket, const string& group);
    void sendMessage(const Message& message);

    void handleUserDisconnection(const string &username, SocketFD socket, const string &groupName);

    void handleUserDisconnection(SocketFD socket);
};


#endif //SISOP2_T1_SERVERGROUPSMANAGER_H
