#include "MessagesManager/ServerMessagesManager.h"
#include "ServerGroupsManager.h"

void ServerGroupsManager::sendMessage(const Message& message) {
    bool groupFound = false;
    string groupName = message.groupName;
    Group groupToSendMessage;
    /// MARK: Critical session access nao deixamos, mas talvez sim

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

    // TODO: handle possible reading file exceptions
    int returnCode = messagesManager.writeMessage(message);

    communicationManager->sendMessageToClients(message, groupToSendMessage.clients);
}

/**
 * Function: ServerGroupsManager::sendMessagesToSpecificUser
 * Kinda dirty but this way we can send multiple messages to one specific user using a single user list
 * @param[in] userConnection, messages, loadedMessagesCount
 * @param[out] void
 */

void ServerGroupsManager::sendMessagesToSpecificUser(UserConnection userConnection, std::list<Message> messages) {
    std::list<UserConnection> singleUserConnectionList;
    singleUserConnectionList.push_back(userConnection);
    for (const auto& message:messages) {
        communicationManager->sendMessageToClients(message, singleUserConnectionList);
    }
}

/**
 * Function: ServerGroupsManager::loadInitialMessagesForNewUserConnection
 * Loading initial messages for user based on the number of messages to be loaded server side parameter
 * @param[in] userConnection, groupName
 * @param[out] void
 */

void ServerGroupsManager::loadInitialMessagesForNewUserConnection(UserConnection userConnection, const string& groupName) {
    std::list<Message> initialMessages;
    messagesManager.loadInitialMessages(groupName, initialMessages, numberOfMessagesToLoadWhenUserJoined);
    this->sendMessagesToSpecificUser(userConnection, initialMessages);
}

// This can throw
void ServerGroupsManager::handleUserConnection(const string& username, SocketFD socket, const string& groupName) {
    UserConnection userConnection;
    userConnection.username = username;
    userConnection.socket = socket;

    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    bool groupFound = false;


    /// MARK: Critical session access geralzao
    this->allGroupsAccessControl.lockAccessForGroup(ALL_GROUPS);
    /// Adicionar a verificacao de numero de conexoes dentro do mutex

    if(checkForUsersMaxConnections(username)) {
        this->allGroupsAccessControl.unlockAccessForGroup(ALL_GROUPS);
        this->handleUserConnectionLimitReached(username, groupName, userConnection);
        throw ERROR_MAX_USER_CONNECTIONS_REACHED;
    }

    for (Group &currentGroup:groups) {
        if (currentGroup.name == groupName) {
            groupFound = true;
            this->groupsListAccessControl.lockAccessForGroup(groupName);
            currentGroup.clients.push_back(userConnection);
            this->groupsListAccessControl.unlockAccessForGroup(groupName);
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

    this->allGroupsAccessControl.unlockAccessForGroup(ALL_GROUPS);

    this->loadInitialMessagesForNewUserConnection(userConnection, groupName);

    Message message = Message(TypeConnection, now(), groupName, username, "Conectou!");

    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);

}

// This can throw
void ServerGroupsManager::handleUserDisconnection(SocketFD socket, const string& username) {
    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    string groupName;
    bool groupFound = false;

    /// MARK: Critical session access
    this->allGroupsAccessControl.lockAccessForGroup(ALL_GROUPS);
    for (Group &currentGroup:groups) {
        /// MARK: Critical session access
        this->groupsListAccessControl.lockAccessForGroup(currentGroup.name);
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.socket == socket) {
                groupFound = true;
                groupName = currentGroup.name;
                currentGroup.clients.remove(currentUserConnection);
                userConnectionsToSendConnectionMessage = currentGroup.clients;
                break;
            }
        }
        this->groupsListAccessControl.unlockAccessForGroup(currentGroup.name);
    }

    this->allGroupsAccessControl.unlockAccessForGroup(ALL_GROUPS);

    if (!groupFound) {
        throw ERROR_GROUP_NOT_FOUND;
    }

    Message message = Message(TypeDesconnection, now(), groupName, username, "Desconectou!");
    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);
}

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}

void ServerGroupsManager::handleUserConnectionLimitReached(const string &username, const string &groupName, const UserConnection &userConnection) {
    Message message = Message(TypeMaxConnectionsReached, now(), groupName, username, "Conexão recusada. Você já está conectado no número máximo de dispositivos (" + std::to_string(MAX_CONNECTIONS_COUNT) + ")");
    std::list<Message> singleMessageList;
    singleMessageList.push_back(message);
    this->sendMessagesToSpecificUser(userConnection, singleMessageList);
}

bool ServerGroupsManager::checkForUsersMaxConnections(const string &username) {
    int connectionsCount = 0;

    for (Group &currentGroup:groups) {
        this->groupsListAccessControl.lockAccessForGroup(currentGroup.name);
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.username == username) {
                connectionsCount++;
            }
        }
        this->groupsListAccessControl.unlockAccessForGroup(currentGroup.name);
    }

    return (connectionsCount >= MAX_CONNECTIONS_COUNT);
}

string ServerGroupsManager::getUserNameForSocket(SocketFD socketFd) {
    this->allGroupsAccessControl.lockAccessForGroup(ALL_GROUPS);
    for (Group &currentGroup:groups) {
        this->groupsListAccessControl.lockAccessForGroup(currentGroup.name);
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.socket == socketFd) {
                this->groupsListAccessControl.unlockAccessForGroup(currentGroup.name);
                this->allGroupsAccessControl.unlockAccessForGroup(ALL_GROUPS);
                return currentUserConnection.username;
            }
        }
        this->groupsListAccessControl.unlockAccessForGroup(currentGroup.name);
    }
    this->allGroupsAccessControl.unlockAccessForGroup(ALL_GROUPS);

    // Precisa ser "", usar nullptr mata o server
    return "";
}
