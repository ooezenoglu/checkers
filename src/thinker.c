#include "thinker.h"

void think() {

    if(!SHMInfo.thinkerAttachedGameState) { attachGameState(); }

    if(gameState -> think) {
        char *move;

        printBoard();
        move = computeMove();
    
        if(write(pipefd[1], move, gameState -> moveLength) != gameState -> moveLength) { 
            errNdie("Failed to write move to pipe."); 
        }

        gameState -> think = false;

        free(move);
    }
}

char *computeMove() {

    char candidate;
    char from[3] = { 0 }; /* 2 chars + Null Byte */
    char to[3] = { 0 }; /* 2 chars + Null Byte */
    /* initially allocate memory for a string of form "<from>:<to>" + Null Byte */
    char *move = (char *) calloc(2+1+2+1, sizeof(char));
    bool isKingPiece;
    int charsNeeded = 0;
    gameState -> moveLength = 0;

    if(move == NULL) { errNdie("Failed to allocate heap memory for the move."); }

    for(int i = 0; i < gameState -> rows; i++) {
        for(int j = 0; j < gameState -> cols; j++) {

            if(!isClientPiece(gameState -> board[i][j])) { continue; }

            candidate = gameState -> board[i][j];
            getIndexAt(i, j, from); // for now, "from" is "A3"
            isKingPiece = isKing(candidate);

            /* debugging */
            printf("Candidate at: %s; Candidate is king: %i\n", from, isKingPiece);

            if(canBeat(from, isKingPiece, to)) {

                /* store first partial, winning move (e.g., "F4:D6") */
                stringConcat(from, to, ":", move);

                gameState -> moveLength += strlen(move) + 1;

                while(1) { /* check whether more partial moves are possible */

                    /* override the starting point 
                    (that is, if move is "F4:D6", "from" will now be "D6") */
                    memcpy(from, to, strlen(to));

                    /* reset "to" */
                    memset(to, 0, strlen(to));
                    
                    if(becomesKing(to)) { return move; }
                    if(!canBeat(from, isKingPiece, to)) { return move; }

                    /* determine the new length of the new move */
                    if((charsNeeded = snprintf(NULL, 0, "%s:%s", move, to)) < 0) {
                        errNdie("Failed to calculate string size.");
                    }

                    /* increase the size of the move */
                    if((move = realloc(move, charsNeeded + 1)) == NULL) {
                        errNdie("Failed to increase heap memory for the move.");
                    }

                    /* append the new "to" value to the existing move */
                    stringConcat(move, to, ":", move);

                    gameState -> moveLength = charsNeeded + 1;
                    return move;
                }
            }
            
            continue;
        }
    }

    memcpy(move, "E3:F4", strlen("E3:F4"));
    gameState -> moveLength += strlen(move) + 1;

    return move;
}

bool isClientPiece(char piece) {

    if(gameInfo -> thisPlayerNumber == WHITE) {
        return (piece == 'w' || piece == 'W');

    } else if (gameInfo -> thisPlayerNumber == BLACK) {
        return (piece == 'b' || piece == 'B');

    } else {
        errNdie("Unknown player number of client.");
    }
}

void getIndexAt(int row, int col, char idx[2]) {
    idx[0] = 65 + col;
    idx[1] = gameState -> rows - row + '0';
}

bool isKing(char piece) {
    return false;
}

bool canBeat(char from[2], bool isKing, char to[2]) {
    to[0] = 'B',
    to[1] = '4';
    return true;
}

bool becomesKing(char to[2]) {
    return false;
}

char *getValidRandomMove() {

    char *move = (char *) calloc(2+1+2+1, sizeof(char));

    stringConcat("E3", "F4", ":", move);

    return move;
}
