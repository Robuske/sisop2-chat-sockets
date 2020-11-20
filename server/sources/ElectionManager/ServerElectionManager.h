//
// Created by Douglas Gehring on 20/11/20.
//

#ifndef SISOP2_T1_SERVERELECTIONMANAGER_H
#define SISOP2_T1_SERVERELECTIONMANAGER_H

#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"


/** Each time a server start a new election
 * it needs to call setupElection() and then
 * send a message with its id to the current
 * connected server using the electionConnnection
 * socket
 */

class ServerElectionManager {

private:
    AvailableConnection serverConnections[5];
    int myID;
    int elected;

public:

    unsigned short port;

    SocketFD electionConnection;

    // Handle servers replicas
    void loadAvailableServersConnections();

    // Handle coordinator election
    void setupElection();

    void mockConnectionsList();
    void setupElectionCommunication(AvailableConnection serverConnection);
     int connectServerToServer(const SocketConnectionInfo &connectionInfo);

    void didReceiveElectionMessage(const string &candidateID);
    void didReceiveElectedMessage(const string &candidateID);

    int getElected();
    void setMyID(int myID);
    void setElected(int electedID);

    int sendMessageForCurrentElection(Message message);

    Message getFirstCandidateDefaultMessage();
};


#endif //SISOP2_T1_SERVERELECTIONMANAGER_H
