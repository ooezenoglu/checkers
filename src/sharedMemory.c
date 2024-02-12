#include "../include/sharedMemory.h"

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
    // printf("Thinker: Attaching Opponent Info SHM segment...\n");
    oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);
    SHMInfo.thinkerAttachedOppInfo = true;
}

void attachGameState() {

    /* attach game state to Thinker process */
    // printf("Thinker: Attaching Game State SHM segment...\n");
    gameState = (struct gameState*) SHMAttach(gameInfo -> shmidGameState);
    SHMInfo.thinkerAttachedGameState = true;
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