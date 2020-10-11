#include "ClientUI.h"
#include <iostream>

void ClientUI::displayMessage(Message message, string currentUserName) {
    std::cout << this->buildTextMessage(message, currentUserName) << std::endl;
}

string ClientUI::buildTextMessage(Message message, string currentUserName) {
    string strTimeStamp = this->fromTimeStampToDateString(message.timestamp);
    string msgPrefix = this->solveMessagePrefix(message, currentUserName);
    string finalMessage = strTimeStamp + " [" + msgPrefix + "] " + message.text;

    return finalMessage;
}

string ClientUI::fromTimeStampToDateString(long timestamp) {
    return "dd.mm.yyyy";
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
