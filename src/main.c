#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "helpers.h"

#define GAMEKINDNAME "Checkers"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"


int main(int argc, char *argv[]) {

    char *gameID;
    int playerCount, sockfd;

    parseCommandLineArgs(argc, argv, &gameID, &playerCount);

    /* connect to game server via an TCP/IP socket */
    if((sockfd = connectToServer(HOSTNAME, PORTNUMBER)) < 0) {
        printf("Failed to establish connection with %s\n", HOSTNAME);
        exit(EXIT_FAILURE);
    }

    if (performConnection(sockfd, gameID) < 0) {
        printf("Prologue phase failed\n");
        exit(EXIT_FAILURE);
    }


    exit(EXIT_SUCCESS);
}