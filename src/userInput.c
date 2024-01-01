#include "userInput.h"

#define DEFAULT_CONFIG "client.conf"

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