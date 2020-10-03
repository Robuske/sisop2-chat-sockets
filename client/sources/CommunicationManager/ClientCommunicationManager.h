#ifndef SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H

#include <string>
#include <ClientDefinitions.h>
#include <SharedDefinitions.h>

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
    SocketFD connectClient(SocketConnectionInfo connectionInfo);
    int writeSocketMessage(struct Message *message);
    int readSocketMessage(char *message);

    int writeConnectionMessageToSocket(Message *message);
};


#endif //SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
