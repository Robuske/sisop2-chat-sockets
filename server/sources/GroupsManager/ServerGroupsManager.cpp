#include "ServerGroupsManager.h"

void ServerGroupsManager::sendMessage(const Message& message) {
    bool groupFound = false;
    string groupName = message.group;
    Group groupToSendMessage;
    for (const Group& currentGroup: groups) {
        if (currentGroup.name == groupName) {
            groupToSendMessage = currentGroup;
            groupFound = true;
            break;
        }
    }

    if (!groupFound) {
        throw -333;
    }

    communicationManager->sendMessageToClients(message.text, groupToSendMessage.clients);
}

// This can throw
void ServerGroupsManager::handleUserConnection(const string& username, SocketFD socket, const string& groupName) {
    UserConnection userConnection;
    userConnection.username = username;
    userConnection.socket = socket;

    std::list<UserConnection> userConnections;
    bool groupFound = false;
    for (Group &currentGroup:groups) {
        if (currentGroup.name == groupName) {
            groupFound = true;
            currentGroup.clients.push_back(userConnection);
            userConnections = currentGroup.clients;
            break;
        }
    }

    if (!groupFound) {
        Group newGroup = Group();
        newGroup.name = groupName;
        newGroup.clients.push_back(userConnection);
        groups.push_back(newGroup);
        userConnections = newGroup.clients;
    }

    const string joinMessage = username + " conectou!";
    communicationManager->sendMessageToClients(joinMessage, userConnections);
}

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}