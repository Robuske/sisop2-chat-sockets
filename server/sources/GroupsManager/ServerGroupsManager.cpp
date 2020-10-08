#include "ServerGroupsManager.h"
#include "MessagesManager/ServerMessagesManager.h"
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
        throw ERROR_GROUP_NOT_FOUND;
    }

    communicationManager->sendMessageToClients(message.text, groupToSendMessage.clients);
}

void ServerGroupsManager::sendMessagesToSpecificUser(SocketFD socket, Message* messages, int messagesCount) {

}

// This can throw
void ServerGroupsManager::handleUserConnection(const string& username, SocketFD socket, const string& groupName) {
    UserConnection userConnection;
    userConnection.username = username;
    userConnection.socket = socket;

    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    bool groupFound = false;
    for (Group &currentGroup:groups) {
        if (currentGroup.name == groupName) {
            groupFound = true;
            currentGroup.clients.push_back(userConnection);
            userConnectionsToSendConnectionMessage = currentGroup.clients;
            break;
        }
    }

    if (!groupFound) {
        Group newGroup = Group();
        newGroup.name = groupName;
        newGroup.clients.push_back(userConnection);
        groups.push_back(newGroup);
        userConnectionsToSendConnectionMessage = newGroup.clients;
    }

    const string joinMessage = username + " conectou!";
    communicationManager->sendMessageToClients(joinMessage, userConnectionsToSendConnectionMessage);
}

// This can throw
void ServerGroupsManager::handleUserDisconnection(SocketFD socket) {
    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    string disconnectedUsername;
    bool groupFound = false;
    for (Group &currentGroup:groups) {
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.socket == socket) {
                groupFound = true;
                currentGroup.clients.remove(currentUserConnection);
                userConnectionsToSendConnectionMessage = currentGroup.clients;
                disconnectedUsername = currentUserConnection.username;
                break;
            }
        }
    }

    if (!groupFound) {
        throw ERROR_GROUP_NOT_FOUND;
    }

    const string joinMessage = disconnectedUsername + " desconectou!";
    communicationManager->sendMessageToClients(joinMessage, userConnectionsToSendConnectionMessage);
}

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}