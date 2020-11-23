#include "CommunicationManager/ServerCommunicationManager.h"
#include <iostream>
#include <string>

// ./server [load messages count] [id]
int main(int argc, char *argv[]) {
    std::cout << "Use um ID que esteja em ServerElectionManager.loadServerConfigFile()" << std::endl;

    if (argc != 3) {
        std::cout << "Usage: server [number of messages to load] [server id]" << std::endl;
        std::cout << "Invalid arguments number" << std::endl;
        exit(EXIT_FAILURE);
    }

    int loadMessageCount = atoi(argv[1]);
    if (loadMessageCount < 0) {
        loadMessageCount = 0;
    }

    int myId = atoi(argv[2]);
    if (myId <= 0) {
        std::cout << "Invalid ID (" << myId << "). Server IDs must be a positive number" << std::endl;
        exit(EXIT_FAILURE);
    }

    clearScreen();

    std::cout << "Iniciando servidor Zap 2.0" << std::endl;
    std::cout << "My ID: " << myId << std::endl;
    std::cout << "Vai conectar no front hardcoded: " << PORT_FRONT_SERVER << std::endl;
    std::cout << "Enviando as # últimas mensagens enviadas no grupo: " << std::to_string(loadMessageCount) << std::endl;

    int startServerResult = ServerCommunicationManager().startServer(loadMessageCount, myId);
    if (startServerResult < 0) {
        string errorPrefix = "Erro (" + std::to_string(startServerResult) + ") iniciando server";
        perror(errorPrefix.c_str());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
