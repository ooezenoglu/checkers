#ifndef sharedMemory
#define sharedMemory

#include "helpers.h"

int SHMAlloc(size_t size);
void *SHMAttach(int shmid);
void attachOppInfo();
void attachGameState();
void SHMDetach(const void *shmaddr);
void SHMDestroy(int shmid);

#endif