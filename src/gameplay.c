#include "gameplay.h"

char buffer[BUFFER_SIZE] = { 0 };

void receiveBoard() {

    char *toks[gameState -> cols];
    int i = 0;
    int len;

    while(1) {

        receiveLineFromServer(buffer);

        if(startsWith(buffer, ENDBOARD)) { break; }

        stringTokenizer(buffer + 4, " ", toks, &len, gameState -> cols);

        if(len != gameState -> cols) { errNdie("Error in receiveBoard()."); }

        /* update the board */
        for(int j = 0; j < gameState -> cols; j++) {
            gameState -> board[i][j] = toks[j][0];
        }

        i++;
    }
}

void printBoard() {

    char *w = "\u26C0";
    char *W = "\u26C1";
    char *b = "\u26C2";
    char *B = "\u26C3";

    bool f = true;

    printf("   A B C D E F G H\n");
    printf(" +-----------------+\n");

    for(int i = 0; i < gameState -> rows; i++) {
        printf("%i| ", i + 1);
        for(int j = 0; j < gameState -> cols; j++) {

            switch(gameState -> board[i][j]) {

                case 'w':
                    printf("%s ", w);
                    break;   
                case 'W':
                    printf("%s ", w);
                    break;
                case 'b':
                    printf("%s ", b);
                    break;
                case 'B':
                    printf("%s ", B);
                    break;
                case '*':
                    if(f) {
                        printf(". ");
                    } else {
                        printf("_ ");
                    }
                    break;
                default:
                    errNdie("Unknown symbol.");
            }
            f = !f;
        }
        printf("|%i", i + 1);
        printf("\n");
        f = !f;
    }
    printf(" +-----------------+\n");
    printf("   A B C D E F G H\n");
}

void moveStatement() {

    char concatStr[BUFFER_SIZE] = { 0 };

    while(1) {

        receiveLineFromServer(buffer);

        if(startsWith(buffer, BOARD)){

            receiveBoard();

            /* request time to think */
            sendLineToServer(THINKING);

        } else if(startsWith(buffer, OKTHINK)) {

            printf("Ok, make your move now!\n");

            /* send a signal to the Thinker that it should think */
            gameState -> think = true;
            kill(gameInfo -> thinkerPID, SIGUSR1);

            /* shortly wait for the update to arrive */
            sleep(1);

            /* read move from the pipe */
            if((read(pipefd[0], gameState -> move, MOVESIZE)) != MOVESIZE) {
                errNdie("Failed to read move from pipe.");
            }

            /* send the move to the server */
            printf("Sending move %s...\n", gameState -> move);
            stringConcat(PLAY, gameState -> move, concatStr);
            sendLineToServer(concatStr);

            waitMoveOK();
            break;

        } else if(startsWith(buffer, QUIT)) {

            errNdie("moveStatement(): Protocol error.");

        } else {

            /* for unexpected things */
            printf("Server: %s\n", buffer);
            errNdie("Error in moveStatement().");
        }
    }
}

void think() {

    if(!SHMInfo.thinkerAttachedGameState) { attachGameState(); }

    if(gameState -> think) {

        char *moveWhite = "A3:B4";
        char *moveBlack = "B6:A5";
        int n;

        printBoard();

        /* TODO calculate the next move here (hardcoded for now) */

        /* write (hardcoded) move to the pipe */
        if(gameInfo -> thisPlayerNumber == 0) {

            n = write(pipefd[1], moveWhite, MOVESIZE);

        } else {

            n = write(pipefd[1], moveBlack, MOVESIZE);
        }

        if(n != MOVESIZE) { errNdie("Failed to write move to pipe."); }

        gameState -> think = false;
    }
}

void waitMoveOK() {

    while(1){
        receiveLineFromServer(buffer);

        if(startsWith(buffer, MOVEOK)) {

            printf("The move is valid. Opponents turn.\n");
            break;

        } else {

            /* for unexpected things */
            printf("Server: %s\n", buffer);
            errNdie("Error in waitMOVEOK().");
        }   
    }
}

void waitStatement() {
    sendLineToServer(OKWAIT);
}

void gameOverStatement() {
    
    while(1) {

        receiveLineFromServer(buffer);

        if(startsWith(buffer, BOARD)){

            receiveBoard();

        } else if(startsWith(buffer, PLAYER0WON)) {

            /* store whether client has won */
            if(sscanf(buffer, "%*s %*s %s", gameInfo -> clientHasWon) != 1) {
                errNdie("Could not store game data.");
            }

            printf("Client has won: %s\n.", gameInfo -> clientHasWon);

        } else if(startsWith(buffer, PLAYER1WON)) {

            /* store whether opponent has won */
            if(sscanf(buffer, "%*s %*s %s", oppInfo -> hasWon) != 1) {
                errNdie("Could not store game data.");
            }

            printf("Opponent has won: %s\n.", oppInfo -> hasWon);
        
        } else if(startsWith(buffer, QUIT)) {

            printf("Quitting game...");
            break;

        } else {

            /* for unexpected things */
            printf("Server: %s\n", buffer);
            errNdie("Error in gameOverStatement().");
        }
     }
}

void performGameplay() {

    char concatStr[BUFFER_SIZE] = { 0 };

    printf("Gameplay phase\n");

    /* create a shared memory segment for the game state */
    gameInfo -> shmidGameState = SHMAlloc(sizeof(struct gameState));

    /* attach game state to Connector process */
    gameState = (struct gameState*) SHMAttach(gameInfo -> shmidGameState);
    SHMInfo.connectorAttachedGameState = true;

    gameState -> rows = BOARDROWS;
    gameState -> cols = BOARDCOLS;
    gameState -> think = false;

    while(1) {

        receiveLineFromServer(buffer);

         if (startsWith(buffer, NACK)) {

            stringConcat("NACK received: ", buffer + 2, concatStr); /* + 2 to skip preceding "- " */

            /* remove newline at the end of the string bc perror adds more stuff to it */
            concatStr[strlen(concatStr)-1] = '\0';

            errNdie(concatStr);

        } else if(startsWith(buffer, GAMEOVER)) { /* game over */

            gameOverStatement();
            break;

        } else if(startsWith(buffer, WAIT)) { /* idle */

            waitStatement();

        } else if(startsWith(buffer, MOVE)) { /* move */

            /* store & print timeout */
            if(sscanf(buffer, "%*s %*s %i", &(gameState -> timeout)) != 1) {
                errNdie("Could not store game data.");
            } else {
                printf("Make your move within %i ms.\n", gameState -> timeout);
            }

            moveStatement();
        }
    }
}