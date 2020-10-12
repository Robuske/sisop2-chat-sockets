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

    this->connectedSocket = sockFd;

    return SUCCESSFUL_OPERATION;
}

int ClientCommunicationManager::writeSocketMessage(Message message) {
    Packet packet = message.asPacket();

    // TODO: Return result
    write(this->connectedSocket, &packet, sizeof(Packet));

    return 1;
}

int ClientCommunicationManager::writeConnectionMessageToSocket(Message message) {
    Packet packet = message.asPacket();

    // TODO: Throw or handle error in callers
    return write(this->connectedSocket, &packet, sizeof(Packet));
}

Message ClientCommunicationManager::readSocketMessage() {
    try {
        // Critical section, a princípio só temos uma thread de leitura, mas só pra garantir
        continuousBufferAccessControl.lock();
        Packet packet = continuousBufferRead(connectedSocket, continuousBuffer);
        continuousBufferAccessControl.unlock();
        return Message(packet);

    } catch (int readOperationResult) {
        continuousBufferAccessControl.unlock();

        if (readOperationResult == 0) {
            throw ERROR_SERVER_DISCONNECTED;

        } else {
            throw ERROR_SOCKET_READ;
        }
    }
}
