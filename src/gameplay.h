#ifndef gameplay
#define gameplay

#include "helpers.h"
#include "stringUtils.h"
#include "connection.h"
#include "sharedMemory.h"

#define MOVESIZE 5
#define MOVE "+ MOVE"
#define BOARD "+ BOARD"
#define ENDBOARD "+ ENDBOARD"
#define THINKING "THINKING"
#define OKTHINK "+ OKTHINK"
#define PLAY "PLAY "
#define MOVEOK "+ MOVEOK"
#define WAIT "+ WAIT"
#define OKWAIT "OKWAIT"
#define GAMEOVER "+ GAMEOVER"
#define PLAYER0WON "+ PLAYER0WON"
#define PLAYER1WON "+ PLAYER1WON"
#define QUIT "+ QUIT"

void receiveBoard();
void printBoard();
void moveStatement();
void think();
void waitMoveOK();
void waitStatement();
void gameOverStatement();
void performGameplay();

#endif