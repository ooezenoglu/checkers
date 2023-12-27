#include "helpers.h"

#define DEFAULT_CONFIG "client.conf"

void cleanup() {

    if(sockfd != -1) {
        printf("Closing socket...\n");
        close(sockfd);
    }
}

void cleanupThinker() {

    if(thinkerAttachedOppInfo) {
        printf("Thinker: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
    }

    if(thinkerAttachedGameInfo) {
        printf("Thinker: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
        SHMDestroy(shmidGameInfo);
    }
}

void cleanupConnector() {
    
    if(connectorAttachedOppInfo) {
        printf("Connector: Detaching Opponent Info SHM segment...\n");
        SHMDetach(oppInfo);
        SHMDestroy(gameInfo -> shmidOpponents);
    }

    if(connectorAttachedGameInfo) {
        printf("Connector: Detaching Game Info SHM segment...\n");
        SHMDetach(gameInfo);
    }
}

void attachOppInfo() {

    /* attach opponent info to Thinker process */
    printf("Thinker: Attaching Opponent Info SHM segment...\n");
    oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);
    thinkerAttachedOppInfo = true;
}

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameInfoPtr) {

    int opt;
    int gflag = 0;
    int pflag = 0;
    int cflag = 0;

    if (argc < 3) {
        perror("Incorrect format. Please enter the data in the format -g <GAME-ID> -p <{1,2}> -c <CONFIG-FILE>.");
        return -1;
    }

    while ((opt = getopt(argc, argv, ":g:p::c::")) != -1) {
        switch (opt) {

            case 'g':

                if (optarg == NULL || strlen(optarg) != 13) {
                    perror("Game ID must be exactly 13 characters long.");
                    return -1;
                }

                memcpy(gameInfoPtr -> gameID, optarg, strlen(optarg) + 1);

                /* game ID is set correctly */
                gflag = 1;

                break;

            case 'p':

                /* checks whether a value for -p is set */
                if (optind < argc && argv[optind][0] != '-') {

                    gameInfoPtr -> requestedPlayerNumber = atoi(argv[optind]);

                    if (gameInfoPtr -> requestedPlayerNumber != 1 && gameInfoPtr -> requestedPlayerNumber != 2) {
                        perror("If set, the player number must be 1 (opponent is the computer) or 2 (opponent is human).");
                        return -1;
                    }

                    /* player number is set correctly */
                    pflag = 1;
                }

                break;

            case 'c':

                /* checks whether a file name is set */
                if(optind < argc && argv[optind][0] != '-') {

                    memcpy(gameInfoPtr -> configFile, argv[optind], strlen(argv[optind]) + 1);
                    cflag = 1;
                }

                break;

            case ':':  
                perror("Option -g needs a value.");  
                return -1;

            case '?':  
                perror("Unknown option(s)."); 
                return -1;

            default:
                perror("Incorrect format. Please enter the data in the format -g <GAME-ID> -p <{1,2}>.");
                return -1;
        }
    }

    if(gflag == 0) {
        perror("Game ID unknown.");
        return -1;
    }

    /* default value if -p is not set */
    if(pflag == 0) {
        gameInfoPtr -> requestedPlayerNumber = -1;
    }

    /* default value if -c is not set */
    if (cflag == 0) {
        memcpy(gameInfoPtr -> configFile, DEFAULT_CONFIG, strlen(DEFAULT_CONFIG) + 1);
    }

    /* debugging */
    printf("Game ID: %s\n", gameInfoPtr -> gameID);
    printf("Player number: %i\n", gameInfoPtr -> requestedPlayerNumber);
    printf("Config file: %s\n", gameInfoPtr -> configFile);

    return 0;
}

int readConfigFile(struct gameInfo *gameInfoPtr) {
    
    FILE *fp;
    char line[256] = { 0 };
    char *key, *value;

    /* open the config file in read mode */
    if ((fp = fopen(gameInfoPtr -> configFile, "r")) == NULL) {
        perror("Error opening configuration file.");
        return -1;
    }
    
    while(fgets(line, sizeof(line), fp) != NULL) {

        /* Remove newline character at the end */
        line[strlen(line)] = '\0';

        /* Seperate key and value and remove = */
        key = strtok(line, "=");
        value = strtok(NULL, "=");

        /* Remove empty spaces */
        key = strtok(key, " \t\n\r");
        value = strtok(value, " \t\n\r");

        if(key == NULL) {

            perror("Missing parameter name.");
            return -1;

        } else if(value == NULL) {

            perror("Missing parameter value.");
            return -1;

        } else {

            if (stringEquals(key, "Hostname")) {

                /* Extract hostname */
                memcpy(gameInfoPtr -> hostName, value, strlen(value) + 1);
                
            } else if (stringEquals(key, "PortNumber")) {

                /* Extract port number */
                if(sscanf(value, "%"SCNu16, &(gameInfoPtr -> port)) != 1) { 
                    perror("Could not store port number.");
                    return -1;
                }

            } else if (stringEquals(key, "GameKindName")) {

                /* Extract game kind name */
                memcpy(gameInfoPtr -> gameKindName, value, strlen(value) + 1);

            } else {

                perror("Unknown parameter.");
                return -1;
            }
        }

        memset(line, 0, strlen(line));
    }

    fclose(fp);

    /* debugging */
    printf("Hostname: %s\n", gameInfoPtr -> hostName);
    printf("Port: %d\n", gameInfoPtr -> port);
    printf("Gamekind name: %s\n", gameInfoPtr -> gameKindName);

    return 0;
}

void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* read-in data until the first newline character is reached */
    for(int i = 0; i < bufferSize; i++) {

        if(recv(sockfd, &buffer[i], 1, 0) != 1) {
            perror("Failed to receive line from server.");
            exit(EXIT_FAILURE);
        }

        if (buffer[i] == '\n') {
            buffer[i+1] = '\0';
            break;
        }
    }

    /* debugging */
    // printf("S: %s\n", buffer);
}

void sendLineToServer(const int sockfd, char *buffer, const char *line) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* write into the buffer and finish with newline character */
    if(sprintf(&buffer[0], "%s\n", line) < 0) {
        perror("Failed to write line to buffer.");
        exit(EXIT_FAILURE);
    }

    if(send(sockfd, buffer, strlen(buffer), 0) == -1) {
        perror("Failed to send line to server.");
        exit(EXIT_FAILURE);
    }

    /* debugging */
    // printf("C: %s\n", buffer);
}

bool startsWith(const char *s1, const char *s2) {

    int n = (strlen(s1) <= strlen(s2)) ? strlen(s1) : strlen(s2);

    return strncmp(s1, s2, n) == 0;
}

bool stringEquals(const char *s1, const char *s2) {

    int s1Length = strlen(s1);
    int s2Length = strlen(s2);

    if(s1Length != s2Length) {
        return false;
    }

    return strncmp(s1, s2, s1Length) == 0;
}

void stringConcat(const char *leftString, const char *rightString, char *dest) {

    if(leftString == NULL && rightString != NULL) {
        /* copy the right string + /0 to the destination */
        if(memcpy(dest, rightString, strlen(rightString) + 1) == NULL) {
            perror("Failed to copy string.");
            exit(EXIT_FAILURE);
        }
    }

    if(leftString != NULL && rightString == NULL) { 
        /* copy the left string + /0 to the destination */
        if(memcpy(dest, leftString, strlen(leftString) + 1) == NULL) {
            perror("Failed to copy string.");
            exit(EXIT_FAILURE);
        };
    }

    if(leftString == NULL && rightString == NULL) {
        perror("Both string arguments are NULL.");
        exit(EXIT_FAILURE);
    }

    /* copy the left string to the destination */
    if(memcpy(dest, leftString, strlen(leftString)) == NULL) {
        perror("Failed to copy string.");
        exit(EXIT_FAILURE);
    }

    /* concatenate the right string and its null terminator to the destination */
    if(strncat(dest, rightString, strlen(rightString) + 1) == NULL) {
        perror("Failed to concatenate strings.");
        exit(EXIT_FAILURE);
    }
}

void stringTokenizer(char *src, char *delim, char **res, int *len) {

    char *token;
    int i = 0;

    token = strtok(src, delim);

    while (token != NULL) {
        res[i] = token;
        i++;
        token = strtok(NULL, delim);
    }

    *len = i;

    /* debugging */
    for(int j = 0; j < *len; j++) {
        printf("TOKEN: %s\n", res[j]);
    }
}

void printWaitDetails(int wstatus) {

    if (WIFEXITED(wstatus)) {
        printf("Connector exited with status %d.\n", WEXITSTATUS(wstatus));

    } else if (WIFSIGNALED(wstatus)) {
        
        printf("Connector was terminated by signal %d.\n", WTERMSIG(wstatus));

    } else if (WIFSTOPPED(wstatus)) {

        printf("Connector was stopped by signal %d.\n", WSTOPSIG(wstatus));

    } else {
                
        printf("Connector terminated for unknown reasons.\n");
    }
}

int SHMAlloc(size_t size) {

    int id;

    if((id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0644)) == -1) {
        perror("Failed to create shared memory segment.");
        exit(EXIT_FAILURE);
    }

    return id;
}

void *SHMAttach(int shmid) {

    void *shmaddr = shmat(shmid, 0, 0);
    
    if(shmaddr == (void*) -1) {
        perror("Failed to attach shared memory segment.");
        exit(EXIT_FAILURE);
    }

    return shmaddr;
}

void SHMDetach(const void *shmaddr) {

    if(shmdt(shmaddr) != 0) {
        perror("Failed to detach shared memory segment.");
        exit(EXIT_FAILURE);
    }
}

void SHMDestroy(int shmid) {

    if(shmctl(shmid, IPC_RMID, NULL) != 0) {
        perror("Failed to mark shared memory segment for destruction.");
        exit(EXIT_FAILURE);
    }
}