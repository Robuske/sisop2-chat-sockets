#include "ClientUI.h"
#include <iostream>
#include <ctime>

void ClientUI::displayMessages(std::list<Message> messages, UserInfo userInfo) {

    clearScreen();
    std::cout << "Grupo: " << userInfo.groupName << std::endl;
    for (const Message& message:messages) {
        std::cout << this->buildTextMessage(message, userInfo.username) << std::endl;
    }
}

void ClientUI::displayMessageSizeError(int messageSize) {
    std::cout << "[SYSTEM] Mensagem muito longa. Sua mensagem deve ter no máximo " << std::to_string(MESSAGE_SIZE) << " caracteres, essa mensagem tinha " << std::to_string(messageSize) << "." << std::endl;
}

void ClientUI::displayTextInputIndicator(string userName) {
    std::cout << std::endl;
    std::cout << userName << ": _";
    std::cout << std::endl;
}

string ClientUI::buildTextMessage(Message message, string currentUserName) {
    string dateString = dateStringFromTimestamp(message.timestamp);
    string senderUsername = this->senderUsernameForMessageAndCurrentUsername(message, currentUserName);
    string msgPrefix = dateString + " [" + senderUsername + "] ";
    string finalMessage = msgPrefix + message.text;

    return finalMessage;
}

string ClientUI::senderUsernameForMessageAndCurrentUsername(Message message, string currentUserName) {

    switch (message.packetType) {
        case TypeMessage:
        case TypeConnection:
        case TypeDisconnection:
            return (message.username == currentUserName) ? "Você" : message.username;
        default:
            return "SYSTEM";
    }
}
