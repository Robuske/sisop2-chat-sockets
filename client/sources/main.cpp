#include <iostream>
#include "UI/ClientUI.h"
#include "MessagesManager/ClientMessagesManager.h"
#include <string>
#include "SharedDefinitions.h"
int main(int argc, char *argv[]) {

    std::cout << " ::::::: Zap ::::::: " << std::endl;

    string username, groupName, serverIPAddress, port;
    if (argc == 5) {
        username = argv[1];
        groupName = argv[2];
        serverIPAddress = argv[3];
        port = argv[4];
    } else {
        std::cout << "Como quer ser chamado?" << std::endl;
        getline(std::cin, username);

        std::cout << "Qual grupo deseja entrar?" << std::endl;
        getline(std::cin, groupName);

        std::cout << "Qual o IP do servidor?" << std::endl;
        getline(std::cin, serverIPAddress);

        std::cout << "Qual a porta do servidor?" << std::endl;
        getline(std::cin, port);
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
