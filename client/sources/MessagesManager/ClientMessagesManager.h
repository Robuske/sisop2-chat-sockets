#ifndef SISOP2_T1_CLIENTMESSAGESMANAGER_H
#define SISOP2_T1_CLIENTMESSAGESMANAGER_H

#include <string>
#include <ClientDefinitions.h>

#define PORT 4000

using std::string;

typedef int SocketFD;

struct SocketConnectionInfo {
    string ipAddress;
    unsigned short port;
};

struct UserInfo {
    string name;
    string group;
};

class ClientMessagesManager {

public:
    int startClient(SocketConnectionInfo connectionInfo, UserInfo userInfo);

private:

    string userName;
    SocketFD connectClient(SocketConnectionInfo connectionInfo);
    int writeUserFirstMessage(SocketFD socketFd);
    void *readMessagesThread(int my_socket);
    static void *staticReadMessagesThread(void *threadParm);
    void *writeMessagesThread(int my_socket);
    static void *staticWriteMessagesThread(void *threadParm);
};


#endif //SISOP2_T1_CLIENTMESSAGESMANAGER_H
