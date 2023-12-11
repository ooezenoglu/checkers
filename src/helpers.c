#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/socket.h>
#include "helpers.h"

#define DEFAULT_CONFIG "client.conf"

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameDataPointer) {

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

                memcpy(gameDataPointer -> gameID, optarg, strlen(optarg) + 1);

                /* game ID is set correctly */
                gflag = 1;

                break;

            case 'p':

                /* checks whether a value for -p is set */
                if (optind < argc && argv[optind][0] != '-') {

                    gameDataPointer -> desPlayerNumber = atoi(argv[optind]);

                    if (gameDataPointer -> desPlayerNumber != 1 && gameDataPointer -> desPlayerNumber != 2) {
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

                    memcpy(gameDataPointer -> configFile, argv[optind], strlen(argv[optind]) + 1);
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
        gameDataPointer -> desPlayerNumber = -1;
    }

    /* default value if -c is not set */
    if (cflag == 0) {
        memcpy(gameDataPointer -> configFile, DEFAULT_CONFIG, strlen(DEFAULT_CONFIG) + 1);
    }

    /* debugging */
    printf("Game ID: %s\n", gameDataPointer -> gameID);
    printf("Player number: %i\n", gameDataPointer -> desPlayerNumber);
    printf("Config file: %s\n", gameDataPointer -> configFile);

    return 0;
}

int readConfigFile(struct gameInfo *gameDataPointer) {
    
    FILE *fp;
    char line[256] = { 0 };
    char *key, *value;

    /* open the config file in read mode */
    if ((fp = fopen(gameDataPointer -> configFile, "r")) == NULL) {
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

            if (stringCompare(key, "Hostname")) {

                /* Extract hostname */
                memcpy(gameDataPointer -> hostName, value, strlen(value) + 1);
                
            } else if (stringCompare(key, "PortNumber")) {

                /* Extract port number */
                if(sscanf(value, "%"SCNu16, &(gameDataPointer -> portNumber)) != 1) { 
                    perror("Could not store port number.");
                    return -1;
                }

            } else if (stringCompare(key, "GameKindName")) {

                /* Extract game kind name */
                memcpy(gameDataPointer -> gameKindName, value, strlen(value) + 1);

            } else {

                perror("Unknown parameter.");
                return -1;
            }
        }

        memset(line, 0, strlen(line));
    }

    fclose(fp);

    /* debugging */
    printf("Hostname: %s\n", gameDataPointer -> hostName);
    printf("Port: %d\n", gameDataPointer -> portNumber);
    printf("Gamekind name: %s\n", gameDataPointer -> gameKindName);

    return 0;
}

int receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* read-in data until the first newline character is reached */
    for(int i = 0; i < bufferSize; i++) {

        if(recv(sockfd, &buffer[i], 1, 0) != 1) {
            perror("Failed to receive line from server.");
            return -1;
        }

        if (buffer[i] == '\n') {
            buffer[i+1] = '\0';
            break;
        }
    }

    /* debugging */
    // printf("S: %s\n", buffer);

    return 0;
}

int sendLineToServer(const int sockfd, char *buffer, const char *line) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* write into the buffer and finish with newline character */
    if(sprintf(&buffer[0], "%s\n", line) < 0) {
        perror("Failed to write line to buffer.");
        return -1;
    }

    if(send(sockfd, buffer, strlen(buffer), 0) == -1) {
        perror("Failed to send line to server.");
        return -1; 
    }

    /* debugging */
    // printf("C: %s\n", buffer);

    return 0;
}

bool stringCompare(const char *s1, const char *s2) {

    int n = (strlen(s1) <= strlen(s2)) ? strlen(s1) : strlen(s2);

    return strncmp(s1, s2, n) == 0;
}

int stringConcat(const char *leftString, const char *rightString, char *dest) {

    if(leftString == NULL && rightString != NULL) {
        /* copy the right string + /0 to the destination */
        if(memcpy(dest, rightString, strlen(rightString) + 1) == NULL) {
            perror("Failed to copy string.");
            return -1;
        }

        return 0;
    }

    if(leftString != NULL && rightString == NULL) { 
        /* copy the left string + /0 to the destination */
        if(memcpy(dest, leftString, strlen(leftString) + 1) == NULL) {
            perror("Failed to copy string.");
            return -1;
        };

        return 0;
    }

    if(leftString == NULL && rightString == NULL) {
        perror("Both string arguments are NULL.");
        return -1; 
    }

    /* copy the left string to the destination */
    if(memcpy(dest, leftString, strlen(leftString)) == NULL) {
        perror("Failed to copy string.");
        return -1;
    }

    /* concatenate the right string and its null terminator to the destination */
    if(strncat(dest, rightString, strlen(rightString) + 1) == NULL) {
        perror("Failed to concatenate strings.");
        return -1;
    }

    return 0;
}

int stringTokenizer(char *src, char *delim, char **res, int *len) {

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

    return 0;
}