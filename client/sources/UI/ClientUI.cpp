#include "ClientUI.h"
#include <iostream>
#include <ctime>

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void ClientUI::displayMessages(std::list<Message> messages, UserInfo userInfo) {

    clearScreen();
    std::cout << "Grupo: " << userInfo.groupName << std::endl;
    for (const Message& message:messages) {
        std::cout << this->buildTextMessage(message, userInfo.username) << std::endl;
    }
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

string ClientUI::fromTimeStampToDateString(long timestamp) {
    struct tm *timeInfo = localtime (&timestamp);
    char strBuffer[15];
    strftime (strBuffer, 15,"%H:%M:%S",timeInfo);
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
