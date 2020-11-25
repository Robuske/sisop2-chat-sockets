#include "SharedDefinitions.h"
#include <unistd.h>
#include <iostream>

std::time_t now() {
    std::time_t now = std::time(nullptr);
    return now;
}

void clearScreen() {
    system("clear");
}

/// Pode dar throw
/// Tecnicamente uma leitura do buffer pode retornar menos dados do que o que foi pedido (apesar de não ser comum), se for o caso nós precisamos continuar lendo até ter um Packet completo.
Packet continuousBufferRead(SocketFD communicationSocket, ContinuousBuffer continuousBuffer) {
    auto packetSize = sizeof(Packet);
    while (true) {
        // Retorna um packet se já tiver dados suficientes disponíveis
        if (continuousBuffer.size() >= packetSize) {
            auto *packet = reinterpret_cast<Packet *>(continuousBuffer.data());
            // Remove do buffer os dados usados
            continuousBuffer.erase(continuousBuffer.begin(), continuousBuffer.begin() + packetSize);

            return *packet;
        }

        char receivingBuffer[packetSize];

        // Tenta ler packetSize bytes
        const int bytesRead = read(communicationSocket, &receivingBuffer, packetSize);

        if (bytesRead <= 0) {
            throw bytesRead;
        } else {
            // Salva novos dados adquiridos
            continuousBuffer.insert(continuousBuffer.end(), receivingBuffer, receivingBuffer + bytesRead);
        }
    }
}

string dateStringFromTimestamp(std::time_t timestamp) {
    struct tm *timeInfo = localtime (&timestamp);
    char strBuffer[20];
    strftime (strBuffer, 20,"%H:%M:%S",timeInfo);
    return strBuffer;
}

string packetTypeAsString(PacketType packetType) {
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

void logPacket(Packet packet) {
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