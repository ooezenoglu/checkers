#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "helpers.h"
#include "prolog.h"

#define BUFFER 4096
#define CLIENT_VERSION 2
#define END_OF_PROLOG "+ ENDPLAYERS\n"


int performConnection(int sockfd, const char *gameID) {

    int data_size;
    char *buffer = malloc(BUFFER * sizeof(char));
    char *resp[3];
    resp[0] = "VERSION 2";
    resp[1] = "ID 1yv8mkx3t1bcl";
    resp[2] = "PLAYER";
    int i = 0;

    while (strncmp(buffer, END_OF_PROLOG, strlen(END_OF_PROLOG)) != 0) {

        memset(buffer, '\0', sizeof(buffer));

        /* server sending */
        data_size = recv(sockfd, buffer, BUFFER-1, 0);
        printf("S: %s\n", buffer);

        if (buffer[0] == '-') { return -1; }

        memset(buffer, '\0', sizeof(buffer));

        /* client sending */
        if (i < 2) {
            sprintf(buffer, "%s\n", resp[i]);
            data_size = send(sockfd, buffer, strlen(buffer), 0);
            printf("C: %s\n", buffer);
            i++;
        }
    }

    free(buffer);
    return 0;
}