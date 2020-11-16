#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <ctime>
#include <string>
#include <vector>

using std::string;

// TODO:
//  The PORTS are here so we can share with client and don't to change the PORT every time we run the server.
//  Sometimes the port hangs and the server won't start until the OS close the release the resource
#define PORT_FRONT_CLIENT 30000
#define PORT_FRONT_SERVER 30050

#define TIMEOUT 60

// Codes

#define ERROR_INVALID_HOST -40
#define ERROR_SOCKET_CREATION -60
#define ERROR_SOCKET_CONNECTION -65
#define ERROR_SOCKET_WRITE -18
#define ERROR_SOCKET_READ -20
#define CODE_SUCCESS 0

#define NAME_SIZE 32
#define MESSAGE_SIZE 512

///MARK: GAMBI Mutex declaration problem - Implicit copy
#define ALL_GROUPS "ALL_GROUPS"

typedef int SocketFD;
typedef std::vector<char> ContinuousBuffer;

enum PacketType { TypeConnection, TypeDisconnection, TypeMessage, TypeKeepAlive, TypeMaxConnectionsReached };

struct tUserInfo {
    string username;
    string groupName;
} typedef UserInfo;

struct tClient {
    int frontID;
    SocketFD clientSocket;

    bool operator ==(const struct tClient &rhs) const {
        return (this->frontID == rhs.frontID) && (this->clientSocket && rhs.clientSocket);
    }

    bool operator <(const struct tClient &rhs) const {
        return (this->frontID < rhs.frontID) || ((this->frontID == rhs.frontID) && (this->clientSocket < rhs.clientSocket));
    }

} typedef Client;

const auto clientNotSet = Client {-1, -1};

struct Packet {
    PacketType type;
    std::time_t timestamp;
    Client sender;
    Client recipient;
    char groupName[NAME_SIZE];
    char username[NAME_SIZE];
    char text[MESSAGE_SIZE];
};

// Only forward definition to avoid duplicate symbols
std::time_t now();
Packet continuousBufferRead(SocketFD communicationSocket, ContinuousBuffer continuousBuffer);

struct SocketConnectionInfo {
    string ipAddress;
    unsigned short port;
};

void clearScreen();

#endif // SISOP2_T1_SHAREDDEFINITIONS_H
