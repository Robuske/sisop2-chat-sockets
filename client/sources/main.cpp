#include <iostream>
#include "UI/ClientUI.h"
#include "MessagesManager/ClientMessagesManager.h"
#include <string>

int main() {
    std::cout << " :::: Zap ::::" << std::endl;
    // Just to do something for now

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
        printf("Success connecting!");
    } else {
        printf("Error connecting!");
    }

    //return ClientUI().startClient(username, groupName, serverIPAddress, std::stoul(port));

    return 0;
}
