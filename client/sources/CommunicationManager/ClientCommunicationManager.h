#ifndef SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H

#include "ClientDefinitions.h"
#include "SharedDefinitions.h"
#include <string>

using std::string;

typedef int SocketFD;

struct SocketConnectionInfo {
    string ipAddress;
    unsigned short port;
};

class ClientCommunicationManager {

private:
    SocketFD socketConnectionResult;

public:
    SocketFD connectClient(const SocketConnectionInfo& connectionInfo);
    int writeSocketMessage(struct Message *message);
    int readSocketMessage(Message *message);

    int writeConnectionMessageToSocket(Message *message);
};


#endif //SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
