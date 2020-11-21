#include "SharedDefinitions.h"
#include <unistd.h>

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
