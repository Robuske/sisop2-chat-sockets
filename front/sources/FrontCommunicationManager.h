//
// Created by Henrique Valcanaia on 06/11/20.
//

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
    SocketFD setupServerSocket();
    void resetContinuousBufferFor(int socket);

    // Threads

    Packet readPacketFromSocket(SocketFD communicationSocket);

    int sendPacketToServerSocket(Packet packet, SocketFD socket);

    static void *staticHandleClientMessageThread(void *newClientArguments);

    static void *staticHandleServerMessageThread(void *newClientArguments);

    void *handleClientMessageThread(HandleNewClientArguments *args);

    void handleServerMessageThread(HandleNewClientArguments *pArguments);

    void forwardPacketFromSocketToSocket(SocketFD fromSocket, SocketFD toSocket);

    int connectToServer(const SocketConnectionInfo &connectionInfo);
};

#endif //SISOP2_T1_FRONTCOMMUNICATIONMANAGER_H
