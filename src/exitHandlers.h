#ifndef exitHandlers
#define exitHandlers

#include "helpers.h"
#include "sharedMemory.h"

void cleanup();
void cleanupThinker();
void cleanupConnector();
void printWaitDetails(int wstatus);

#endif