#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "helpers.h"
#include "prolog.h"

#define BUFFER 255
#define CLIENT_VERSION 2
#define END_OF_PROLOG "+ ENDPLAYERS\n"


int performConnection(int sockfd, const char *gameID) {

    char buffer[BUFFER]; 
    char *resp[3];

    /* TODO allow storing arbitrary game data */
    resp[0] = "VERSION 2";
    resp[1] = "ID 1yv8mkx3t1bcl";
    resp[2] = "PLAYER";
    int i = 0;

    while (strncmp(buffer, END_OF_PROLOG, strlen(END_OF_PROLOG)) != 0) {

        memset(buffer, 0, strlen(buffer));

        /* server sending */
        recv(sockfd, buffer, sizeof(buffer), 0);
        printf("S: %s\n", buffer);

        /* NACK received; abort */
        if (buffer[0] == '-') { return -1; }

        memset(buffer, 0, strlen(buffer));

        /* client sending */
        if (i < 2) {
            sprintf(&buffer[0], "%s\n", resp[i]);
            send(sockfd, buffer, strlen(buffer), 0);
            printf("C: %s\n", buffer);
            i++;
        }
    }

    return 0;
}