#include "ClientUI.h"
#include "SharedDefinitions.h"
#include <iostream>

// TODO: Se for o mesmo usuário a mensagem deve ser "Você: msg msg msg"
void ClientUI::displayMessage(string message) {
    std::cout << message;
}
