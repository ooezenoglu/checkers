/* damit getopt() deklariert wird */
#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "helpers.h"

void parse_commandline_args(int argc, char *argv[], long *gameID, int *playerCount) {

    int opt;

    if (argc <= 1) {
        perror("Incorrect format");
        exit(EXIT_FAILURE);
    }

    /* parse command line arguments */
    while ((opt = getopt(argc, argv, "g:p:")) != -1) {
        switch (opt) {

            case 'g':
                *gameID = atol(optarg);

                /* check that gameID is positive and has length 13;
                note that whether gameID = 0 is tested seperately and 
                before testing the length ( otherwise error due to log10(0) ) */
                if(*gameID <= 0 || floor(log10(*gameID)) + 1 != 13) {
                    perror("Game-ID must be positive and exactly 13 digits long");
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
    printf("Game-ID: %ld\n", *gameID);
    printf("Spielernummer: %i\n", *playerCount);    
}