//
// Created by Douglas Gehring on 20/11/20.
//

#ifndef SISOP2_T1_SERVERELECTIONMANAGER_H
#define SISOP2_T1_SERVERELECTIONMANAGER_H


#include "Message/Message.h"
#include "ServerDefinitions.h"
#include "SharedDefinitions.h"

class ServerCommunicationManager;


/** Each time a server start a new election
 * it needs to call setupElection() and then
 * send a message with its id to the current
 * connected server using the electionConnnection
 * socket
 */

class ServerElectionManager {

private:

    int elected;

    ServerCommunicationManager* communicationManager;

public:

    int serverID;

    SocketFD electionConnection;

    // Handle coordinator election
    void setupElection();

    void setupElectionCommunication(AvailableConnection serverConnection);

    void didReceiveElectionMessage(const string &candidateID);
    void didReceiveElectedMessage(const string &candidateID);

    int getElected();
    void setServerID(int serverID);
    void setElected(int electedID);

    void setupCoordinator();

    int sendMessageForCurrentElection(Message message);

    Message getFirstCandidateDefaultMessage();

    SocketConnectionInfo loadCoordinatorConnectionInfo();

    SocketConnectionInfo searchConnectionInfoForServerID(int serverID);
    void setupElectionManager(int serverID, ServerCommunicationManager* serverCommunicationManager);

    bool isCoordinator();
};


#endif //SISOP2_T1_SERVERELECTIONMANAGER_H
