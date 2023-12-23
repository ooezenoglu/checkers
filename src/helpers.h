#ifndef helpers
#define helpers

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct gameInfo {
    char gameID[15];
    int requestedPlayerNumber;
    char configFile[40];
    char hostName[40];
    char gameKindName[20];
    uint16_t port;
    char serverVersion[10];
    char clientVersion[10];
    pid_t connectorPID;
    pid_t thinkerPID;
    char gameName[40];
    int thisPlayerNumber;
    char thisPlayerName[20];
    int nPlayers;
    int shmidOpponents;
};

struct player {
    int playerNumber;
    char playerName[20];
    int isReady;
};

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameInfoPtr);
int readConfigFile(struct gameInfo *gameInfoPtr);
int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, struct gameInfo *gameInfoPtr);
int receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
int sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
int stringConcat(const char *leftString, const char *rightString, char *dest);
int stringTokenizer(char *src, char *delim, char **res, int *len);
void printWaitDetails(int wstatus);
int SHMAlloc(size_t size);

#endif