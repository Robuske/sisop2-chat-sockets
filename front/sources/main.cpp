#include <iostream>
#include <stdlib.h>
#include "FrontCommunicationManager.h"
#include "FrontDefinitions.h"
#include <string>

int main(int argc, char *argv[]) {
    std::cout << "Iniciando FRONT Zap" << std::endl;
    std::cout << "Porta: " << PORT_FRONT << std::endl;

    int startServerResult = FrontCommunicationManager().startFront();
    if (startServerResult < 0) {
        string errorPrefix = "Erro (" + std::to_string(startServerResult) + ") iniciando server";
        perror(errorPrefix.c_str());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
