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
                    
                    if(becomesKing(to, candidate)) { return move; }
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
                }
                return move;
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

bool isKing(char piece) {
    return (piece == 'B') || (piece == 'W');
}

bool becomesKing(char to[3], char candidate) {
    
    if(gameInfo -> thisPlayerNumber == 0) {
        return (to[1] == '8') && !isKing(candidate);
        
    } else if (gameInfo -> thisPlayerNumber == 1) {
        return (to[1] == '1') && !isKing(candidate);

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

bool canJumpLeftUp(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'A' || from[0] == 'B'|| from[1] == '7' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'H' || from[0] == 'G'|| from[1] == '1' || from[1] == '2');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canJumpLeftDown(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'A' || from[0] == 'B'|| from[1] == '1' || from[1] == '2');
            break;
        case 1:
            return !(from[0] == 'H' || from[0] == 'G'|| from[1] == '8' || from[1] == '7');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canMoveLeftUp(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'A' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'H' || from[1] == '1');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canMoveLeftDown(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'A' || from[1] == '1');
            break;
        case 1:
            return !(from[0] == 'H' || from[1] == '8');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canJumpRightUp(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'G' || from[0] == 'H' || from[1] == '7' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'A' || from[0] == 'B' || from[1] == '1' || from[1] == '2');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canJumpRightDown(char from[3]) {
    
    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'G' || from[0] == 'H' || from[1] == '1' || from[1] == '2');
            break;
        case 1:
            return !(from[0] == 'A' || from[0] == 'B' || from[1] == '7' || from[1] == '8');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canMoveRightUp(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'H' || from[1] == '8');
            break;
        case 1:
            return !(from[0] == 'A' || from[1] == '1');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool canMoveRightDown(char from[3]) {

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            return !(from[0] == 'H' || from[1] == '1');
            break;
        case 1:
            return !(from[0] == 'A' || from[1] == '8');
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    return false;
}

bool getLeftCellUp(char from[3], char dest[3]) {

    if(!canMoveLeftUp(from)) {
        return false;
    }

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            dest[0] = from[0] - 1;
            dest[1] = from[1] + 1;
            dest[2] = '\0';
            break;
        case 1:
            dest[0] = from[0] + 1;
            dest[1] = from[1] - 1;
            dest[2] = '\0';
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    /* debugging */
    // printf("Left diagonal cell of %s is %s.\n", from, dest);

    return true;
}

bool getLeftCellDown(char from[3], char dest[3]) {

    if(!canMoveLeftDown(from)) {
        return false;
    }

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            dest[0] = from[0] - 1;
            dest[1] = from[1] - 1;
            dest[2] = '\0';
            break;
        case 1:
            dest[0] = from[0] + 1;
            dest[1] = from[1] + 1;
            dest[2] = '\0';
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    /* debugging */
    // printf("Left diagonal cell of %s is %s.\n", from, dest);

    return true;
}

bool getRightCellUp(char from[3], char dest[3]){

    if(!canMoveRightUp(from)) {
        return false;
    }

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            dest[0] = from[0] + 1;
            dest[1] = from[1] + 1;
            dest[2] = '\0';
            break;
        case 1:
            dest[0] = from[0] - 1;
            dest[1] = from[1] - 1;
            dest[2] = '\0';
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    /* debugging */
    // printf("Right diagonal cell of %s is %s.\n", from, dest);

    return true;
}

bool getRightCellDown(char from[3], char dest[3]){

    if(!canMoveRightDown(from)) {
       return false;
    }

    switch(gameInfo -> thisPlayerNumber) {
        case 0: 
            dest[0] = from[0] + 1;
            dest[1] = from[1] - 1;
            dest[2] = '\0';
            break;
        case 1:
            dest[0] = from[0] - 1;
            dest[1] = from[1] + 1;
            dest[2] = '\0';
            break;
        default:
            errNdie("Player number unknown.");
            break;
    }

    /* debugging */
    // printf("Right diagonal cell of %s is %s.\n", from, dest);

    return true;
}

bool isCellFree(char source[3]) {
    
    int i, j;

    getBoardIJ(&i, &j, source);
    
    return gameState -> board[i][j] == '*';
}

bool pursueLeftUp(char from[3], char to[3]) {
    
    char dest[3] = { 0 };
    int i, j;

    if(canJumpLeftUp(from)) {
        printf("Pursuing left up...\n");
        getLeftCellUp(from, dest);
        getBoardIJ(&i, &j, dest);
        // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
        if(isOpponentPiece(gameState -> board[i][j])) {
            // printf("Opponent piece at %s\n", dest);
            getLeftCellUp(dest, to);
            if(isCellFree(to)){
                printf("Move found; Go to %s!\n", to);
                gameState -> board[i][j] = '*';
                return true;
            }
        } else if(isCellFree(dest)) {
            return pursueLeftUp(dest, to);
        }
   }

    printf("Can't pursue left up!\n");
    return false;
}

bool pursueRightUp(char from[3], char to[3]) {
    
    char dest[3] = { 0 };
    int i, j;

    if(canJumpRightUp(from)) {
        printf("Pursuing right up...\n");
        getRightCellUp(from, dest);
        getBoardIJ(&i, &j, dest);
        // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
        if(isOpponentPiece(gameState -> board[i][j])) {
            // printf("Opponent piece at %s\n", dest);
            getRightCellUp(dest, to);
            if(isCellFree(to)){
                printf("Move found; Go to %s!\n", to);
                gameState -> board[i][j] = '*';
                return true;
            }
        } else if(isCellFree(dest)) {
            return pursueRightUp(dest, to);
        }
   }

    printf("Can't pursue right up!\n");
    return false;
}

bool pursueLeftDown(char from[3], char to[3]) {
    
    char dest[3] = { 0 };
    int i, j;

    if(canJumpLeftDown(from)) {
        printf("Pursuing left down...\n");
        getLeftCellDown(from, dest);
        getBoardIJ(&i, &j, dest);
        // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
        if(isOpponentPiece(gameState -> board[i][j])) {
            // printf("Opponent piece at %s\n", dest);
            getLeftCellDown(dest, to);
            if(isCellFree(to)){
                printf("Move found; Go to %s!\n", to);
                gameState -> board[i][j] = '*';
                return true;
            }
        } else if(isCellFree(dest)) {
            return pursueLeftDown(dest, to);
        }
   }

    printf("Can't pursue left down!\n");
    return false;
}

bool pursueRightDown(char from[3], char to[3]) {

    char dest[3] = { 0 };
    int i, j;

    if(canJumpRightDown(from)) {
        printf("Pursuing right down...\n");
        getRightCellDown(from, dest);
        getBoardIJ(&i, &j, dest);
        // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
        if(isOpponentPiece(gameState -> board[i][j])) {
            // printf("Opponent piece at %s\n", dest);
            getRightCellDown(dest, to);
            if(isCellFree(to)){
                printf("Move found; Go to %s!\n", to);
                gameState -> board[i][j] = '*';
                return true;
            }
        } else if(isCellFree(dest)) {
            return pursueRightDown(dest, to);
        }
   }

    printf("Can't pursue right down!\n");
    return false;
}

bool canBeat(char from[3], bool isKing, char to[3]) {

    char dest[3] = { 0 };
    int i, j;

    if(!isKing) {
        if(canJumpLeftUp(from)) {
            getLeftCellUp(from, dest);
            getBoardIJ(&i, &j, dest);
            // printf("Left diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
            if(isOpponentPiece(gameState -> board[i][j])) {
                // printf("Opponent piece at %s\n", dest);
                getLeftCellUp(dest, to);
                if(isCellFree(to)) {
                    return true;
                }
            }
        } 

        if(canJumpRightUp(from)) {
            getRightCellUp(from, dest);
            getBoardIJ(&i, &j, dest);
            // printf("Right diagonal cell is %s, with row index %i and col index %i\n", dest, i, j);
            if(isOpponentPiece(gameState -> board[i][j])) {
                // printf("Opponent piece at %s\n", dest);
                getRightCellUp(dest, to);
                if(isCellFree(to)) {
                    return true;
                }
            }
        }

        return false;
    }

    if(isKing) {
        return pursueLeftUp(from, to) || pursueRightUp(from, to) || pursueLeftDown(from, to) || pursueRightDown(from, to);
    }

    return false;
}

bool getValidRandomMove(char move[8]) {

    char candidate;
    char from[3] = { 0 }; /* 2 chars + Null Byte */
    char to[3] = { 0 }; /* 2 chars + Null Byte */

    for(int i = 0; i < gameState -> rows; i++) {
        for(int j = 0; j < gameState -> cols; j++) {

            if(!isClientPiece(gameState -> board[i][j])) { continue; }

            candidate = gameState -> board[i][j];
            getIndexAt(i, j, from);

            if(canMoveLeftUp(from)){
                getLeftCellUp(from, to);
                if(isCellFree(to)) {
                    /* store the move */
                    stringConcat(from, to, ":", move);
                    return true;
                }
            }

            if(canMoveRightUp(from)){
                getRightCellUp(from, to);
                if(isCellFree(to)) {
                    /* store the move */
                    stringConcat(from, to, ":", move);
                    return true;
                }
            }
                
            if(isKing(candidate)) {
                if(canMoveLeftDown(from)){
                    getLeftCellDown(from, to);
                    if(isCellFree(to)) {
                        /* store the move */
                        stringConcat(from, to, ":", move);
                        printf("Sending random move left down\n");
                        return true;
                    }
                }

                if(canMoveRightDown(from)){
                    getRightCellDown(from, to);
                    if(isCellFree(to)) {
                        /* store the move */
                        stringConcat(from, to, ":", move);
                        return true;
                    }
                }
            } 
        }
    }
    
    return false;
}