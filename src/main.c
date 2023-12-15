#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include "helpers.h"

#define CLIENT_VERSION "2.3"

int main(int argc, char *argv[]) {

    pid_t pid;
    int segmentID, cstatus, sockfd;
    struct gameInfo *gameData;

    /* create a shared memory segmet to store the game data into */
    if((segmentID = shmget(IPC_PRIVATE, sizeof(gameData), IPC_CREAT | 0644)) == -1) {

        perror("Failed to create shared memory segment.");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) < 0) {
        
        perror("Failed to fork.");
        exit(EXIT_FAILURE);

    } else if(pid == 0) { /* Connector process */

        if((gameData = (struct gameInfo*) shmat(segmentID, 0, 0)) == (void*) -1) {
            perror("Failed to attach shared memory segment to parent (thinker) process.");
        }

        /* read game ID and desired player number from the console */
        if(parseCommandLineArgs(argc, argv, gameData) < 0) {
            perror("Failed to parse command line arguments.");
            exit(EXIT_FAILURE);
        }

        /* parse and store game configuration from file */
        if(readConfigFile(gameData) < 0) {
            perror("Failed to read configuration file.");
            exit(EXIT_FAILURE);
        }

        /* connect to game server via TCP/IP socket */
        if((sockfd = connectToServer(gameData -> hostName, gameData -> portNumber)) < 0) {
            printf("Failed to establish connection with %s\n", gameData -> hostName);
            exit(EXIT_FAILURE);
        }

        /* store the client version in the game data struct */
        memcpy(gameData -> clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION) + 1);

        if (performConnection(sockfd, gameData) < 0) {
            perror("Prologue phase failed. Closing socket...");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if(shmdt(gameData) == -1) {
            perror("Failed to detach shared memory segment in the Connector.");
        }

        exit(EXIT_SUCCESS);

    } else { /* Thinker process */

        if((gameData = (struct gameInfo*) shmat(segmentID, 0, 0)) == (void*) -1) {
            perror("Failed to attach shared memory segment to parent (thinker) process.");
        }

        /* waiting for Connector to finish execution */
        if((waitpid(pid, &cstatus, 0)) < 0) {

            perror("Failed to wait for Connector process.");
            exit(EXIT_FAILURE);

        } else {

            /* debugging */
            if (WIFEXITED(cstatus)) {
                printf("Connector exited with status %d.\n", WEXITSTATUS(cstatus));

            } else if (WIFSIGNALED(cstatus)) {

                printf("Connector was terminated by signal %d.\n", WTERMSIG(cstatus));

            } else if (WIFSTOPPED(cstatus)) {

                printf("Connector was stopped by signal %d.\n", WSTOPSIG(cstatus));

            } else {
                
                printf("Connector terminated for unknown reasons.\n");
            }

            if(shmdt(gameData) == -1) {
                perror("Failed to detach shared memory segment in the Thinker.");
            }

            /* destroy shared memory segment */
            if(shmctl(segmentID, IPC_RMID, NULL) == -1) {
                perror("Failed to destroy shared memory segment.");
            }

            /* finish execution since connector terminated */
            exit(EXIT_SUCCESS);
        }
    }
}