#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"
#include "ElectionManager/ServerElectionManager.h"
#include <list>
#include <map>
#include <mutex>
#include <vector>

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

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);
    int handleElectedStatus(string electedID);

    void sendMessageToClients(Message message, const std::list<UserConnection>& userConnections);

private:
    SocketFD setupServerSocket();

    // Ping/pong for keep alive
    KeepAlive socketsLastPing;
    KeepAlive socketsLastPong;
    void updateLastPingForSocket(SocketFD socket);
    void updateLastPongForSocket(SocketFD socket);

    // Continuous buffer
    ContinuousBuffersMap continuousBuffers;
    void resetContinuousBufferFor(SocketFD socket);

    // Mutexes
    KeepAliveAccessControl pingAccessControl;
    KeepAliveAccessControl pongAccessControl;
    ContinuousBufferAccessControl continuousBufferAccessControl;

    // Threads
    static void *staticHandleNewClientConnectionThread(void *newClientArguments);
    void *handleNewClientConnection(HandleNewClientArguments *args);
    static void *staticNewClientConnectionKeepAliveThread(void *newClientArguments);
    void *newClientConnectionKeepAlive(HandleNewClientArguments *args);

    // Socket methods
    Packet readPacketFromSocket(SocketFD communicationSocket);

    //  Handle errors
    void handleNewClientConnectionErrors(int errorCode,SocketFD communicationSocket, const string& username, ServerGroupsManager* groupsManager);

    // Connection termination
    void terminateClientConnection(SocketFD socketFileDescriptor, string username, ServerGroupsManager* groupsManager);
    bool shouldTerminateSocketConnection(SocketFD socket);
    void closeSocketConnection(SocketFD socket);

    // Lider election

    ServerElectionManager electionManager;
};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
