#include <iostream>
#include "UI/ClientUI.h"
#include "MessagesManager/ClientMessagesManager.h"
#include <string>

int main() {
    std::cout << " ::::::: Zap ::::::: " << std::endl;

    SocketConnectionInfo connectionInfo;
    UserInfo userInfo;

    string username;
    std::cout << "Como quer ser chamado?" << std::endl;
    getline(std::cin, userInfo.username);

    string groupName;
    std::cout << "Qual grupo deseja entrar?" << std::endl;
    getline(std::cin, userInfo.groupName);

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
