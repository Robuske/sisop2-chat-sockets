#ifndef SISOP2_T1_CLIENTUI_H
#define SISOP2_T1_CLIENTUI_H

#include <string>

using std::string;

class ClientUI {
public:
    int startClient(string userName, string groupName, string serverIPAddress, unsigned short port);

private:
    string userName;

    void *readingMessagesThread(int my_socket);
    static void *staticReadingMessagesThread(void *threadParm);

    void *writingMessagesThread(int my_socket);
    static void *staticWritingMessagesThread(void *threadParm);
};


#endif //SISOP2_T1_CLIENTUI_H
