#ifndef SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
#define SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H

#include <string>
#include <ClientDefinitions.h>

#define PORT 4000

using std::string;

typedef int SocketFD;

struct Message {
    uint16_t timestamp;
    string group;
    string username;
    string text;
};

struct PacketHeader {
    uint16_t type;
    uint16_t length;
};

struct Packet {
    struct PacketHeader header;
    char* _payload;
};

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
};


#endif //SISOP2_T1_CLIENTCOMMUNICATIONMANAGER_H
