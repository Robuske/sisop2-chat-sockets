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
class ServerCommunicationManager;

struct ThreadArguments {
    ServerCommunicationManager *communicationManager;
    SocketFD socket;
};

struct KeepAliveThreadArguments {
    UserConnection userConnection;
    ServerCommunicationManager *communicationManager;
};

typedef std::map<Client, std::time_t> ClientKeepAlive;
typedef std::map<Client, std::mutex> ClientKeepAliveAccessControl;

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount, int serverID);
    void sendMessageToClients(Message message, const std::list<UserConnection>& userConnections);

private:

    SocketFD openServerToServerPort(unsigned short port);
    int connectToFront(const SocketConnectionInfo& connectionInfo);
    void setupFronts();
    int setupServerToServerConnection(SocketConnectionInfo connectionInfo);

    ServerGroupsManager *groupsManager;

    // Ping/pong for keep alive
    ClientKeepAlive clientsLastPing;
    ClientKeepAlive clientsLastPong;
    void updateLastPingForClient(Client client);
    void updateLastPongForClient(Client client);

    // Continuous buffer
    ContinuousBuffersMap continuousBuffers;
    void resetContinuousBufferFor(SocketFD socket);

    // Mutexes
    ClientKeepAliveAccessControl pingAccessControl;
    ClientKeepAliveAccessControl pongAccessControl;
    ContinuousBufferAccessControl continuousBufferAccessControl;

    // Threads
    static void *staticHandleNewFrontConnectionThread(void *newClientArguments);
    void *handleNewFrontConnectionThread(ThreadArguments *args);
    static void *staticNewClientConnectionKeepAliveThread(void *newClientArguments);
    void *newClientConnectionKeepAlive(KeepAliveThreadArguments *args);

    static void *staticHandleNewServerConnectionThread(void *newClientArguments);
    void *handleNewServerConnectionThread(ThreadArguments *args);

    // Socket methods
    Packet readPacketFromSocket(SocketFD communicationSocket);

    //  Handle errors
    void handleNewClientConnectionErrors(int errorCode, SocketFD frontCommunicationSocket, const string &username);

    // Connection termination
    void terminateClientConnection(UserConnection userConnection, ServerGroupsManager *groupsManager);
    bool shouldTerminateClientConnection(Client client);

    // Election
    ServerElectionManager electionManager;
    void startTestElection();

    void setupMainConnection();
};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
