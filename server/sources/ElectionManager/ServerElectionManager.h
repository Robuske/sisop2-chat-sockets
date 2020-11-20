//
// Created by Douglas Gehring on 20/11/20.
//

#ifndef SISOP2_T1_SERVERELECTIONMANAGER_H
#define SISOP2_T1_SERVERELECTIONMANAGER_H

#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"

class ServerElectionManager {

private:
    AvailableConnection serverConnections[4];
    int myID = 40;
    int elected = 219;

public:
    // Handle servers replicas

    void loadAvailableServersConnections();

    // Handle coordinator election
    void startElection();

    void mockConnectionsList();
    void handleElectionCommunication(AvailableConnection serverConnection);

    int connectServerToServer(const SocketConnectionInfo &connectionInfo);
};


#endif //SISOP2_T1_SERVERELECTIONMANAGER_H
