#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <ctime>
#include <string>
#include <vector>
#include <map>

using std::string;

// TODO:
//  The PORT is here so we can share with client and don't to change the PORT every time we run the server.
//  Sometimes the port hangs and the server won't start until the OS close the release the resource
#define PORT 2000

#define TIMEOUT 60

// Codes

#define ERROR_SOCKET_CREATION -2
#define ERROR_SOCKET_WRITE -18
#define ERROR_SOCKET_READ -20
#define CODE_SUCCESS 0

#define NAME_SIZE 32
#define MESSAGE_SIZE 512

///MARK: GAMBI Mutex declaration problem - Implicit copy
#define ALL_GROUPS "ALL_GROUPS"

typedef int SocketFD;
typedef std::vector<char> ContinuousBuffer;

enum PacketType { TypeConnection, TypeDesconnection, TypeMessage, TypeKeepAlive, TypeMaxConnectionsReached };

struct tUserInfo {
    string username;
    string groupName;
} typedef UserInfo;

struct Packet {
    PacketType type;
    std::time_t timestamp;
    char groupName[NAME_SIZE];
    char username[NAME_SIZE];
    char text[MESSAGE_SIZE];
};

struct SocketConnectionInfo {
    string ipAddress;
    unsigned short port;
};

// Only forward definition to avoid duplicate symbols
std::time_t now();
Packet continuousBufferRead(SocketFD communicationSocket, ContinuousBuffer continuousBuffer);

// Definitions for the front end and server connections configuration files

/**
 * Map: AvailableConnections
 * Responsible to store all the connection information of the front-end and server
 * to make it possible the creation of coordinator elections and to manage
 * Client -> Front End | Front End -> Server | Server -> Server connections
 * @attributes [server/frontEnd id, ConnectionInfo]
 */

//typedef std::map<int , SocketConnectionInfo> AvailableConnections;

typedef struct AvailableConnections {
    int id;
    SocketConnectionInfo connectionInfo;
} AvailableConnections;

#endif // SISOP2_T1_SHAREDDEFINITIONS_H
