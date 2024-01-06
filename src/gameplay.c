#include "gameplay.h"

#define BUFFER_SIZE 255

void receiveBoard(const int sockfd) {

    char buffer[BUFFER_SIZE] = { 0 };
    char *toks[gameState -> cols];
    int i = 0;
    int len;

    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, ENDBOARD)){ break; }

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

    for (int i = 0; i < gameState -> rows; i++) {
        printf("%i| ", i + 1);
        for (int j = 0; j < gameState -> cols; j++) {

            switch (gameState -> board[i][j]) {

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

void gameOverStatement(const int sockfd, struct gameInfo *gameDataPointer, char *buffer) {
    
    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, BOARD)){

            receiveBoard(sockfd);

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

int moveStatement(const int sockfd, struct gameInfo *gameDataPointer, char *buffer) {

    int n = 5;
    char concatStr[BUFFER_SIZE] = { 0 };

    /* store & print timeout */
    if(sscanf(buffer, "%*s %*s %i", &(gameState -> timeout)) != 1) {
        errNdie("Could not store game data.");
    } else {
        printf("Make your move within %i ms.\n", gameState -> timeout);
    }

    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, BOARD)){

            receiveBoard(sockfd);

            /* request time to think */
            sendLineToServer(sockfd, buffer, THINKING);

        } else if(startsWith(buffer, OKTHINK)) {

            printf("Ok, make your move now!\n");

            /* send a signal to the Thinker that it should think */
            gameState -> think = true;
            kill(gameInfo -> thinkerPID, SIGUSR1);

            /* shortly wait for the update to arrive */
            sleep(1);

            /* read move from the pipe */
            if((read(pipefd[0], gameState -> move, n)) != n) {
                errNdie("Failed to read move from pipe.");
            }

            /* send the move to the server */
            printf("Sending move %s...\n", gameState -> move);
            stringConcat("PLAY ", gameState -> move, concatStr);
            sendLineToServer(sockfd, buffer, concatStr);

            waitMOVEOK();
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

    if(!SHMInfo.thinkerAttachedGameState) {
        attachGameState();
    }

    if(gameState -> think) {

        char *move = "A3:B4";
        int n = sizeof(move);

        printBoard();

        /* TODO calculate the next move here (hardcoded for now) */

        /* write move to the pipe */
        if((write(pipefd[1], move, n)) != n) {
            errNdie("Failed to write move to pipe.");
        }

        gameState -> think = false;
    }
}

void waitMOVEOK() {

    char buffer[BUFFER_SIZE] = { 0 };

    while(1){
        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

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

void waitStatement(const int sockfd, char *buffer) {
    sendLineToServer(sockfd, buffer, OKWAIT);
}

void performGameplay(const int sockfd, struct gameInfo *gameDataPointer) {

    char buffer[BUFFER_SIZE] = { 0 };
    char concatStr[BUFFER_SIZE] = { 0 };

    printf("Gameplay phase\n");

    /* create a shared memory segment for the game state */
    gameDataPointer -> shmidGameState = SHMAlloc(sizeof(struct gameState));

    /* attach game state to Connector process */
    gameState = (struct gameState*) SHMAttach(gameDataPointer -> shmidGameState);
    SHMInfo.connectorAttachedGameState = true;

    gameState -> rows = BOARDROWS;
    gameState -> cols = BOARDCOLS;
    gameState -> think = false;

    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

         if (buffer[0] == '-') {

            stringConcat("NACK received: ", buffer + 2, concatStr); /* + 2 to skip preceding "- " */

            /* remove newline at the end of the string bc perror adds more stuff to it */
            concatStr[strlen(concatStr)-1] = '\0';

            errNdie(concatStr);

        } else if(startsWith(buffer, GAMEOVER)) { /* game over */

            gameOverStatement(sockfd, gameDataPointer, buffer);
            break;

        } else if(startsWith(buffer, WAIT)) { /* idle */

            waitStatement(sockfd, buffer);

        } else if(startsWith(buffer, MOVE)) { /* move */

            moveStatement(sockfd, gameDataPointer, buffer);
        }
    }
}