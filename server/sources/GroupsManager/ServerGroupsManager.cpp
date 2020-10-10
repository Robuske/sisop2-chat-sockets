#include "MessagesManager/ServerMessagesManager.h"
#include "ServerGroupsManager.h"

void ServerGroupsManager::sendMessage(const Message& message) {
    bool groupFound = false;
    string groupName = message.groupName;
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

void ServerGroupsManager::sendMessagesToSpecificUser(UserConnection userConnection, std::list<Message> messages, int loadedMessagesCount) {
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
    int numberOfLoadedMessages = messagesManager.loadInitialMessages(groupName, initialMessages, numberOfMessagesToLoadWhenUserJoined);
    this->sendMessagesToSpecificUser(userConnection, initialMessages, numberOfLoadedMessages);
}
#include <time.h>
// This can throw
void ServerGroupsManager::handleUserConnection(const string& username, SocketFD socket, const string& groupName) {
    UserConnection userConnection;
    userConnection.username = username;
    userConnection.socket = socket;

    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    bool groupFound = false;

    if(checkForUsersMaxConnections(username)) {
        this->handleUserConnectionLimitReached(username, groupName, userConnection);
        throw ERROR_MAX_USER_CONNECTIONS_REACHED;
    }

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

    // TODO: Confirmar se precisa persistir as mensagens de conexão/desconexão

    // TODO: Timestamp
    // https://www.epochconverter.com/programming/c
    //    time_t     now;
    //    struct tm  ts;
    //    char       buf[80];
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    //    ts = *localtime(&now);
    //    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    //    time(&now);
    Message message = Message(TypeConnection, 1234, groupName, username, "Conectou!");


    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);

}

// This can throw
void ServerGroupsManager::handleUserDisconnection(SocketFD socket, const string& username) {
    std::list<UserConnection> userConnectionsToSendConnectionMessage;
    string groupName;
    bool groupFound = false;

    for (Group &currentGroup:groups) {
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.socket == socket) {
                groupFound = true;
                currentGroup.clients.remove(currentUserConnection);
                userConnectionsToSendConnectionMessage = currentGroup.clients;
                groupName = currentGroup.name;
                break;
            }
        }
    }

    if (!groupFound) {
        throw ERROR_GROUP_NOT_FOUND;
    }

    // TODO: Timestamp
    Message message = Message(TypeDesconnection, 1234, groupName, username, "Desconectou!");
    communicationManager->sendMessageToClients(message, userConnectionsToSendConnectionMessage);
}

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}

void ServerGroupsManager::handleUserConnectionLimitReached(const string &username, const string &groupName, const UserConnection &userConnection) {
    Message message = Message(TypeMaxConnectionsReached, 1234, groupName, username, "Conexão recusada. Você já está conectado no número máximo de dispositivos (2)");
    std::list<Message> singleMessageList;
    singleMessageList.push_back(message);
    this->sendMessagesToSpecificUser(userConnection, singleMessageList, 0);
}

bool ServerGroupsManager::checkForUsersMaxConnections(const string &username) {
    int connectionsCount = 0;

    for (Group &currentGroup:groups) {
        for (UserConnection &currentUserConnection:currentGroup.clients) {
            if (currentUserConnection.username == username) {
                connectionsCount++;
            }
        }
    }

    return (connectionsCount >= MAX_CONNECTIONS_COUNT);
}