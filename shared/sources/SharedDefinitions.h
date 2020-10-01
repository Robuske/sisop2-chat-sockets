#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <string>
#include <iostream>

using std::string;

// Codes

#define CODE_SUCCESS 0

typedef int SocketFD;

struct tUserInfo {
    string name;
    string group;
} typedef UserInfo;

struct PacketHeader {
    uint16_t type;
    uint16_t length;
};

struct Message {
    uint16_t timestamp;
    string group;
    string username;
    string text;
};

struct Packet {
    Message payload;
};


#endif // SISOP2_T1_SHAREDDEFINITIONS_H
