#include "MessagesManager/ClientMessagesManager.h"
#include "SharedDefinitions.h"
#include <iostream>
#include <signal.h>
#include <string>

void handleControlC(int signal) {
    std::cout << std::endl << "ctrl+C signal(" + std::to_string(signal) + "), disconnecting client..." << std::endl;
    exit(signal);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handleControlC);

    std::cout << " ::::::: Zap ::::::: " << std::endl;

    // Initialize random seed
    srand (time(NULL));

    // Generate random number between 1 and 50
    int random = rand() % 50;
    string username = "Client " + std::to_string(random + 1);
    string groupName = "Group " + std::to_string(random%2 + 1);
    string serverIPAddress = "localhost";
    string port = std::to_string(PORT);
    switch (argc) {
        case 2:
            username = argv[1];
            break;

        case 3:
            username = argv[1];
            groupName = argv[2];
            break;

        case 4:
            username = argv[1];
            groupName = argv[2];
            serverIPAddress = argv[3];
            break;

        case 5:
            username = argv[1];
            groupName = argv[2];
            serverIPAddress = argv[3];
            port = argv[4];
            break;

        default:
            std::cout << "Como quer ser chamado?" << std::endl;
            getline(std::cin, username);

            std::cout << "Qual grupo deseja entrar?" << std::endl;
            getline(std::cin, groupName);

            std::cout << "Qual o IP do servidor?" << std::endl;
            getline(std::cin, serverIPAddress);

            std::cout << "Qual a porta do servidor?" << std::endl;
            getline(std::cin, port);
            break;
    }

    UserInfo userInfo;
    userInfo.username = username;
    userInfo.groupName = groupName;

    SocketConnectionInfo connectionInfo;
    connectionInfo.ipAddress = serverIPAddress;

    // TODO: Use correct port
    connectionInfo.port = std::stoul(port);
    connectionInfo.port = PORT;

    int sessionResult = ClientMessagesManager().startClient(connectionInfo, userInfo);
    if(sessionResult > 0) {
        std::cout << "Success connecting!" << std::endl;
    } else {
        string errorPrefix = "Error(" + std::to_string(sessionResult) + ") connecting: ";
        perror(errorPrefix.c_str());
    }

    return 0;
}
