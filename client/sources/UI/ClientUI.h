#ifndef SISOP2_T1_CLIENTUI_H
#define SISOP2_T1_CLIENTUI_H

#include <string>
#include "Message/Message.h"
#include "SharedDefinitions.h"

using std::string;

class ClientUI {

private:
    string buildTextMessage(Message message, string currentUserName);
    string fromTimeStampToDateString(long timestamp);
    string solveMessagePrefix(Message message, string currentUserName);
public:
    void displayMessage(Message message, string currentUserName);
};


#endif //SISOP2_T1_CLIENTUI_H
