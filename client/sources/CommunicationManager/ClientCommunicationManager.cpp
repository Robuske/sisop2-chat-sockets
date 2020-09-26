#include "ClientCommunicationManager.h"

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
        fprintf(stderr,"ERROR, no such host\n");
        return INVALID_HOST_ERROR;
    }

    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("ERROR opening socket\n");
        return SOCKET_CREATION_ERROR;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(connectionInfo.port);
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockFd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        return SOCKET_CONNECTION_ERROR;
    }

    this->socketConnectionResult = sockFd;

    return SUCCESSFUL_OPERATION;
}

int ClientCommunicationManager::writeSocketMessage(struct Message *message) {
    struct Packet packet;
    struct PacketHeader packetHeader;

    int _message = 123;

    packetHeader.type = 0;
    packetHeader.length = 100;
    packet.header = packetHeader;
    packet._payload = (char*) malloc(100);
    packet._payload =  (char*) &_message;

    return write(this->socketConnectionResult, &packet, sizeof(packet));
}

int ClientCommunicationManager::readSocketMessage(char* message) {
    int bufferSize = 1;
    return read(this->socketConnectionResult, message, bufferSize);
}
