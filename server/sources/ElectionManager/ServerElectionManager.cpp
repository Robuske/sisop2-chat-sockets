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

void ServerElectionManager::mockConnectionsList() {

    // Mock connections
    AvailableConnection fstConnection;
    fstConnection.id = 219;
    fstConnection.connectionInfo.ipAddress = "localhost";
    fstConnection.connectionInfo.port = 2000;

    AvailableConnection  sndConnection;
    sndConnection.id = 12;
    sndConnection.connectionInfo.ipAddress = "localhost";
    sndConnection.connectionInfo.port = 2001;

    AvailableConnection  thirdConnection;
    thirdConnection.id = 45;
    thirdConnection.connectionInfo.ipAddress = "localhost";
    thirdConnection.connectionInfo.port = 2002;


    AvailableConnection  fourthConnection;
    fourthConnection.id = 6;
    fourthConnection.connectionInfo.ipAddress = "localhost";
    fourthConnection.connectionInfo.port = 2003;

    AvailableConnection  fifthConnection;
    fifthConnection.id = 100;
    fifthConnection.connectionInfo.ipAddress = "localhost";
    fifthConnection.connectionInfo.port = 2004;

    this->serverConnections[0] = fstConnection;
    this->serverConnections[1] = sndConnection;
    this->serverConnections[2] = thirdConnection;
    this->serverConnections[3] = fourthConnection;
    this->serverConnections[4] = fifthConnection;
}


// MARK: - Instance methods
void ServerElectionManager::loadAvailableServersConnections() {
//    string path = "servers.txt";
//    std::ifstream file(path.c_str());
//
//    // Defining the file size
//    // To do so we need to set the file pointer to the EOF
//
//    const auto begin = file.tellg();
//    file.seekg (0, std::ios::end);
//    const auto end = file.tellg();
//    const long long fileSize = end-begin;
//
//    //Rewinding file the file pointer previously located at the EOF
//    file.seekg(0, std::ios::beg);
//
//    char *buffer = new char[fileSize]();
//    file.read(buffer, fileSize);
//    file.close();
    this->mockConnectionsList();

    //this->serverConnections = (AvailableConnection*) buffer;
}

// When we are the responsibles

void ServerElectionManager::setupElection() {
    // Acessar minha lista de conexoes disponivel
    // Se conectar com o socket no próximo
    // Mandar uma mensagem de start election pro proximo server da lista junto com o ID


    // Getting my index in the connections list using myID as reference
    int myIndex = 0;

    for (AvailableConnection &serverConnection:this->serverConnections) {
        if (serverConnection.id == this->myID) break;
        myIndex++;
    }

    // Checking whats the server im about to connect

    int numberOfConnections =  sizeof(this->serverConnections)/sizeof(AvailableConnection);

    AvailableConnection nextConnetion;

    int nextConnectionIndex =  ((numberOfConnections -1) == myIndex) ? 0 : (myIndex + 1);

    if(this->serverConnections[nextConnectionIndex].id == this->elected) {
        nextConnectionIndex =  ((numberOfConnections -1) == nextConnectionIndex) ? 0 : (nextConnectionIndex + 1);
        nextConnetion = this->serverConnections[nextConnectionIndex];
    } else {
        nextConnetion = this->serverConnections[nextConnectionIndex];
    }

    this->elected = -1;

    this->setupElectionCommunication(nextConnetion);
}


int ServerElectionManager::connectServerToServer(const SocketConnectionInfo& connectionInfo) {

    SocketFD socketFD;
    struct sockaddr_in front_addr{};
    struct hostent *front;

    front = gethostbyname(connectionInfo.ipAddress.c_str());
    if (front == nullptr) {
        string errorPrefix = "Error no such host '" + connectionInfo.ipAddress + "'";
        perror(errorPrefix.c_str());
        return ERROR_INVALID_HOST;
    }

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        string errorPrefix = "Error(" + std::to_string(socketFD) + ") opening socket";
        perror(errorPrefix.c_str());
        return ERROR_SOCKET_CREATION;
    }

    front_addr.sin_family = AF_INET;
    front_addr.sin_port = htons(connectionInfo.port);
    front_addr.sin_addr = *((struct in_addr *)front->h_addr);

    int connectionResult = connect(socketFD, (struct sockaddr *) &front_addr, sizeof(front_addr));
    if (connectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(connectionResult) + ") connecting";
        perror(errorPrefix.c_str());
        return ERROR_CONNECTING_SOCKET_SERVER_TO_SERVER;
    }

    return socketFD;
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

    std::cout<< "Recebeu mensagem de eleicao com candidate" << candidateID.c_str() << std::endl;

    if(this->elected > 0) {
        this->setupElection();
    }

    int currentCandidate = atoi(candidateID.c_str());

    if(currentCandidate == this->myID) {
        // Send elected message
        Message message = Message(TypeElected, now(), "", "", candidateID);
        this->sendMessageForCurrentElection(message);
    } else {
        // Find next candidate
        int nextCandidate = std::max(currentCandidate, this->myID);
        Message message = Message(TypeElection, now(), "", "", std::to_string(nextCandidate));
        this->sendMessageForCurrentElection(message);
    }
}

void ServerElectionManager::didReceiveElectedMessage(const string &candidateID) {

    std::cout<<"Recebeu mensagem de eleito com candidate"<<candidateID.c_str()<< std::endl;

    this->elected = atoi(candidateID.c_str());
    if(this->elected == this->myID) {
        // do nothing and close socket
        std::cout << "Ganhou a eleicao" << this->elected << std::endl;
       // close(this->electionConnection);
    } else {
        Message message = Message(TypeElected, now(), "", "", candidateID);
        this->sendMessageForCurrentElection(message);
    }
}

int ServerElectionManager::getElected() {
    return this->elected;
}

void ServerElectionManager::setupElectionCommunication(AvailableConnection serverConnection) {
    SocketFD connectionResult = this->connectServerToServer(serverConnection.connectionInfo);
    if (connectionResult < 0) {
        throw connectionResult;
    }
    this->electionConnection = connectionResult;
}

void ServerElectionManager::setElected(int electedID) {
    this->elected = electedID;
}

void ServerElectionManager::setMyID(int myID) {
    this->myID = myID;
}

Message ServerElectionManager::getFirstCandidateDefaultMessage() {
    return Message(TypeElection, now(), "", "", std::to_string(this->myID));
}
