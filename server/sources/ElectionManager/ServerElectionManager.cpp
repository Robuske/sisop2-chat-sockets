//
// Created by Douglas Gehring on 20/11/20.
//

#include "ServerElectionManager.h"
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <netdb.h>
#include <string>

#include "CommunicationManager/ServerCommunicationManager.h"


AvailableConnection serverConfigFile[SERVER_REPLICAS_NUMBER];

void loadServerConfigFile() {

    // Mock connections
    AvailableConnection fstConnection;
    fstConnection.id = 10;
    fstConnection.connectionInfo.ipAddress = "localhost";
    fstConnection.connectionInfo.port = 2000;

    AvailableConnection sndConnection;
    sndConnection.id = 20;
    sndConnection.connectionInfo.ipAddress = "localhost";
    sndConnection.connectionInfo.port = 2001;

    AvailableConnection thirdConnection;
    thirdConnection.id = 30;
    thirdConnection.connectionInfo.ipAddress = "localhost";
    thirdConnection.connectionInfo.port = 2002;

    AvailableConnection fourthConnection;
    fourthConnection.id = 40;
    fourthConnection.connectionInfo.ipAddress = "localhost";
    fourthConnection.connectionInfo.port = 2003;

    AvailableConnection fifthConnection;
    fifthConnection.id = 50;
    fifthConnection.connectionInfo.ipAddress = "localhost";
    fifthConnection.connectionInfo.port = 2004;

    serverConfigFile[0] = fstConnection;
    serverConfigFile[1] = sndConnection;
    serverConfigFile[2] = thirdConnection;
    serverConfigFile[3] = fourthConnection;
    serverConfigFile[4] = fifthConnection;
}

void ServerElectionManager::setupElectionManager(int serverID, ServerCommunicationManager* serverCommunicationManager) {
    loadServerConfigFile();
    this->setServerID(serverID);
    this->setupCoordinator();
    this->communicationManager = serverCommunicationManager;
}

void ServerElectionManager::setupElection() {
    // Acessar minha lista de conexoes disponivel
    // Se conectar com o socket no próximo
    // Mandar uma mensagem de start election pro proximo server da lista junto com o ID


    // Getting my index in the connections list using myID as reference
    int myIndex = 0;

    for (AvailableConnection &serverConnection: serverConfigFile) {
        if (serverConnection.id == this->serverID) break;
        myIndex++;
    }

    // Checking whats the server im about to connect

    int numberOfConnections =  sizeof(serverConfigFile)/sizeof(AvailableConnection);

    AvailableConnection nextConnection;

    int nextConnectionIndex =  ((numberOfConnections -1) == myIndex) ? 0 : (myIndex + 1);

    if(serverConfigFile[nextConnectionIndex].id == this->elected) {
        nextConnectionIndex = ((numberOfConnections -1) == nextConnectionIndex) ? 0 : (nextConnectionIndex + 1);
        nextConnection = serverConfigFile[nextConnectionIndex];
    } else {
        nextConnection = serverConfigFile[nextConnectionIndex];
    }

    this->elected = ELECTION_RUNNING;

    this->setupElectionCommunication(nextConnection);
}

void ServerElectionManager::setupCoordinator() {
    int electedID = serverConfigFile[0].id;
    this->elected = electedID;
}

bool ServerElectionManager::isCoordinator() {
    return (this->elected == this->serverID);
}

SocketConnectionInfo ServerElectionManager::loadCoordinatorConnectionInfo() {
    return this->searchConnectionInfoForServerID(this->elected);
}

SocketConnectionInfo ServerElectionManager::searchConnectionInfoForServerID(int serverID) {
    // Getting my index in the connections list using myID as reference
    int myIndex = 0;

    for (AvailableConnection &serverConnection: serverConfigFile) {
        if (serverConnection.id == serverID) break;
        myIndex++;
    }

    return serverConfigFile[myIndex].connectionInfo;
}


int ServerElectionManager::sendMessageForCurrentElection(Message message) {
    Packet packet = message.asPacket();
    return write(this->electionConnection, &packet, sizeof(Packet));
}

void ServerElectionManager::didReceiveElectionMessage(const string &candidateID) {

    // Assuming that the empty state of the elected attribute
    // is a number less than 0. If the elected attribute has an empty state
    // this means that the election have already started. Otherwise we need
    // to connect this server to its subsequent in the available connections
    // list.

    std::cout<< "Recebeu mensagem de ELEICAO com candidate " << candidateID.c_str() << std::endl;
    int currentCandidate = atoi(candidateID.c_str());
    if (currentCandidate > this->serverID) {
        if (this->elected != ELECTION_RUNNING) {
            this->setupElection();
            this->elected = ELECTION_RUNNING;
        }

        int nextCandidate = currentCandidate;
        Message message = Message(TypeElection, now(), clientNotSet, clientNotSet, "", "", std::to_string(nextCandidate));
        this->sendMessageForCurrentElection(message);
    } else if (currentCandidate == this->serverID) {
        std::cout << "Vou me enviar como eleito " << currentCandidate << std::endl;
        this->elected = this->serverID;

        Message message = Message(TypeElected, now(), clientNotSet, clientNotSet, "", "", candidateID);
        this->sendMessageForCurrentElection(message);
    } else if (currentCandidate < this->serverID) {
        if (this->elected != ELECTION_RUNNING) {
            this->setupElection();
            this->elected = ELECTION_RUNNING;

            int nextCandidate = this->serverID;
            Message message = Message(TypeElection, now(), clientNotSet, clientNotSet, "", "", std::to_string(nextCandidate));
            this->sendMessageForCurrentElection(message);
        } else {
            // do nothing baby
        }
    } else {
        std::cout << "Algo muito errado aconteceu e nao precerbemos" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ServerElectionManager::didReceiveElectedMessage(const string &candidateID) {
    std::cout << "Recebeu mensagem de ELEITO com candidate " << candidateID.c_str() << std::endl;

    this->elected = atoi(candidateID.c_str());
    if (this->elected == this->serverID) {
        std::cout << "Ganhou a eleicao" << this->elected << std::endl;
        // avisa o front
    } else {
        Message message = Message(TypeElected, now(), clientNotSet, clientNotSet, "", "", candidateID);
        this->sendMessageForCurrentElection(message);
    }

    close(this->electionConnection);
}

int ServerElectionManager::getElected() {
    return this->elected;
}

void ServerElectionManager::setupElectionCommunication(AvailableConnection serverConnection) {
    SocketFD connectionResult = this->communicationManager->performConnectionTo(serverConnection.connectionInfo);
    if (connectionResult < 0) {
        throw connectionResult;
    }
    this->electionConnection = connectionResult;
}

void ServerElectionManager::setElected(int electedID) {
    this->elected = electedID;
}

void ServerElectionManager::setServerID(int serverID) {
    this->serverID = serverID;
}

Message ServerElectionManager::getFirstCandidateDefaultMessage() {
    return Message(TypeElection, now(), clientNotSet, clientNotSet, "", "", std::to_string(this->serverID));
}
