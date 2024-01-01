#include "sharedMemory.h"

int SHMAlloc(size_t size) {

    int id;

    if((id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0644)) == -1) {
        perror("Failed to create shared memory segment.");
        exit(EXIT_FAILURE);
    }

    return id;
}

void *SHMAttach(int shmid) {

    void *shmaddr = shmat(shmid, 0, 0);
    
    if(shmaddr == (void*) -1) {
        perror("Failed to attach shared memory segment.");
        exit(EXIT_FAILURE);
    }

    return shmaddr;
}

void attachOppInfo() {

    /* attach opponent info to Thinker process */
    printf("Thinker: Attaching Opponent Info SHM segment...\n");
    oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);
    thinkerAttachedOppInfo = true;
}

void SHMDetach(const void *shmaddr) {

    if(shmdt(shmaddr) != 0) {
        perror("Failed to detach shared memory segment.");
        exit(EXIT_FAILURE);
    }
}

void SHMDestroy(int shmid) {

    if(shmctl(shmid, IPC_RMID, NULL) != 0) {
        perror("Failed to mark shared memory segment for destruction.");
        exit(EXIT_FAILURE);
    }
}