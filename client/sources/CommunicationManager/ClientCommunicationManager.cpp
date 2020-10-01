#include "ClientCommunicationManager.h"
#include "SharedDefinitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>

int ClientCommunicationManager::connectClient(SocketConnectionInfo connectionInfo) {

    SocketFD sockFd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(connectionInfo.ipAddress.c_str());
    if (server == NULL) {
        string errorPrefix = "Error no such host '" + connectionInfo.ipAddress + "'";
        perror(errorPrefix.c_str());
        return INVALID_HOST_ERROR;
    }

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        string errorPrefix = "Error(" + std::to_string(sockFd) + ") opening socket";
        perror(errorPrefix.c_str());
        return SOCKET_CREATION_ERROR;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(connectionInfo.port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    int connectionResult = connect(sockFd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    if (connectionResult < 0) {
        string errorPrefix = "Error(" + std::to_string(connectionResult) + ") connecting";
        perror(errorPrefix.c_str());
        return SOCKET_CONNECTION_ERROR;
    }

    this->socketConnectionResult = sockFd;

    return SUCCESSFUL_OPERATION;
}

int ClientCommunicationManager::writeSocketMessage(struct Message *message) {
//    struct Packet packet;
    struct PacketHeader packetHeader;

    packetHeader.type = 0;
    packetHeader.length = sizeof(Message);

   write(this->socketConnectionResult, &packetHeader, sizeof(PacketHeader));
   write(this->socketConnectionResult, message, sizeof(Message));

   return 1;
}

int ClientCommunicationManager::readSocketMessage(char* message) {
    int bufferSize = 256;
    return read(this->socketConnectionResult, message, bufferSize);
}
