#include "CommunicationManager/ServerCommunicationManager.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: server <number of messages to load>";
        exit(EXIT_FAILURE);
    }

    int loadMessageCount = atoi(argv[1]);
    if (loadMessageCount < 0) {
        loadMessageCount = 0;
    }

    std::cout << "Iniciando servidor Zap" << std::endl;
    std::cout << "Porta: " << PORT << std::endl;
    std::cout << "Enviando as # últimas mensagens enviadas no grupo: " << std::to_string(loadMessageCount) << std::endl;

    int startServerResult = ServerCommunicationManager().startServer(loadMessageCount);
    if (startServerResult < 0) {
        string errorPrefix = "Erro (" + std::to_string(startServerResult) + ") iniciando server";
        perror(errorPrefix.c_str());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
