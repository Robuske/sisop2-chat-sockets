#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"
#include <list>

class ServerGroupsManager;
struct UserConnection;
class ServerCommunicationManager;

struct HandleNewClientArguments {
    ServerCommunicationManager *communicationManager;
    ServerGroupsManager *groupsManager;
    SocketFD newClientSocket;
};

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);

    void sendMessageToClients(Message message, const std::list<UserConnection>& userConnections);

private:
    SocketFD setupServerSocket();

    std::list<SocketFD> clients;

    static void *staticHandleNewClientConnection(void *newClientArguments);
    void *handleNewClientConnection(HandleNewClientArguments *args);

    void terminateClientConnection(SocketFD socketFileDescriptor, string username, ServerGroupsManager* groupsManager);

    Packet readPacketFromSocket(SocketFD communicationSocket, int packetSize);

    static void *staticNewClientConnectionKeepAlive(void *newClientArguments);
    void *newClientConnectionKeepAlive(HandleNewClientArguments *args);
    bool shouldTerminateSocketConnection(SocketFD socket);
};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
