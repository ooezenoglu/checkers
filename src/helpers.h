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
#include <sys/epoll.h>
#include "exitHandlers.h"

#define BUFFER_SIZE 255
#define MAXEVENTS 2
#define NACK "-"
#define BOARDROWS 8
#define BOARDCOLS 8

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
    char clientHasWon[10];
    int nPlayers;
    int shmidOpponents;
    int shmidGameState;
};

struct player {
    int playerNumber;
    char playerName[20];
    int isReady;
    char hasWon[10];
};

struct gameState {
    int timeout;
    int rows;
    int cols;
    bool think;
    char move[20];
    char board[BOARDROWS][BOARDCOLS];
};

struct SHMInfo {
    bool thinkerAttachedGameInfo;
    bool thinkerAttachedOppInfo;
    bool thinkerAttachedGameState;
    bool connectorAttachedGameInfo;
    bool connectorAttachedOppInfo;
    bool connectorAttachedGameState;
};

/* global variables */
extern int shmidGameInfo;
extern int sockfd;
extern int epfd;
extern int pipefd[2];
extern struct gameInfo *gameInfo;
extern struct player *oppInfo;
extern struct gameState *gameState;
extern struct SHMInfo SHMInfo;
extern struct epoll_event sockEV;
extern struct epoll_event pipeEV;

#endif