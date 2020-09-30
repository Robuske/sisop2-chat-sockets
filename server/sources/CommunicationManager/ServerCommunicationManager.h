#ifndef SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
#define SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H

#include <ServerDefinitions.h>

typedef int SocketFD;

class ServerCommunicationManager {
public:
    int startServer(int loadMessageCount);

private:
    SocketFD setupServerSocket();
};


#endif //SISOP2_T1_SERVERCOMMUNICATIONMANAGER_H
