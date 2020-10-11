#ifndef SISOP2_T1_CLIENTUI_H
#define SISOP2_T1_CLIENTUI_H

#include <string>
#include "Message/Message.h"
#include "SharedDefinitions.h"
#include <list>

using std::string;

class ClientUI {

private:
    string buildTextMessage(const Message& message, const string& currentUserName);
    string fromTimeStampToDateString(long timestamp);
    string solveMessagePrefix(const Message& message, const string& currentUserName);
public:
    void displayMessages(std::list<Message>& messages, UserInfo userInfo);
    void displayTextInputIndicator(const string& userName);
};


#endif //SISOP2_T1_CLIENTUI_H
