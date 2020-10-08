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

    // handle possible reading file exceptions
    int returnCode = messagesManager.writeMessage(message);

    communicationManager->sendMessageToClients(message, groupToSendMessage.clients);
}

/**
 * Function: ServerGroupsManager::sendMessagesToSpecificUser
 * Kinda dirty but this way we can send multiple messages to one specific user using a single user list
 * @param[in] userConnection, messages, loadedMessagesCount
 * @param[out] void
 */

void ServerGroupsManager::sendMessagesToSpecificUser(UserConnection userConnection, Message* messages, int loadedMessagesCount) {
    std::list<UserConnection> singleUserConnectionList;
    singleUserConnectionList.push_back(userConnection);
    for(int index = 0; index < loadedMessagesCount; index++) {
        communicationManager->sendMessageToClients(messages[index], singleUserConnectionList);
    }
}

/**
 * Function: ServerGroupsManager::loadInitialMessagesForNewUserConnection
 * Loading initial messages for user based on the number of messages to be loaded server side parameter
 * @param[in] userConnection, groupName
 * @param[out] void
 */

void ServerGroupsManager::loadInitialMessagesForNewUserConnection(UserConnection userConnection, const string& groupName) {
    Message initialMessages[numberOfMessagesToLoadWhenUserJoined];
    int numberOfLoadedMessages = messagesManager.loadInitialMessages(groupName, initialMessages, numberOfMessagesToLoadWhenUserJoined);
    this->sendMessagesToSpecificUser(userConnection, initialMessages, numberOfLoadedMessages);
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

    this->loadInitialMessagesForNewUserConnection(userConnection, groupName);

    Message message;
    message.username = username;
    message.text = username + " conectou!";
    message.timestamp = 1234;
    message.group = groupName;

    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);

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

    const string disconnectedMessage = disconnectedUsername + " desconectou!";
    Message message;
    message.username = disconnectedUsername;
    message.text = disconnectedMessage;
    message.timestamp = 1234;
    message.group = "";

    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);
}

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}