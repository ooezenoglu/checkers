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
    char randomMove[3+2+3] = { 0 };
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
            // printf("Candidate at: %s; Candidate is king: %i\n", from, isKingPiece);

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

    getValidRandomMove(randomMove);
    memcpy(move, randomMove, strlen(randomMove));
    gameState -> moveLength += strlen(move) + 1;

    return move;
}

bool isClientPiece(char piece) {

    if(gameInfo -> thisPlayerNumber == 0) {
        return (piece == 'w') || (piece == 'W');
        
    } else if(gameInfo -> thisPlayerNumber == 1){
        return (piece == 'b') || (piece == 'B');

    } else {
        errNdie("piece could not be found");
        return false;
    }
}

bool isOpponentPiece(char piece) {
    return !isClientPiece(piece) && piece != '*';
}

bool isPlayerWhite() {
    return gameInfo -> thisPlayerNumber == 0;
}

bool isPlayerBlack() {
    return gameInfo -> thisPlayerNumber == 1;
}

bool isKing(char piece) {
    return (piece == 'B') || (piece == 'W');
}

bool becomesKing(char to[3]) {
    
    if(gameInfo -> thisPlayerNumber == 0) {
        return (to[1] == '8');
        
    } else if (gameInfo -> thisPlayerNumber == 1) {
        return (to[1] == '1');

    } else {
        errNdie("Unknown player number.");
        return false;
    }
}

void getIndexAt(int row, int col, char idx[3]) {
    idx[0] = 'A' + col;
    idx[1] = gameState -> rows - row + '0';
    idx[2] = '\0'; 
}

void getBoardIJ(int* row, int* col, char src[3]) {
    *col = src[0] - 'A';
    *row = gameState -> rows - (src[1] - '0');
}

bool canJumpLeftForward(char from[3]) {

    switch (gameInfo -> thisPlayerNumber) {
        case 0:
            return !(from[0] == 'A' || from[0] == 'B'|| from[1] == '7' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'G' || from[0] == 'H' || from[1] == '1' || from[1] == '2');
            break;
        default:
            errNdie("Wrong Player Number!");
    }

    return true;
}

bool canMoveLeftForward(char from[3]) {

    switch (gameInfo -> thisPlayerNumber) {
        case 0:
            return !(from[0] == 'A' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'H' || from[1] == '1');
            break;
        default:
            errNdie("Wrong Player Number!");
    }
    
    return true;
}

bool canJumpRightForward(char from[3]) {

    switch (gameInfo -> thisPlayerNumber) {
        case 0:
            return !(from[0] == 'G' || from[0] == 'H' || from[1] == '7' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'A' || from[0] == 'B' || from[1] == '1' || from[1] == '2');
            break;
        default:
            errNdie("Wrong Player Number!");
    }
        
    return true;
}

bool canMoveRightForward(char from[3]) {

    switch (gameInfo -> thisPlayerNumber) {
        case 0:
            return !(from[0] == 'H' || from[1] == 8);
            break;
        case 1:
            return !(from[0] == 'A' || from[1] == '1');
            break;
        default:
            errNdie("Wrong Player Number!");
    }
    
    return true;
}

void getLeftDiagonalCell(char from[3], char dest[3]) {

   /* TODO switch playerNumber */

    if(!canMoveLeftForward(from)) {
        errNdie("Cannot get left diagonal cell."); 
    }

    dest[0] = from[0] - 1;
    dest[1] = from[1] + 1;
    dest[2] = '\0';

    /* debugging */
    // printf("Left diagonal cell of %s is %s.\n", from, dest);
}

void getRightDiagonalCell(char from[3], char dest[3]){

    /* TODO switch playerNumber */

    if(!canMoveRightForward(from)) {
        errNdie("Cannot get right diagonal cell."); 
    }

    dest[0] = from[0] + 1;
    dest[1] = from[1] + 1;
    dest[2] = '\0';

    /* debugging */
    // printf("Right diagonal cell of %s is %s.\n", from, dest);
}

bool isCellFree(char source[3]) {
    
    int i, j;

    getBoardIJ(&i, &j, source);
    
    return gameState -> board[i][j] == '*';
}

bool canBeat(char from[3], bool isKing, char to[3]) {

    char dest[3] = {0};
    int i, j;

    if(!isKing) {
        if(canJumpLeftForward(from)) {
            getLeftDiagonalCell(from, dest);
            getBoardIJ(&i, &j, dest);
            // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
            if(isOpponentPiece(gameState -> board[i][j])) {
                // printf("Opponent piece at %s\n", dest);
                getLeftDiagonalCell(dest, to);
                if(isCellFree(to)){
                    return true;
                }
                return false; // just for now, if done with other stuff change this to true;
            }
        } 

        if(canJumpRightForward(from)) {
            getRightDiagonalCell(from, dest);
            getBoardIJ(&i, &j, dest);
            // printf("Right diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
            if(isOpponentPiece(gameState -> board[i][j])) {
                // printf("Opponent piece at %s\n", dest);
                getRightDiagonalCell(dest, to);
                if(isCellFree(to)){
                    return true;
                }
                return false; // just for now, if done with other stuff change this to true;
            }
        } 
    }

    /* TODO code for isKing == true */
    return false;
}

bool getValidRandomMove(char move[8]) {

    /* TODO randomly go to left or rght */

    char candidate;
    char from[3] = { 0 }; /* 2 chars + Null Byte */
    char to[3] = { 0 }; /* 2 chars + Null Byte */
    bool isKingPiece;

        for(int i = 0; i < gameState -> rows; i++) {
            for(int j = 0; j < gameState -> cols; j++) {

                if(!isClientPiece(gameState -> board[i][j])) { continue; }

                candidate = gameState -> board[i][j];
                getIndexAt(i, j, from);
                isKingPiece = isKing(candidate);

                if(!isKingPiece) {

                    if(canMoveLeftForward(from)){
                        getLeftDiagonalCell(from, to);
                        isCellFree(to);
                        /* store the move */
                        stringConcat(from, to, ":", move);
                        return true;
                    }

                    if(canMoveRightForward(from)){
                        getRightDiagonalCell(from, to);
                        isCellFree(to);
                        /* store the move */
                        stringConcat(from, to, ":", move);
                        return true;
                    }
                    return false;
                }
            
                //TODO noch für isKing prüfen
            }
        }
    
    return false;
}