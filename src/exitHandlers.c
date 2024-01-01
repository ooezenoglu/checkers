#include "exitHandlers.h"

void cleanup() {

    if(sockfd != -1) {
        printf("Closing socket...\n");
        close(sockfd);
    }
}

void cleanupThinker() {

    if(thinkerAttachedOppInfo) {
        printf("Thinker: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
    }

    if(thinkerAttachedGameInfo) {
        printf("Thinker: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
        SHMDestroy(shmidGameInfo);
    }
}

void cleanupConnector() {
    
    if(connectorAttachedOppInfo) {
        printf("Connector: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
        SHMDestroy(gameInfo -> shmidOpponents);
    }

    if(connectorAttachedGameInfo) {
        printf("Connector: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
    }
}

void printWaitDetails(int wstatus) {

    if (WIFEXITED(wstatus)) {
        printf("Connector exited with status %d.\n", WEXITSTATUS(wstatus));

    } else if (WIFSIGNALED(wstatus)) {
        
        printf("Connector was terminated by signal %d.\n", WTERMSIG(wstatus));

    } else if (WIFSTOPPED(wstatus)) {

        printf("Connector was stopped by signal %d.\n", WSTOPSIG(wstatus));

    } else {
                
        printf("Connector terminated for unknown reasons.\n");
    }
}