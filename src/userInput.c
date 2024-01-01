#include "userInput.h"

#define DEFAULT_CONFIG "client.conf"

void parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameInfoPtr) {

    int opt;
    int gflag = 0;
    int pflag = 0;
    int cflag = 0;

    if (argc < 3) {
        errNdie("Incorrect format. Please enter the data in the format -g <GAME-ID> -p <{1,2}> -c <CONFIG-FILE>.");
    }

    while ((opt = getopt(argc, argv, ":g:p::c::")) != -1) {
        switch (opt) {

            case 'g':

                if (optarg == NULL || strlen(optarg) != 13) {
                    errNdie("Game ID must be exactly 13 characters long.");
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
                        errNdie("If set, the player number must be 1 (opponent is the computer) or 2 (opponent is human).");
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
                errNdie("Option -g needs a value.");  

            case '?':  
                errNdie("Unknown option(s)."); 

            default:
                errNdie("Incorrect format. Please enter the data in the format -g <GAME-ID> -p <{1,2}>.");
        }
    }

    if(gflag == 0) {
        errNdie("Game ID unknown.");
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
}

void readConfigFile(struct gameInfo *gameInfoPtr) {
    
    FILE *fp;
    char line[256] = { 0 };
    char *key, *value;

    /* open the config file in read mode */
    if ((fp = fopen(gameInfoPtr -> configFile, "r")) == NULL) {
        errNdie("Error opening configuration file.");
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

            errNdie("Missing parameter name.");

        } else if(value == NULL) {

            errNdie("Missing parameter value.");

        } else {

            if (stringEquals(key, "Hostname")) {

                /* Extract hostname */
                memcpy(gameInfoPtr -> hostName, value, strlen(value) + 1);
                
            } else if (stringEquals(key, "PortNumber")) {

                /* Extract port number */
                if(sscanf(value, "%"SCNu16, &(gameInfoPtr -> port)) != 1) { 
                    errNdie("Could not store port number.");
                }

            } else if (stringEquals(key, "GameKindName")) {

                /* Extract game kind name */
                memcpy(gameInfoPtr -> gameKindName, value, strlen(value) + 1);

            } else {

                errNdie("Unknown parameter.");
            }
        }

        memset(line, 0, strlen(line));
    }

    fclose(fp);

    /* debugging */
    printf("Hostname: %s\n", gameInfoPtr -> hostName);
    printf("Port: %d\n", gameInfoPtr -> port);
    printf("Gamekind name: %s\n", gameInfoPtr -> gameKindName);
}