#include "ServerGroupsManager.h"

// let groups: [Group]

// This can throws
void ServerGroupsManager::handleUserConnection(string username, SocketFD socket, string group) {
    // let grupo: Group
    // if groups nao tem (group) {
    //      grupo = novo Group(name: group)
    //      groups.append(grupo);
    // } else {
    //      grupo = findGrupo(group);
    // }
    // grupo.addClient(socket, username);
    // joinMessage = messageManager.newUserJoinedGroup(userame, grupo);
    // for client in group.clients {
    //     communicationManager.sendMessageToClient(client, joinMessage);
    // }
};

ServerGroupsManager::ServerGroupsManager(int numberOfMessagesToLoadWhenUserJoined, ServerCommunicationManager *communicationManager) {
    this->numberOfMessagesToLoadWhenUserJoined = numberOfMessagesToLoadWhenUserJoined;
    this->communicationManager = communicationManager;
}