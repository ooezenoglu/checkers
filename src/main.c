#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "helpers.h"

#define GAMEKINDNAME "Checkers"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"


int main(int argc, char *argv[]) {

    /* TODO: possibly make number of bits explicit */
    long gameID;
    int playerCount;

    parse_commandline_args(argc, argv, &gameID, &playerCount);
}