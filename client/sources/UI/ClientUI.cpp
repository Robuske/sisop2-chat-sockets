#include "ClientUI.h"
#include <iostream>
#include <ctime>

void clearScreen() {
    system("clear");
}

void ClientUI::displayMessages(std::list<Message> messages, UserInfo userInfo) {

    clearScreen();
    std::cout << "Grupo: " << userInfo.groupName << std::endl;
    for (const Message& message:messages) {
        std::cout << this->buildTextMessage(message, userInfo.username) << std::endl;
    }
}

void ClientUI::displayMessageSizeError() {
    std::cout << "[SYSTEM] Mensagem muito longa. Sua mensagem deve ter no máximo " << std::to_string(MESSAGE_SIZE) << " caracteres."<< std::endl;
}

void ClientUI::displayTextInputIndicator(string userName) {
    std::cout << std::endl;
    std::cout << userName << ": _";
    std::cout << std::endl;
}

string ClientUI::buildTextMessage(Message message, string currentUserName) {
    string strTimeStamp = this->fromTimeStampToDateString(message.timestamp);
    string msgPrefix = this->solveMessagePrefix(message, currentUserName);
    string finalMessage = strTimeStamp + " [" + msgPrefix + "] " + message.text;

    return finalMessage;
}

string ClientUI::fromTimeStampToDateString(std::time_t timestamp) {
    struct tm *timeInfo = localtime (&timestamp);
    char strBuffer[20];
    strftime (strBuffer, 20,"%H:%M:%S",timeInfo);
    return strBuffer;
}

string ClientUI::solveMessagePrefix(Message message, string currentUserName) {

    switch (message.packetType) {
        case TypeMessage:
        case TypeConnection:
        case TypeDesconnection:
            return (message.username == currentUserName) ? "Você" : message.username;
        default:
            return "SYSTEM";
    }
}
