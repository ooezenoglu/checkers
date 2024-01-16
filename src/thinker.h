#ifndef thinker
#define thinker

#include "helpers.h"
#include "gameplay.h"

void think();
char *computeMove();
bool isClientPiece(char piece);
void getIndexAt(int row, int col, char idx[2]);
bool isKing(char piece);
bool canBeat(char from[2], bool isKing, char to[2]);
bool becomesKing(char to[2]);
char *getValidRandomMove();

#endif
