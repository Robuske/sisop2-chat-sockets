#ifndef SISOP2_T1_CLIENTUI_H
#define SISOP2_T1_CLIENTUI_H

#include <string>
#include "Message/Message.h"
#include "SharedDefinitions.h"
#include <list>

using std::string;

class ClientUI {

private:
    string buildTextMessage(Message message, string currentUserName);
    string senderUsernameForMessageAndCurrentUsername(Message message, string currentUserName);
public:
    void displayMessages(std::list<Message> messages, UserInfo userInfo);
    void displayTextInputIndicator(string userName);
    void displayMessageSizeError(int messageSize);
};


#endif //SISOP2_T1_CLIENTUI_H
