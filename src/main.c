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
    int shmidGameInfo, wstatus, sockfd;
    struct gameInfo *gameInfo;
    struct player *oppInfo;

    /* create a shared memory segmet to store the game data into */
    if((shmidGameInfo = shmget(IPC_PRIVATE, sizeof(gameInfo), IPC_CREAT | 0644)) == -1) {

        perror("Failed to create shared memory segment.");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) < 0) {
        
        perror("Failed to fork.");
        exit(EXIT_FAILURE);

    } else if(pid == 0) { /* Connector process */

        if((gameInfo = (struct gameInfo*) shmat(shmidGameInfo, 0, 0)) == (void*) -1) {
            perror("Failed to attach shared memory segment (game info) to child (connector) process.");            
        }

        /* store PID of Connector process */
        gameInfo -> connectorPID = getpid();

        /* store PID of Thinker process */
        gameInfo -> thinkerPID = getppid();

        /* store the client version in the game data struct */
        memcpy(gameInfo -> clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION) + 1);

        /* read game ID and desired player number from the console */
        if(parseCommandLineArgs(argc, argv, gameInfo) < 0) {
            perror("Failed to parse command line arguments.");
            exit(EXIT_FAILURE);
        }

        /* parse and store game configuration from file */
        if(readConfigFile(gameInfo) < 0) {
            perror("Failed to read configuration file.");
            exit(EXIT_FAILURE);
        }

        /* connect to game server via TCP/IP socket */
        if((sockfd = connectToServer(gameInfo -> hostName, gameInfo -> portNumber)) < 0) {
            printf("Failed to establish connection with %s\n", gameInfo -> hostName);
            exit(EXIT_FAILURE);
        }

        if (performConnection(sockfd, gameInfo) < 0) {
            perror("Prologue phase failed. Closing socket...");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if(shmdt(gameInfo) == -1) {
            perror("Failed to detach shared memory segment (game info) in the Connector.");
        }

        exit(EXIT_SUCCESS);

    } else { /* Thinker process */

        if((gameInfo = (struct gameInfo*) shmat(shmidGameInfo, 0, 0)) == (void*) -1) {
            perror("Failed to attach shared memory segment (game info) to parent (thinker) process.");
        }

        /* TODO remove this once Thinker attaches oppInfo when Connector is finished storing the opponent data */
        sleep(1);

        if((oppInfo = (struct player*) shmat(gameInfo -> shmidOpponents, 0, 0)) == (void*) -1) {
            perror("Failed to attach shared memory segment (opponents) to parent (thinker) process.");
        }

        /* waiting for Connector to finish execution */
        if((waitpid(pid, &wstatus, 0)) < 0) {

            perror("Failed to wait for Connector process.");
            exit(EXIT_FAILURE);

        } else {

            /* debugging */
            printWaitDetails(wstatus);


            if(shmdt(oppInfo) == -1) {
                perror("Failed to detach shared memory segment (opponents) in the Thinker.");
            }

            if(shmctl(gameInfo -> shmidOpponents, IPC_RMID, NULL) == -1) {
                perror("Failed to destroy shared memory segment (opponents).");
            }

            if(shmdt(gameInfo) == -1) {
                perror("Failed to detach shared memory segment (game info) in the Thinker.");
            }

            /* destroy shared memory segment */
            if(shmctl(shmidGameInfo, IPC_RMID, NULL) == -1) {
                perror("Failed to destroy shared memory segment (game info).");
            }

            /* finish execution since connector terminated */
            exit(EXIT_SUCCESS);
        }
    }
}