#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include <ServerDefinitions.h>
#include <SharedDefinitions.h>
#include <list>

class ServerGroupsManager;
class ServerCommunicationManager;
struct UserConnection;

struct HandleNewClientArguments {
    ServerCommunicationManager *communicationManager;
    ServerGroupsManager *groupsManager;
    SocketFD newClientSocket;
};

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);

    void sendMessageToClients(string message, std::list<UserConnection> userConnections);

private:
    SocketFD setupServerSocket();
//    ServerGroupsManager *groupsManager;

    std::list<SocketFD> clients;

    static void *staticHandleNewClientConnection(void *newClientArguments);

    void *handleNewClientConnection(HandleNewClientArguments *args);

    bool handleReadResult(int readResult, int socket);
    void terminateClientConnection(SocketFD socketFileDescriptor, string username);
    PacketHeader readPacketHeaderFromSocket(SocketFD communicationSocket);
    Packet readPacketFromSocket(SocketFD communicationSocket, int packetSize);

};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
