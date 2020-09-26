#include <iostream>
#include "UI/ClientUI.h"
#include "MessagesManager/ClientMessagesManager.h"
#include <string>

void handleControlC(int signal) {
    std::cout << std::endl << "ctrl+C signal(" + std::to_string(signal) + "), disconnecting client..." << std::endl;
    exit(signal);
}

int main() {
    // FIXME: Handle ctrl + C
    // Q: Do we need a thread to handle this?
    // I suspect we'll need a thread that does something like:
    // int keepConnected = 1; // set to 1 when ctrl c
    // while (keepConnected) {
        // yield() to leave CPU so we can continue executing whatever
    // }
    // disconnect();

    signal(SIGINT, handleControlC);

    std::cout << " ::::::: Zap ::::::: " << std::endl;

    SocketConnectionInfo connectionInfo;
    UserInfo userInfo;

    string username;
    std::cout << "Como quer ser chamado?" << std::endl;
    getline(std::cin, userInfo.name);

    string groupName;
    std::cout << "Qual grupo deseja entrar?" << std::endl;
    getline(std::cin, userInfo.group);

    string serverIPAddress;
    std::cout << "Qual o IP do servidor?" << std::endl;
    getline(std::cin, connectionInfo.ipAddress);

    string port;
    std::cout << "Qual a porta do servidor?" << std::endl;
    getline(std::cin, port);

    connectionInfo.port = std::stoul(port);

    int sessionResult = ClientMessagesManager().startClient(connectionInfo, userInfo);
    if(sessionResult > 0) {
        std::cout << "Success connecting!" << std::endl;
    } else {
        string errorPrefix = "Error(" + std::to_string(sessionResult) + ") connecting: ";
        perror(errorPrefix.c_str());
    }

    return 0;
}
