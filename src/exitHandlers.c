#include "exitHandlers.h"

void errNdie(char *msg) {

    perror(msg);
    exit(EXIT_FAILURE);
}

void cleanup() {

    if(sockfd != -1) {
        printf("Closing socket...\n");
        close(sockfd);
    }
}

void cleanupThinker() {

    close(pipefd[1]);

    if(SHMInfo.thinkerAttachedOppInfo) {
        printf("Thinker: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
    }

    if(SHMInfo.thinkerAttachedGameState) {
        printf("Thinker: Detaching Game State SHM segment...\n");
        SHMDetach(gameState);
    }

    if(SHMInfo.thinkerAttachedGameInfo) {
        printf("Thinker: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
        SHMDestroy(shmidGameInfo);
    }
}

void cleanupConnector() {
    
    close(pipefd[0]);

    if(SHMInfo.connectorAttachedOppInfo) {
        printf("Connector: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
        SHMDestroy(gameInfo -> shmidOpponents);
    }

    if(SHMInfo.connectorAttachedGameState) {
        printf("Connector: Detaching Game State SHM segment...\n");
        SHMDetach(gameState);
        SHMDestroy(gameInfo -> shmidGameState);
    }

    if(SHMInfo.connectorAttachedGameInfo) {
        printf("Connector: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
    }
}

void printWaitDetails(int wstatus) {

    if(WIFEXITED(wstatus)) {
        
        printf("Connector exited with status %d.\n", WEXITSTATUS(wstatus));

    } else if(WIFSIGNALED(wstatus)) {
        
        printf("Connector was terminated by signal %d.\n", WTERMSIG(wstatus));

    } else if(WIFSTOPPED(wstatus)) {

        printf("Connector was stopped by signal %d.\n", WSTOPSIG(wstatus));

    } else {

        printf("Connector terminated for unknown reasons.\n");
    }
}