#include "helpers.h"
#include "userInput.h"
#include "connection.h"
#include "sharedMemory.h"
#include "gameplay.h"

#define CLIENT_VERSION "3.1"

pid_t pid;
int shmidGameInfo, wstatus;
int sockfd = -1;
int pipefd[2];
struct gameInfo *gameInfo;
struct player *oppInfo;
struct gameState *gameState;
struct SHMInfo SHMInfo = { false };

int main(int argc, char *argv[]) {
    atexit(cleanup);
    signal(SIGUSR2, attachOppInfo);
    signal(SIGUSR1, think);

    /* create a shared memory segmet to store the game data into */
    shmidGameInfo = SHMAlloc(sizeof(gameInfo));

    /* attach game info to Thinker process */
    gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);
    SHMInfo.thinkerAttachedGameInfo = true;

    /* set up an unnamed pipe for ipc */
    if(pipe(pipefd) < 0) { errNdie("Failed to create pipe."); }

    if((pid = fork()) < 0) {
        
        errNdie("Failed to fork.");

    } else if(pid == 0) { /* Connector process (child) */
        atexit(cleanupConnector);

        /* close write end of pipe */
        if(close(pipefd[1]) != 0) { errNdie("Failed to close write end of pipe."); }

        /* attach game info to Connector process */
        gameInfo = (struct gameInfo*) SHMAttach(shmidGameInfo);
        SHMInfo.connectorAttachedGameInfo = true;

        /* store PID of Connector & Thinker process */
        gameInfo -> connectorPID = getpid();
        gameInfo -> thinkerPID = getppid();

        /* store the client version in the game data struct */
        memcpy(gameInfo -> clientVersion, CLIENT_VERSION, strlen(CLIENT_VERSION) + 1);

        /* read game ID and requested player number from the console */
        parseCommandLineArgs(argc, argv);

        /* parse and store game configuration from file */
        readConfigFile();

        /* connect to game server via TCP/IP socket */
        connectToServer();

        /* perform prologue phase */
        performConnection();

        /* perform game play phase */
        performGameplay();

    } else { /* Thinker process (parent) */
        atexit(cleanupThinker);

        /* close read end of pipe */
        if(close(pipefd[0]) != 0) { errNdie("Failed to close read end of pipe."); }

        /* waiting for Connector to finish execution */
        while(waitpid(pid, &wstatus, WNOHANG) == 0) {

            if(!SHMInfo.thinkerAttachedOppInfo) { continue; }
        }
        
        /* debugging */
        printWaitDetails(wstatus);

        /* finish execution since Connector terminated */
        exit(EXIT_SUCCESS);
    }
}