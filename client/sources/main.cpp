#include <iostream>
#include "UI/ClientUI.h"
#include <string>

int main() {
    std::cout << " :::: Zap ::::" << std::endl;
    // Just to do something for now

    string username;
    std::cout << "Como quer ser chamado?" << std::endl;
    getline(std::cin, username);

    string groupName;
    std::cout << "Qual grupo deseja entrar?" << std::endl;
    getline(std::cin, groupName);

    string serverIPAddress;
    std::cout << "Qual o IP do servidor?" << std::endl;
    getline(std::cin, serverIPAddress);

    string port;
    std::cout << "Qual a porta do servidor?" << std::endl;
    getline(std::cin, port);

    return ClientUI().startClient(username, groupName, serverIPAddress, std::stoul(port));
}
