#include "gameplay.h"

#define BUFFER_SIZE 255

void receiveBoard(const int sockfd) {

    char buffer[BUFFER_SIZE] = { 0 };
    char *toks[gameState.cols];
    int i = 0;
    int len;

    if(!(gameInfo -> boardExistsInSHM)) { storeBoardInSHM(); }

    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, ENDBOARD)){ break; }

        stringTokenizer(buffer + 4, " ", toks, &len, gameState.cols);

        if(len != gameState.cols) { errNdie("Error in receiveBoard()."); }

        for(int j = 0; j < gameState.cols; j++) {
            gameState.board[i][j] = toks[j][0];
        }

        i++;
    }

    /* debugging */
    for (int i = 0; i < gameState.rows; i++) {
        for (int j = 0; j < gameState.cols; j++) {
            printf("%c ", gameState.board[i][j]);
        }
        printf("\n");
    }
}

void gameOverStatement(const int sockfd, struct gameInfo *gameDataPointer, char *buffer) {
    
    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, BOARD)){

            /* store board size */
            if(sscanf(buffer, "%*s %*s %i,%i", &(gameState.cols), &(gameState.rows)) != 2) {
                errNdie("Could not store game data.");
            }

            /* add / update the board in shared memory */
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
            errNdie("Error in gameOverStatement().");
        }
     }
}

void waitStatement(const int sockfd, char *buffer) {
    sendLineToServer(sockfd, buffer, OKWAIT);
}

void moveStatement(const int sockfd, struct gameInfo *gameDataPointer, char *buffer) {

    /* store & print timeout */
    if(sscanf(buffer, "%*s %*s %i", &(gameState.timeout)) != 1) {
        errNdie("Could not store game data.");
    } else {
        printf("Make your move within %i ms.\n", gameState.timeout);
    }

    while(1) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if(startsWith(buffer, BOARD)){

            /* store board size */
            if(sscanf(buffer, "%*s %*s %i,%i", &(gameState.cols), &(gameState.rows)) != 2) {
                errNdie("Could not store game data.");
            }

            /* add / update the board in shared memory */
            receiveBoard(sockfd);

            /* request time to think */
            sendLineToServer(sockfd, buffer, THINKING);

        } else if(startsWith(buffer, OKTHINK)) {

            printf("Ok, make your move now!\n");
            think(sockfd, buffer);
            break;

        } else if(startsWith(buffer, QUIT)) {

            errNdie("moveStatement(): Protocol error.");

        } else {

            /* for unexpected things */
            errNdie("Error in moveStatement().");
        }
    }
}

void think(const int sockfd, char *buffer) {
    
    char concatStr[BUFFER_SIZE] = { 0 };

    stringConcat(PLAY, " A3:B4", concatStr);
    printf("Playing A3:B4.\n");
    
    sendLineToServer(sockfd, buffer, concatStr);

    receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

    if(startsWith(buffer, MOVEOK)){

        printf("The move is valid.\n");

    } else if(startsWith(buffer, GAMEOVER)){

        gameOverStatement(sockfd, gameInfo, buffer);

    } else if(startsWith(buffer, QUIT)) {

        errNdie("think(): Protocol error.");
    
    } else {

        /* for unexpected things */
        errNdie("Error in think().");
    }
}

void performGameplay(const int sockfd, struct gameInfo *gameDataPointer) {

    char buffer[BUFFER_SIZE] = { 0 };
    char concatStr[BUFFER_SIZE] = { 0 };

    printf("Gameplay phase\n");

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