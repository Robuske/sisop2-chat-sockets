//
// Created by Henrique Valcanaia on 06/11/20.
//
#include <pthread.h>
#include "FrontCommunicationManager.h"
#include <map>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "FrontDefinitions.h"

#define ERROR_INVALID_HOST -3
#define ERROR_SOCKET_CONNECTION -4
#define SUCCESSFUL_OPERATION 1

int FrontCommunicationManager::connectToServer(const SocketConnectionInfo& connectionInfo) {

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
        return ERROR_SOCKET_CONNECTION;
    }

    this->serverSocket = sockFd;

    return SUCCESSFUL_OPERATION;
}

std::map<SocketFD, SocketFD> socketMap;

#define ERROR_SOCKET_BINDING -1
#define ERROR_SOCKET_ACCEPT_CONNECTION -2

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;
ContinuousBuffersMap continuousBuffers;
ContinuousBufferAccessControl continuousBufferAccessControl;

SocketFD FrontCommunicationManager::setupServerSocket() {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT_FRONT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(connectionSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
        return ERROR_SOCKET_BINDING;

    //    The backlog argument defines the maximum length to which the queue of
    //    pending connections for sockfd may grow.  If a connection request
    //    arrives when the queue is full, the client may receive an error with
    //    an indication of ECONNREFUSED or, if the underlying protocol supports
    //    retransmission, the request may be ignored so that a later reattempt
    //    at connection succeeds.
    int backlog = 100;
    listen(connectionSocketFD, backlog);

    return connectionSocketFD;
}

void *FrontCommunicationManager::staticHandleClientMessageThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleClientMessageThread(t);
    return nullptr;
}

void *FrontCommunicationManager::staticHandleServerMessageThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleServerMessageThread(t);
    return nullptr;
}

void FrontCommunicationManager::forwardPacketFromSocketToSocket(SocketFD fromSocket, SocketFD toSocket) {
    // TODO: Como decidir os sockets?
    // O server vai ter um socket fixo?
    // Cada front vai ter 1 socket?
    // Teremos mais de 1 front rodando?
    Packet packet;
    while (true) {
        packet = this->readPacketFromSocket(fromSocket);
        this->sendPacketToServerSocket(packet, toSocket);
    }
}

void *FrontCommunicationManager::handleClientMessageThread(HandleNewClientArguments *args) {
    // TODO: Precisamos de IP e porta pra identificar os participantes?
    this->forwardPacketFromSocketToSocket(args->socket, args->communicationManager->serverSocket);
}

void FrontCommunicationManager::handleServerMessageThread(HandleNewClientArguments *args) {
    // TODO: Precisamos de IP e porta pra identificar os participantes?
    this->forwardPacketFromSocketToSocket(args->communicationManager->serverSocket, args->socket);
}

int FrontCommunicationManager::sendPacketToServerSocket(Packet packet, SocketFD socket) {
    std::cout << "Enviando " << packet.text << " para " << socket << std::endl;
    return write(socket, &packet, sizeof(Packet));
}

#define ERROR_CLIENT_DISCONNECTED -3
Packet FrontCommunicationManager::readPacketFromSocket(SocketFD communicationSocket) {
    try {
        // Critical section
        continuousBufferAccessControl[communicationSocket].lock();
        ContinuousBuffer continuousBuffer = continuousBuffers[communicationSocket];
        Packet packet = continuousBufferRead(communicationSocket, continuousBuffer);
        continuousBufferAccessControl[communicationSocket].unlock();
        return packet;

    } catch (int readOperationResult) {
        continuousBufferAccessControl[communicationSocket].unlock();

        if (readOperationResult == 0) {
            throw ERROR_CLIENT_DISCONNECTED;

        } else {
            throw readOperationResult;
        }
    }
}

void FrontCommunicationManager::resetContinuousBufferFor(SocketFD socket) {
    continuousBufferAccessControl[socket].lock();
    continuousBuffers[socket].clear();
    continuousBufferAccessControl[socket].unlock();
}

#define PORT_SERVER 2000
int FrontCommunicationManager::startFront() {
    SocketConnectionInfo connectionInfo;
    connectionInfo.ipAddress = "localhost";
    connectionInfo.port = PORT_SERVER;

    int socketConnectionResult = this->connectToServer(connectionInfo);
    if (socketConnectionResult != SUCCESSFUL_OPERATION) {
        string errorPrefix = "Error(" + std::to_string(socketConnectionResult) + ") connecting client";
        perror(errorPrefix.c_str());
        return socketConnectionResult;
    }
    std::cout << "Server connection successful, socket: " << this->serverSocket << std::endl;

    // -------------------------------------------
    SocketFD connectionSocketFDResult;
    connectionSocketFDResult = this->setupServerSocket();
    if (connectionSocketFDResult < 0)
        return connectionSocketFDResult;

    socklen_t clientSocketLength;
    struct sockaddr_in clientAddress;
    SocketFD communicationSocketFD;
    pthread_t handleClientMessageThread, handleServerMessageThread;
    while (true) {
        clientSocketLength = sizeof(struct sockaddr_in);
        if ((communicationSocketFD = accept(connectionSocketFDResult, (struct sockaddr *) &clientAddress, &clientSocketLength)) == -1)
            return ERROR_SOCKET_ACCEPT_CONNECTION;

        resetContinuousBufferFor(communicationSocketFD);

        struct HandleNewClientArguments clientArgs;
        clientArgs.socket = communicationSocketFD;
        clientArgs.communicationManager = this;

        pthread_create(&handleServerMessageThread, nullptr, FrontCommunicationManager::staticHandleServerMessageThread, &clientArgs);
        pthread_create(&handleClientMessageThread, nullptr, FrontCommunicationManager::staticHandleClientMessageThread, &clientArgs);
    }

    return 0;
}