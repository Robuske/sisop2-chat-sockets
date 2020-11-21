#include "FrontCommunicationManager.h"
#include "FrontDefinitions.h"
#include "Message/Message.h"
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

SocketFD FrontCommunicationManager::setupConnectionSocketForPort(int port) {
    SocketFD connectionSocketFD;
    if ((connectionSocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    struct sockaddr_in connectionAddress{};
    connectionAddress.sin_family = AF_INET;
    connectionAddress.sin_port = htons(port);
    connectionAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(connectionSocketFD, (struct sockaddr *) &connectionAddress, sizeof(connectionAddress)) < 0)
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

/// Even though we only have one thread reading from one server, we need to be prepared in case a new server is elected and the front hasn't lost connection to the old one. So the front is always expecting new server connections.
void *FrontCommunicationManager::staticExpectServerConnectionThread(void *expectServerConnectionArgs) {
    auto* threadArguments = static_cast<ThreadArguments*>(expectServerConnectionArgs);

    socklen_t socketLength = sizeof(struct sockaddr_in);
    struct sockaddr_in serverAddress;
    SocketFD serverCommunicationSocketFD;
    pthread_t handleServerMessageThread;
    while (true) {
        if ((serverCommunicationSocketFD = accept(threadArguments->socket, (struct sockaddr *) &serverAddress, &socketLength)) == -1) {
            perror("Server Connection Accept Error");
            exit(EXIT_FAILURE);
        }
        // TODO: Maybe add a lock for the serverCommunicationSocket property?
        // Not setting to the correct value yet because we haven't started the read thread
        threadArguments->communicationManager->serverCommunicationSocket = -1;
        threadArguments->communicationManager->resetContinuousBufferFor(serverCommunicationSocketFD);

        struct ThreadArguments serverArgs;
        serverArgs.socket = serverCommunicationSocketFD;
        serverArgs.communicationManager = threadArguments->communicationManager;

        pthread_create(&handleServerMessageThread, nullptr, FrontCommunicationManager::staticHandleServerMessageThread, &serverArgs);
    }

    return nullptr;
}

void *FrontCommunicationManager::staticHandleClientMessageThread(void *newClientArguments) {
    auto* t = static_cast<ThreadArguments*>(newClientArguments);
    t->communicationManager->handleClientMessageThread(t);
    return nullptr;
}

void FrontCommunicationManager::handleClientMessageThread(ThreadArguments *args) {
    SocketFD clientSocket = args->socket;
    Packet packet;
    while (true) {
        try {
            packet = this->readPacketFromSocket(clientSocket);
            if (serverCommunicationSocket > 0) {
                packet.sender.clientSocket = clientSocket;
                packet.sender.frontID = this->frontID;
                std::cout << "Client sending message to server" << std::endl;
                sendPacketToSocket(packet, serverCommunicationSocket);
            } else {
                std::cout << "[WARNING] No server available, will wait for 1 second" << std::endl;
                // TODO: Send message to client asking to wait
                sleep(1);
            }
        } catch (int error) {
            std::cout << "[ERROR] Client read error: " << error << std::endl;

            if (error == ERROR_SOCKET_DISCONNECTED) {
                std::cout << "Will send disconnection message to server with last packet info:" << std::endl;
                std::cout << "socket: " << clientSocket << std::endl;
                std::cout << "username: " << packet.username << std::endl;
                std::cout << "groupName: " << packet.groupName << std::endl;

                Client client{ frontID, clientSocket };
                Message message = Message(TypeDisconnection, now(), client, client, packet.groupName, packet.username, "Desconectou!");
                sendPacketToSocket(message.asPacket(), serverCommunicationSocket);
            } else {
                std::cout << "Will NOT send disconnection message to server" << std::endl;
            }

            close(clientSocket);
            return;
        }
    }
}

void *FrontCommunicationManager::staticHandleServerMessageThread(void *newServerArguments) {
    auto* t = static_cast<ThreadArguments*>(newServerArguments);
    t->communicationManager->handleServerMessageThread(t);
    return nullptr;
}

void FrontCommunicationManager::handleServerMessageThread(ThreadArguments *args) {
    SocketFD serverSocket = args->socket;
    serverCommunicationSocket = serverSocket;

    Packet packet;
    while (true) {
        try {
            packet = this->readPacketFromSocket(serverCommunicationSocket);
            // This makes sure this still is the valid server
            if (serverCommunicationSocket == serverSocket) {
                if (packet.type == TypeDisconnection) {
                    // The SENDER is the one that was disconnected
                    close(packet.sender.clientSocket);
                }

                std::cout << "Server sending message to client" << std::endl;
                this->sendPacketToSocket(packet, packet.recipient.clientSocket);

                if (packet.type == TypeMaxConnectionsReached) {
                    std::cout << "Message was max connections, will close connection" << std::endl;
                    close(packet.sender.clientSocket);
                }

            } else {
                std::cout << "[WARNING]: Invalid server sending message" << std::endl;
                close(serverSocket);
                return;
            }
        } catch (int error) {
            std::cout << "Server read error: " << error << std::endl;
            // TODO: Handle somehow?
            if(serverCommunicationSocket == serverSocket) {
                serverCommunicationSocket = -1;
            }
            return;
        }
    }
}

string FrontCommunicationManager::packetTypeAsString(PacketType packetType) {
    switch (packetType) {
        case TypeConnection:
            return "Connection";

        case TypeDisconnection:
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
        std::cout << "Sender - frontID: " << packet.sender.frontID << std::endl;
        std::cout << "Sender - clientSocket: " << packet.sender.clientSocket << std::endl;
        std::cout << "Recipient - frontID: " << packet.recipient.frontID << std::endl;
        std::cout << "Recipient - clientSocket: " << packet.recipient.clientSocket << std::endl;
        std::cout << "Username: " << packet.username << std::endl;
        std::cout << "Group name: " << packet.groupName << std::endl;
        std::cout << "Timestamp: " << dateStringFromTimestamp(packet.timestamp) << std::endl;
        std::cout << "Text: " << packet.text << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
}

int FrontCommunicationManager::sendPacketToSocket(Packet packet, SocketFD socket) {
    std::cout << "Sending to socket " << socket << std::endl;
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
            throw ERROR_SOCKET_DISCONNECTED;

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
    // TODO: Get ID and ports from config file
    this->frontID = 15;
    int serverPort = PORT_FRONT_SERVER;
    int clientPort = PORT_FRONT_CLIENT;

    // ----- Server Setup
    this->serverCommunicationSocket = -1;
    SocketFD serverConnectionSocketFD;
    serverConnectionSocketFD = this->setupConnectionSocketForPort(serverPort);
    if (serverConnectionSocketFD < 0)
        return serverConnectionSocketFD;

    ThreadArguments args{};
    args.communicationManager = this;
    args.socket = serverConnectionSocketFD;

    pthread_t serverConnectionThread;
    pthread_create(&serverConnectionThread, nullptr, FrontCommunicationManager::staticExpectServerConnectionThread, &args);


    // ----- Client Setup
    SocketFD clientConnectionSocketFD;
    clientConnectionSocketFD = this->setupConnectionSocketForPort(clientPort);
    if (clientConnectionSocketFD < 0)
        return clientConnectionSocketFD;

    socklen_t socketLength = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddress;
    SocketFD clientCommunicationSocketFD;
    pthread_t handleClientMessageThread;
    while (true) {
        if ((clientCommunicationSocketFD = accept(clientConnectionSocketFD, (struct sockaddr *) &clientAddress, &socketLength)) == -1)
            return ERROR_SOCKET_ACCEPT_CLIENT_CONNECTION;

        resetContinuousBufferFor(clientCommunicationSocketFD);

        struct ThreadArguments clientArgs;
        clientArgs.socket = clientCommunicationSocketFD;
        clientArgs.communicationManager = this;

        pthread_create(&handleClientMessageThread, nullptr, FrontCommunicationManager::staticHandleClientMessageThread, &clientArgs);
    }

    return 0;
}
