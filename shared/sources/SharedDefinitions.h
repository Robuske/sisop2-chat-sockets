#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <string>

using std::string;

// Codes

#define CODE_SUCCESS 0

typedef int SocketFD;

struct tUserInfo {
    string name;
    string group;
} typedef UserInfo;

struct Message {
    uint16_t timestamp;
    string group;
    string username;
    string text;
};

struct PacketHeader {
    uint16_t type;
    uint16_t length;
};

struct Packet {
    struct PacketHeader header;
    const char* _payload;
};


#endif // SISOP2_T1_SHAREDDEFINITIONS_H
