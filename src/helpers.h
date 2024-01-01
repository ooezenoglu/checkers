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
#include <signal.h>
#include "exitHandlers.h"

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

/* global variables */
extern int shmidGameInfo;
extern int sockfd;
extern struct gameInfo *gameInfo;
extern struct player *oppInfo;
extern bool thinkerAttachedGameInfo;
extern bool thinkerAttachedOppInfo;
extern bool connectorAttachedGameInfo;
extern bool connectorAttachedOppInfo;

#endif