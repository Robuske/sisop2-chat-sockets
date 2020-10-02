#include "ServerGroupsManager.h"

void ServerGroupsManager::sendMessage(Message message) {
    bool groupFound = false;
    string groupName = message.group;
    Group groupToSendMessage;
    for (Group currentGroup: groups) {
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
void ServerGroupsManager::handleUserConnection(string username, SocketFD socket, string groupName) {
    bool groupFound = false;
    Group groupToAddUser;
    for (Group currentGroup:groups) {
        if (currentGroup.name == groupName) {
            groupToAddUser = currentGroup;
            groupFound = true;
            break;
        }
    }

    if (!groupFound) {
        groupToAddUser = Group();
        groupToAddUser.name = groupName;
    }

    UserConnection userConection;
    userConection.username = username;
    userConection.socket = socket;
    groupToAddUser.clients.push_back(userConection);
    if (!groupFound) {
        // TODO: groupToAddUser is not correctly appearing in this.groups
        // Only 1 client is receiving messages
        // Pegar o ponteiro direito dessa bosta. Vamos ver amanha.
        groups.push_back(groupToAddUser);
    }
    string joinMessage = username + " conectou!";
    communicationManager->sendMessageToClients(joinMessage, groupToAddUser.clients);
};

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}