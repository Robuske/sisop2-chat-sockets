#include "CommunicationManager/ServerCommunicationManager.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

//    if (argc != 2) {
//      //  std::cout << "Usage: server <number of messages to load>" << std::endl;
//      std::cout << "Invalid arguments number" << std::endl;
//      exit(EXIT_FAILURE);
//    }
//
//    int loadMessageCount = atoi(argv[1]);
//    if (loadMessageCount < 0) {
//        loadMessageCount = 0;
//    }
//
//    int myID = atoi(argv[2]);
//    if(myID < 0) {
//        std::cout << "Invalid ID number. Server IDs must be a positive number" << std::endl;
//        exit(EXIT_FAILURE);
//    }
//
//    int coordinatorID = atoi(argv[3]);
//    if(coordinatorID < 0) {
//        std::cout << "Invalid coordinator ID number. Server IDs must be a positive number" << std::endl;
//        exit(EXIT_FAILURE);
//    }
//

    if (argc != 2) {
        std::cout << "Usage: server <number of messages to load>" << std::endl;
        exit(EXIT_FAILURE);
    }

    int loadMessageCount = atoi(argv[1]);
    if (loadMessageCount < 0) {
        loadMessageCount = 0;
    }


    string port = "2004";
    int myID = 100;
    int coordID = 219;
    clearScreen();

    std::cout << "Iniciando servidor Zap" << std::endl;
    std::cout << "Porta: " << port << std::endl;
    std::cout << "My ID: " << myID << std::endl;
    std::cout << "Coord ID: " << coordID << std::endl;
    std::cout << "Iniciando servidor Zap" << std::endl;
    std::cout << "Vai conectar no front hardcoded: " << PORT_FRONT_SERVER << std::endl;
    std::cout << "Enviando as # últimas mensagens enviadas no grupo: " << std::to_string(loadMessageCount) << std::endl;


    int startServerResult = ServerCommunicationManager().startServer(loadMessageCount, myID, coordID,   std::stoul(port));
    if (startServerResult < 0) {
        string errorPrefix = "Erro (" + std::to_string(startServerResult) + ") iniciando server";
        perror(errorPrefix.c_str());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
