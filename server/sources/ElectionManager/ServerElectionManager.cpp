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

    AvailableConnection  forthConnection;
    forthConnection.id = 40;
    forthConnection.connectionInfo.ipAddress = "localhost";
    forthConnection.connectionInfo.port = 2003;

    this->serverConnections[0] = fstConnection;
    this->serverConnections[1] = sndConnection;
    this->serverConnections[2] = thirdConnection;
    this->serverConnections[3] = forthConnection;
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

void ServerElectionManager::startElection() {
    // Acessar minha lista de conexoes disponivel
    // Se conectar com o socket no próximo
    // Mandar uma mensagem de start election pro proximo server da lista junto com o ID


    // Getting my index in the connections list using myID as reference
    int myIndex = 0;

    for (AvailableConnection &serverConnection:this->serverConnections) {
        if (serverConnection.id == this->myID) break;
        myIndex++;
    }

    // Checking if its the last index in the array

    int numberOfConnections =  sizeof(this->serverConnections)/sizeof(AvailableConnection);

    AvailableConnection nextConnetion;

    // Means that im the last connection in the list
    if(myIndex == (numberOfConnections -1)) {
        nextConnetion = this->serverConnections[0];
    } else {
        nextConnetion = this->serverConnections[myIndex + 1];
    }

    this->handleElectionCommunication(nextConnetion);
}

int ServerElectionManager::connectServerToServer(const SocketConnectionInfo& connectionInfo) {
    SocketFD sockFd;
    struct sockaddr_in serv_addr{};
    struct hostent *server;

    server = gethostbyname(connectionInfo.ipAddress.c_str());
    if (server == nullptr) {
        string errorPrefix = "Error no such host '" + connectionInfo.ipAddress + "'";
        perror(errorPrefix.c_str());
        return ERROR_INVALID_HOST;
    }

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        string errorPrefix = "Error(" + std::to_string(sockFd) + ") opening socket";
        perror(errorPrefix.c_str());
        return ERROR_SOCKET_CREATION;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(connectionInfo.port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);

    int connectionResult = connect(sockFd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (connectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(connectionResult) + ") connecting";
        perror(errorPrefix.c_str());
        return ERROR_CONNECTING_SOCKET_SERVER_TO_SERVER;
    }

    return connectionResult;
}

void ServerElectionManager::handleElectionCommunication(AvailableConnection serverConnection) {

    SocketFD connectionResult = this->connectServerToServer(serverConnection.connectionInfo);


}