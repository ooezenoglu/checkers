#ifndef thinker
#define thinker

#include "helpers.h"
#include "gameplay.h"

void think();
char *computeMove();
bool isClientPiece(char piece);
bool isOpponentPiece(char piece);
bool isPlayerWhite();
bool isPlayerBlack();
bool isKing(char piece);
bool becomesKing(char to[3]);
void getIndexAt(int row, int col, char idx[3]);
void getBoardIJ(int* row, int* col, char src[3]);
bool canJumpLeftForward(char from[3]);
bool canMoveLeftForward(char from[3]);
bool canJumpRightForward(char from[3]);
bool canMoveRightForward(char from[3]);
void getLeftDiagonalCell(char from[3], char dest[3]);
void getRightDiagonalCell(char from[3], char dest[3]);
bool canBeat(char from[3], bool isKing, char to[3]);
bool isCellFree(char source[3]);
bool getValidRandomMove(char move[8]);

#endif