/* damit getopt() deklariert wird */
#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "helpers.h"

void parse_commandline_args(int argc, char *argv[], char **gameID, int *playerCount) {

    int opt;

    if (argc <= 1) {
        perror("Incorrect format");
        exit(EXIT_FAILURE);
    }

    /* parse command line arguments */
    while ((opt = getopt(argc, argv, "g:p:")) != -1) {
        switch (opt) {

            case 'g':
                *gameID = optarg;

                if(strlen(*gameID) != 13) {
                    perror("Game-ID must be exactly 13 characters long");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'p':
                *playerCount = atoi(optarg);

                if(*playerCount != 2) {
                    perror("Player count must be 2 for Checkers");
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                perror("Incorrect format");
                exit(EXIT_FAILURE);
        }
    }

    /* debugging */
    printf("Game-ID: %s\n", *gameID);
    printf("Spielernummer: %i\n", *playerCount);    
}