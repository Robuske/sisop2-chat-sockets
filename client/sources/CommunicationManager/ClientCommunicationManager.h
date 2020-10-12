#ifndef SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H

#include "ClientDefinitions.h"
#include "Message/Message.h"
#include "SharedDefinitions.h"
#include <mutex>
#include <string>

using std::string;

struct SocketConnectionInfo {
    string ipAddress;
    unsigned short port;
};

class ClientCommunicationManager {

private:
    SocketFD connectedSocket;
    ContinuousBuffer continuousBuffer;
    std::mutex continuousBufferAccessControl;

public:
    SocketFD connectClient(const SocketConnectionInfo& connectionInfo);
    int writeSocketMessage(Message message);
    Message readSocketMessage();

    int writeConnectionMessageToSocket(Message message);
};


#endif //SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
