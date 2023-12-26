#include "helpers.h"

#define CLIENT_VERSION "3.1"

int main(int argc, char *argv[]) {

    pid_t pid;
    int shmidGameInfo, wstatus, sockfd;
    struct gameInfo *gameInfo;
    struct player *oppInfo;

    /* create a shared memory segmet to store the game data into */
    shmidGameInfo = SHMAlloc(sizeof(gameInfo));

    if((pid = fork()) < 0) {
        
        perror("Failed to fork.");
        exit(EXIT_FAILURE);

    } else if(pid == 0) { /* Connector process (child) */

        /* attach game info to Connector process */
        gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);

        /* store PID of Connector process */
        gameInfo -> connectorPID = getpid();

        /* store PID of Thinker process */
        gameInfo -> thinkerPID = getppid();

        /* store the client version in the game data struct */
        memcpy(gameInfo -> clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION) + 1);

        /* read game ID and requested player number from the console */
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
        if((sockfd = connectToServer(gameInfo -> hostName, gameInfo -> port)) < 0) {
            perror("Failed to establish connection with Game Server.");
            exit(EXIT_FAILURE);
        }

        /* perform prologue phase */
        if (performConnection(sockfd, gameInfo) < 0) {
            perror("Prologue phase failed. Closing socket...");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        SHMDetach(gameInfo);
        SHMDestroy(gameInfo -> shmidOpponents);

        exit(EXIT_SUCCESS);

    } else { /* Thinker process (parent) */

        /* attach game info to Thinker process */
        gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);
        
        /* TODO remove this once Thinker attaches oppInfo when Connector is finished storing the opponent data */
        sleep(1);

        /* attach opponent info to Thinker process */
        oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);

        /* waiting for Connector to finish execution */
        if((waitpid(pid, &wstatus, 0)) < 0) {

            perror("Failed to wait for Connector process.");
            exit(EXIT_FAILURE);

        } else {

            /* debugging */
            printWaitDetails(wstatus);

            SHMDetach(oppInfo);
            SHMDetach(gameInfo);
            SHMDestroy(shmidGameInfo);

            /* finish execution since Connector terminated */
            exit(EXIT_SUCCESS);
        }
    }
}