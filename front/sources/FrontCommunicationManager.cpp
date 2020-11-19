#include "FrontCommunicationManager.h"
#include "FrontDefinitions.h"
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

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

typedef std::map<SocketFD, ContinuousBuffer> ContinuousBuffersMap;
typedef std::map<SocketFD, std::mutex> ContinuousBufferAccessControl;
ContinuousBuffersMap continuousBuffers;
ContinuousBufferAccessControl continuousBufferAccessControl;

SocketFD FrontCommunicationManager::setupClientSocket() {
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

void FrontCommunicationManager::forwardPacketFromSocketToSocket(SocketFD fromSocket, SocketFD toSocket) {
    // TODO: Como decidir os sockets?
    // O server vai ter um socket fixo?
    // Cada front vai ter 1 socket?
    // Teremos mais de 1 front rodando?
    Packet packet;
    while (true) {
        packet = this->readPacketFromSocket(fromSocket);
        this->sendPacketToSocket(packet, toSocket);
    }
}

void *FrontCommunicationManager::staticHandleClientMessageThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleClientMessageThread(t);
    return nullptr;
}

void FrontCommunicationManager::handleClientMessageThread(HandleNewClientArguments *args) {
    // TODO: Precisamos de IP e porta pra identificar os participantes?
    this->forwardPacketFromSocketToSocket(args->socket, args->communicationManager->serverSocket);
}

void *FrontCommunicationManager::staticHandleServerMessageThread(void *newClientArguments) {
    auto* t = static_cast<HandleNewClientArguments*>(newClientArguments);
    t->communicationManager->handleServerMessageThread(t);
    return nullptr;
}

void FrontCommunicationManager::handleServerMessageThread(HandleNewClientArguments *args) {
    // TODO: Precisamos de IP e porta pra identificar os participantes?
    this->forwardPacketFromSocketToSocket(args->communicationManager->serverSocket, args->socket);
}

string FrontCommunicationManager::packetTypeAsString(PacketType packetType) {
    switch (packetType) {
        case TypeConnection:
            return "Connection";

        case TypeDesconnection:
            return "Disconnection";

        case TypeMessage:
            return "Message";

        case TypeKeepAlive:
            return "Keep Alive";

        case TypeMaxConnectionsReached:
            return "Max Connections Reached";
    }
}

void FrontCommunicationManager::logPacket(Packet packet) {
    bool debug = true;
    if (debug) {
        std::cout << "------------- Packet -----------" << std::endl;
        std::cout << "Type: " << packetTypeAsString(packet.type) << std::endl;
        std::cout << "Username: " << packet.username << std::endl;
        std::cout << "Group name: " << packet.groupName << std::endl;
        std::cout << "Timestamp: " << packet.timestamp << std::endl;
        std::cout << "Text: " << packet.text << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
}

int FrontCommunicationManager::sendPacketToSocket(Packet packet, SocketFD socket) {
    std::cout << "Enviando para socket " << socket << std::endl;
    logPacket(packet);
    return write(socket, &packet, sizeof(Packet));
}

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
    connectionSocketFDResult = this->setupClientSocket();
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