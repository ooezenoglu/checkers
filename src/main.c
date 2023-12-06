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

#define CLIENT_VERSION "2.3"
#define GAMEKINDNAME "Checkers"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"


int main(int argc, char *argv[]) {

    int sockfd;
    struct gameInfo gameData;

    /* read game ID and desired player number from the console */
    if(parseCommandLineArgs(argc, argv, &gameData) < 0) {
        perror("Failed to parse command line arguments."),
        exit(EXIT_FAILURE);
    }

    /* connect to game server via TCP/IP socket */
    if((sockfd = connectToServer(HOSTNAME, PORTNUMBER)) < 0) {
        printf("Failed to establish connection with %s\n", HOSTNAME);
        exit(EXIT_FAILURE);
    }

    /* store the client version in the game data struct */
    memcpy(gameData.clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION));
    gameData.clientVersion[strlen(CLIENT_VERSION)] = '\0';

    if (performConnection(sockfd, &gameData) < 0) {
        perror("Prologue phase failed. Closing socket...");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}