#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <string>
#include <iostream>

using std::string;

// Codes

#define CODE_SUCCESS 0

typedef int SocketFD;

enum PacketHeaderType { TypeConnection, TypeMessage };

struct tUserInfo {
    string username;
    string groupName;
} typedef UserInfo;

struct PacketHeader {
    PacketHeaderType type;
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
