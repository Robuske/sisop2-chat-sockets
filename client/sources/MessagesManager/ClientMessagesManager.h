#ifndef SISOP2_T1_CLIENTMESSAGESMANAGER_H
#define SISOP2_T1_CLIENTMESSAGESMANAGER_H

#include "ClientDefinitions.h"
#include "CommunicationManager/ClientCommunicationManager.h"
#include "UI/ClientUI.h"
#include "SharedDefinitions.h"
#include <string>

using std::string;

typedef int SocketFD;

class ClientMessagesManager {

public:
    int startClient(const SocketConnectionInfo& connectionInfo, UserInfo userInfo);

private:
    UserInfo userInfo;
    ClientCommunicationManager communicationManager;
    ClientUI clientUI;

    void *readMessagesThread();
    static void *staticReadMessagesThread(void *threadParm);
    void *writeMessagesThread();
    static void *staticWriteMessagesThread(void *threadParm);

    void sendKeepAliveMessage();
};


#endif // SISOP2_T1_CLIENTMESSAGESMANAGER_H
