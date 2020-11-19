#ifndef SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H

#include "SharedDefinitions.h"

class FrontCommunicationManager;
struct HandleNewClientArguments {
    FrontCommunicationManager *communicationManager;
    SocketFD socket;
};

class FrontCommunicationManager {

public:
    int startFront();

private:
    SocketFD serverSocket;
    SocketFD setupClientSocket();
    void resetContinuousBufferFor(int socket);

    // Threads
    static void *staticHandleClientMessageThread(void *newClientArguments);
    void handleClientMessageThread(HandleNewClientArguments *args);
    static void *staticHandleServerMessageThread(void *newClientArguments);
    void handleServerMessageThread(HandleNewClientArguments *pArguments);

    Packet readPacketFromSocket(SocketFD communicationSocket);

    int sendPacketToSocket(Packet packet, SocketFD socket);

    void forwardPacketFromSocketToSocket(SocketFD fromSocket, SocketFD toSocket);

    int connectToServer(const SocketConnectionInfo &connectionInfo);

    void logPacket(Packet packet);

    string packetTypeAsString(PacketType packetType);
};

#endif //SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
