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

int ClientCommunicationManager::writeSocketMessage(Message message) {
    Packet packet = message.asPacket();
//    packetHeader.type = TypeMessage;
//    packetHeader.length = sizeof(Message);

    // TODO: Return result
    write(this->socketConnectionResult, &packet, sizeof(Packet));
//    write(this->socketConnectionResult, message, sizeof(Message));

    return 1;
}

int ClientCommunicationManager::writeConnectionMessageToSocket(Message message) {
//    struct PacketHeader packetHeader;
//    packetHeader.type = TypeConnection;
//    packetHeader.length = sizeof(Packet);

    Packet packet = message.asPacket();
//    packet.payload = *message;

//    write(this->socketConnectionResult, &packet, sizeof(PacketHeader));
    // TODO: Return result
    write(this->socketConnectionResult, &packet, sizeof(Packet));

    return 1;
}

Message ClientCommunicationManager::readSocketMessage() {
    auto *packet = static_cast<Packet *>(malloc(sizeof(Packet)));

    int readResult = read(this->socketConnectionResult, packet, sizeof(Packet));
    if (readResult < 0) {
        throw ERROR_SOCKET_READ;
    }

    return Message(*packet);
}
