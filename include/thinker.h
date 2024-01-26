#ifndef thinker
#define thinker

#include "helpers.h"
#include "gameplay.h"

void think();
char *computeMove();
bool isClientPiece(char piece);
bool isOpponentPiece(char piece);
bool isKing(char piece);
bool becomesKing(char to[3], char candidate);
void getIndexAt(int row, int col, char idx[3]);
void getBoardIJ(int* row, int* col, char src[3]);
bool canJumpLeftUp(char from[3]);
bool canJumpLeftDown(char from[3]);
bool canMoveLeftUp(char from[3]);
bool canMoveLeftDown(char from[3]);
bool canJumpRightUp(char from[3]);
bool canJumpRightDown(char from[3]);
bool canMoveRightUp(char from[3]);
bool canMoveRightDown(char from[3]);
bool getLeftCellUp(char from[3], char dest[3]);
bool getLeftCellDown(char from[3], char dest[3]);
bool getRightCellUp(char from[3], char dest[3]);
bool getRightCellDown(char from[3], char dest[3]);
bool canBeat(char from[3], bool isKing, char to[3]);
bool pursueLeftUp(char from[3], char to[3]);
bool pursueLeftDown(char from[3], char to[3]);
bool pursueRightUp(char from[3], char to[3]);
bool pursueRightDown(char from[3], char to[3]);
bool isCellFree(char source[3]);
bool getValidRandomMove(char move[8]);

#endif