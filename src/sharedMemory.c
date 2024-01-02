#include "sharedMemory.h"

int SHMAlloc(size_t size) {

    int id;

    if((id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0644)) == -1) {
        errNdie("Failed to create shared memory segment.");
    }

    return id;
}

void *SHMAttach(int shmid) {

    void *shmaddr = shmat(shmid, 0, 0);
    
    if(shmaddr == (void*) -1) {
        errNdie("Failed to attach shared memory segment.");
    }

    return shmaddr;
}

void attachOppInfo() {

    /* attach opponent info to Thinker process */
    printf("Thinker: Attaching Opponent Info SHM segment...\n");
    oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);
    SHMInfo.thinkerAttachedOppInfo = true;
}

void storeBoardInSHM() {

    /* create and attach a SHM segment that stores as many pointers to strings as there are rows */
    gameInfo -> shmidBoard = SHMAlloc(gameState.rows * sizeof(char**));
    gameState.board = (char**) SHMAttach(gameInfo -> shmidBoard);
    SHMInfo.connectorAttachedBoard = true;

    /* create and attach a SHM segment that stores as many shmids as there are rows */
    gameInfo -> shmidBoardRows = SHMAlloc(gameState.rows * sizeof(int));
    gameState.shmidBoardRowsPtr = (int*) SHMAttach(gameInfo -> shmidBoardRows);
    SHMInfo.connectorAttachedBoardRowIDs = true;

    /* for each row, create and attach a SHM segment that stores as many chars as there are columns;
    store the shmids in the separate SHM segment */
    for (int i = 0; i < gameState.rows; i++) {
        gameState.shmidBoardRowsPtr[i] = SHMAlloc(gameState.cols * sizeof (char));
        gameState.board[i] = (char *) SHMAttach(gameState.shmidBoardRowsPtr[i]);
    }

    SHMInfo.connectorAttachedBoardRows = true;
    gameInfo -> boardExistsInSHM = true;
}

void SHMDetach(const void *shmaddr) {

    if(shmdt(shmaddr) != 0) {
        errNdie("Failed to detach shared memory segment.");
    }
}

void SHMDestroy(int shmid) {

    if(shmctl(shmid, IPC_RMID, NULL) != 0) {
        errNdie("Failed to mark shared memory segment for destruction.");
    }
}