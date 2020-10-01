#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include <ServerDefinitions.h>
#include <SharedDefinitions.h>
#include <list>

class ServerGroupsManager;
class ServerCommunicationManager;

struct HandleNewClientArguments {
    ServerCommunicationManager *communicationManager;
    ServerGroupsManager *groupsManager;
    SocketFD newClientSocket;
};

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);

private:
    SocketFD setupServerSocket();

    std::list<SocketFD> clients;

    static void *staticHandleNewClientConnection(void *newClientArguments);

    void *handleNewClientConnection(HandleNewClientArguments *args);

    bool handleReadResult(int readResult, int socket);
    void terminateClientConnection(SocketFD socketFileDescriptor, string username);
    PacketHeader readPacketHeaderFromSocket(SocketFD communicationSocket);
    Packet readPacketFromSocket(SocketFD communicationSocket, int packetSize);

};

#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
