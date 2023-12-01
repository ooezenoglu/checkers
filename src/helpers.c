/* damit getopt() deklariert wird */
#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "helpers.h"

void parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameDataPointer) {

    int opt;

    if (argc <= 1) {
        perror("Incorrect format");
        exit(EXIT_FAILURE);
    }

    /* parse command line arguments */
    while ((opt = getopt(argc, argv, "g:p:")) != -1) {
        switch (opt) {

            case 'g':
                gameDataPointer -> gameID = optarg;

                if(strlen(gameDataPointer -> gameID) != 13) {
                    perror("Game-ID must be exactly 13 characters long");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'p':
                gameDataPointer -> desPlayerNumber = atoi(optarg);

                if(gameDataPointer -> desPlayerNumber != 1 && gameDataPointer -> desPlayerNumber != 2) {
                    perror("The player number must be 1 or 2.");
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                perror("Incorrect format");
                exit(EXIT_FAILURE);
        }
    }

    /* debugging */
    printf("Game-ID: %s\n", gameDataPointer -> gameID);
    printf("Spielernummer: %i\n", gameDataPointer -> desPlayerNumber);    
}

void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* read-in data until the first newline character is reached */
    for(int i = 0; i < bufferSize && recv(sockfd, &buffer[i], 1, 0) == 1; i++) {
        if (buffer[i] == '\n') {
            buffer[i+1] = '\0';
            break;
        }
    }

    /* debugging */
    printf("S: %s\n", buffer);
}

void sendLineToServer(const int sockfd, char *buffer, const char *line) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* write into the buffer and finish with newline character */
    sprintf(&buffer[0], "%s\n", line);
    send(sockfd, buffer, strlen(buffer), 0);

    /* debugging */
    printf("C: %s\n", buffer);
}

bool stringCompare(const char *s1, const char *s2) {
    return strncmp(s1, s2, strlen(s2)) == 0;
}

char *stringConcat(const char *leftString, const char *rightString) {

    if(leftString == NULL) { return (char *) rightString; }
    if(rightString == NULL) { return (char *) leftString; }

    /* allocate and clear memory for destination string 
    note that we need one more byte for the null terminator /0 */
    char *dest = calloc(strlen(leftString) + strlen(rightString) + 1, sizeof(char));

    /* copy the left string to the destination */
    memcpy(dest, leftString, strlen(leftString));

    /* concatenate the right string and it's null terminator to the destination */
    strncat(dest, rightString, strlen(rightString) + 1);

    return dest;
}
