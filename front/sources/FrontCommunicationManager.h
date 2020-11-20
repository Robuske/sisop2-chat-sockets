#ifndef SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H

#include "SharedDefinitions.h"
#include <map>
#include <mutex>

class FrontCommunicationManager;

struct ThreadArguments {
    FrontCommunicationManager *communicationManager;
    SocketFD socket;
};

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;

class FrontCommunicationManager {

public:
    int startFront();

private:
    int frontID;

    // Connection
    SocketFD serverCommunicationSocket;
    SocketFD setupConnectionSocketForPort(int port);

    // Threads
    static void *staticExpectServerConnectionThread(void *expectServerConnectionArgs);
    static void *staticHandleServerMessageThread(void *newServerArguments);
    void handleServerMessageThread(ThreadArguments *pArguments);
    static void *staticHandleClientMessageThread(void *newClientArguments);
    void handleClientMessageThread(ThreadArguments *args);

    // Continuous buffer
    ContinuousBuffersMap continuousBuffers;
    void resetContinuousBufferFor(int socket);

    // Mutexes
    ContinuousBufferAccessControl continuousBufferAccessControl;

    Packet readPacketFromSocket(SocketFD communicationSocket);
    int sendPacketToSocket(Packet packet, SocketFD socket);

    // Debug
    void logPacket(Packet packet);
    string packetTypeAsString(PacketType packetType);
};

#endif //SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
