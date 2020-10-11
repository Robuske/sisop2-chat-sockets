#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"
#include <list>
#include <map>
#include <mutex>

class ServerGroupsManager;
struct UserConnection;
class ServerCommunicationManager;

struct HandleNewClientArguments {
    ServerCommunicationManager *communicationManager;
    ServerGroupsManager *groupsManager;
    SocketFD newClientSocket;
};

typedef std::map<SocketFD, std::time_t> KeepAlive;
typedef std::map<SocketFD, std::mutex> KeepAliveAccessControl;

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);

    void sendMessageToClients(Message message, const std::list<UserConnection>& userConnections);

private:
    SocketFD setupServerSocket();

    KeepAlive socketsLastPing;
    KeepAlive socketsLastPong;

    KeepAliveAccessControl pingAccessControl;
    KeepAliveAccessControl pongAccessControl;

    void handleNewClientConnectionErrors(int errorCode,SocketFD communicationSocket, const string& username, ServerGroupsManager* groupsManager);
    static void *staticHandleNewClientConnection(void *newClientArguments);
    void *handleNewClientConnection(HandleNewClientArguments *args);
    void closeSocketConnection(SocketFD socket);
    bool handleReadResult(int readResult, int socketFileDescriptor);
    void terminateClientConnection(SocketFD socketFileDescriptor, string username, ServerGroupsManager* groupsManager);

    Packet readPacketFromSocket(SocketFD communicationSocket, int packetSize);

    static void *staticNewClientConnectionKeepAlive(void *newClientArguments);
    void *newClientConnectionKeepAlive(HandleNewClientArguments *args);
    bool shouldTerminateSocketConnection(SocketFD socket);

    void updateLastPingForSocket(SocketFD socket);
    void updateLastPongForSocket(SocketFD socket);
};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
