#include "ClientCommunicationManager.h"
#include <netdb.h>
#include <unistd.h>

int ClientCommunicationManager::connectClient(const SocketConnectionInfo& connectionInfo) {

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

    this->socketConnectionResult = sockFd;

    return SUCCESSFUL_OPERATION;
}

int ClientCommunicationManager::writeSocketMessage(struct Message *message) {
//    struct Packet packet;
    struct PacketHeader packetHeader;

    packetHeader.type = TypeMessage;
    packetHeader.length = sizeof(Message);

    write(this->socketConnectionResult, &packetHeader, sizeof(PacketHeader));
    write(this->socketConnectionResult, message, sizeof(Message));

    return 1;
}

int ClientCommunicationManager::writeConnectionMessageToSocket(struct Message *message) {
    struct PacketHeader packetHeader;
    packetHeader.type = TypeConnection;
    packetHeader.length = sizeof(Packet);

    struct Packet packet;
    packet.payload = *message;

    write(this->socketConnectionResult, &packetHeader, sizeof(PacketHeader));
    write(this->socketConnectionResult, &packet, sizeof(Packet));

    return 1;
}

int ClientCommunicationManager::readSocketMessage(Message* message) {
    return read(this->socketConnectionResult, message, sizeof(Message));
}
