#include "helpers.h"
#include "userInput.h"
#include "connection.h"
#include "sharedMemory.h"

#define CLIENT_VERSION "3.1"

pid_t pid;
int shmidGameInfo, wstatus;
int sockfd = -1;
struct gameInfo *gameInfo;
struct player *oppInfo;
bool thinkerAttachedGameInfo = false;
bool thinkerAttachedOppInfo = false;
bool connectorAttachedGameInfo = false;
bool connectorAttachedOppInfo = false;

int main(int argc, char *argv[]) {
    atexit(cleanup);
    signal(SIGUSR1, attachOppInfo);

    /* create a shared memory segmet to store the game data into */
    shmidGameInfo = SHMAlloc(sizeof(gameInfo));

    /* attach game info to Thinker process */
    gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);
    thinkerAttachedGameInfo = true;

    if((pid = fork()) < 0) {
        
        errNdie("Failed to fork.");

    } else if(pid == 0) { /* Connector process (child) */
        atexit(cleanupConnector);

        /* attach game info to Connector process */
        gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);
        connectorAttachedGameInfo = true;

        /* store PID of Connector process */
        gameInfo -> connectorPID = getpid();

        /* store PID of Thinker process */
        gameInfo -> thinkerPID = getppid();

        /* store the client version in the game data struct */
        memcpy(gameInfo -> clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION) + 1);

        /* read game ID and requested player number from the console */
        parseCommandLineArgs(argc, argv, gameInfo);

        /* parse and store game configuration from file */
        readConfigFile(gameInfo);

        /* connect to game server via TCP/IP socket */
        sockfd = connectToServer(gameInfo -> hostName, gameInfo -> port);

        /* perform prologue phase */
        performConnection(sockfd, gameInfo);

        /* workaround so that the Connector destroys the opponent 
        segment only if the Thinker attached it first */
        sleep(3);

        /* do Connector stuff */

    } else { /* Thinker process (parent) */
        atexit(cleanupThinker);

        /* waiting for Connector to finish execution */
        while(waitpid(pid, &wstatus, WNOHANG) == 0) {

            if(!thinkerAttachedOppInfo) { continue; }

            /* do Thinker stuff */
        }
        
        /* debugging */
        printWaitDetails(wstatus);

        /* finish execution since Connector terminated */
        exit(EXIT_SUCCESS);
    }
}