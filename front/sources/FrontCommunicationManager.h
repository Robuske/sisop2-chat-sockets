#ifndef SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H

#include "SharedDefinitions.h"
#include <map>
#include <mutex>

class FrontCommunicationManager;
struct HandleNewClientArguments {
    FrontCommunicationManager *communicationManager;
    SocketFD socket;
};

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;

class FrontCommunicationManager {

public:
    int startFront();

private:
    SocketFD serverSocket;
    SocketFD setupClientSocket();

    // Threads
    static void *staticHandleClientMessageThread(void *newClientArguments);
    void handleClientMessageThread(HandleNewClientArguments *args);
    static void *staticHandleServerMessageThread(void *newClientArguments);
    void handleServerMessageThread(HandleNewClientArguments *pArguments);

    // Continuous buffer
    ContinuousBuffersMap continuousBuffers;
    void resetContinuousBufferFor(int socket);

    // Mutexes
    ContinuousBufferAccessControl continuousBufferAccessControl;

    Packet readPacketFromSocket(SocketFD communicationSocket);
    int sendPacketToSocket(Packet packet, SocketFD socket);

    void forwardPacketFromSocketToSocket(SocketFD fromSocket, SocketFD toSocket);

    int connectToServer(const SocketConnectionInfo &connectionInfo);

    // Debug
    void logPacket(Packet packet);
    string packetTypeAsString(PacketType packetType);
};

#endif //SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
