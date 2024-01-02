#ifndef gameplay
#define gameplay

#include "helpers.h"
#include "stringUtils.h"
#include "connection.h"
#include "sharedMemory.h"

#define QUIT "+ QUIT"
#define BOARD "+ BOARD"
#define ENDBOARD "+ ENDBOARD"
#define GAMEOVER "+ GAMEOVER"
#define PLAYER0WON "+ PLAYER0WON"
#define PLAYER1WON "+ PLAYER1WON"
#define WAIT "+ WAIT"
#define OKWAIT "OKWAIT"
#define MOVE "+ MOVE"
#define THINKING "THINKING"
#define OKTHINK "+ OKTHINK"
#define PLAY "PLAY"
#define MOVEOK "+ MOVEOK"

void receiveBoard(const int sockfd);
void gameOverStatement(const int sockfd, struct gameInfo *gameDataPointer, char *buffer);
void waitStatement(const int sockfd, char *buffer);
void moveStatement(const int sockfd, struct gameInfo *gameInfoPtr, char *buffer);
void think(const int sockfd, char *buffer);
void performGameplay(const int sockfd, struct gameInfo *gameDataPointer);

#endif