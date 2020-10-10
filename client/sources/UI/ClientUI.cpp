#include "ClientUI.h"
#include <iostream>

// TODO: Se for o mesmo usuário a mensagem deve ser "Você: msg msg msg"
void ClientUI::displayMessage(Message message, string currentUserName) {
    std::cout << this->buildTextMessage(message, currentUserName) << std::endl;
}

string ClientUI::buildTextMessage(Message message, string currentUserName) {
    string strTimeStamp = this->fromTimeStampToDateString(message.timestamp);
    string msgPrefix = this->solveMessagePrefix(message, currentUserName);
    string finalMessage = strTimeStamp.append(" ")
            .append("[").append(msgPrefix).append("]")
            .append(" ").append(message.text);
    return finalMessage;
}

string ClientUI::fromTimeStampToDateString(long timestamp) {
    return "dd.mm.yyyy";
}

string ClientUI::solveMessagePrefix(Message message, string currentUserName) {

    if(message.packetType == TypeMessage || message.packetType == TypeConnection) {
        return (message.username == currentUserName) ? "you" : message.username;
    } else {
        return "SYSTEM";
    }
}
