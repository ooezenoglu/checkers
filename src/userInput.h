#ifndef userInput
#define userInput

#include "helpers.h"
#include "stringUtils.h"

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameInfoPtr);
int readConfigFile(struct gameInfo *gameInfoPtr);

#endif