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

    // Ping/pong for keep alive
    KeepAlive socketsLastPing;
    KeepAlive socketsLastPong;
    void updateLastPingForSocket(SocketFD socket);
    void updateLastPongForSocket(SocketFD socket);

    // Mutexes
    KeepAliveAccessControl pingAccessControl;
    KeepAliveAccessControl pongAccessControl;

    // Threads
    static void *staticHandleNewClientConnectionThread(void *newClientArguments);
    void *handleNewClientConnection(HandleNewClientArguments *args);
    static void *staticNewClientConnectionKeepAliveThread(void *newClientArguments);
    void *newClientConnectionKeepAlive(HandleNewClientArguments *args);

    // Socket methods
    Packet readPacketFromSocket(SocketFD communicationSocket, int packetSize);

    //  Handle errors
    void handleNewClientConnectionErrors(int errorCode,SocketFD communicationSocket, const string& username, ServerGroupsManager* groupsManager);

    // Connection termination
    void terminateClientConnection(SocketFD socketFileDescriptor, string username, ServerGroupsManager* groupsManager);
    bool shouldTerminateSocketConnection(SocketFD socket);
    void closeSocketConnection(SocketFD socket);

};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
