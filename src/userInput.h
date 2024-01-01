#ifndef userInput
#define userInput

#include "helpers.h"
#include "stringUtils.h"

void parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameInfoPtr);
void readConfigFile(struct gameInfo *gameInfoPtr);

#endif