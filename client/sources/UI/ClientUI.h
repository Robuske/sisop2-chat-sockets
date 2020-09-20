#ifndef SISOP2_T1_CLIENTUI_H
#define SISOP2_T1_CLIENTUI_H

#include <string>

using std::string;

class ClientUI {
public:
    int startClient(string userName, string groupName, string serverIPAddress, string port);
};


#endif //SISOP2_T1_CLIENTUI_H
