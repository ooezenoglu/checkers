#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <stdint.h>
#include "helpers.h"

#define BUFFER_SIZE 255

int performConnection(const int sockfd, struct gameInfo *gameDataPointer) {

    char buffer[BUFFER_SIZE] = { 0 };
    char concatStr[BUFFER_SIZE] = { 0 };
    bool endOfPrologReached = false;
    char desPlayerNumberAsStr[12];
    char *pstart, *pend;
    int istart, iend, j;

    while(!endOfPrologReached) {

        if(receiveLineFromServer(sockfd, buffer, BUFFER_SIZE) < 0) {
            perror("Failed to receive line from server.");
            return -1;
        }

        if (buffer[0] == '-') {
            
            stringConcat("NACK received: ", buffer + 2, concatStr); /* + 2 to skip preceding "- " */

            /* remove newline at the end of the string bc perror adds more stuff to it */
            concatStr[strlen(concatStr)-1] = '\0';

            perror(concatStr);
            return -1;

        } else if(stringCompare(buffer, "+ MNM Gameserver")) {

            /* store & print gameserver version */
            if(sscanf(buffer, "%*s%*s%*s%s", gameDataPointer -> serverVersion) != 1) { 
                perror("Could not store game data.");
                return -1;
            } else {
                printf("Connecting with the MNM Gameserver %s...\n", gameDataPointer -> serverVersion);
            }

            /* generate & send client version response */
            if (stringConcat("VERSION ", gameDataPointer -> clientVersion, concatStr) < 0) {
                perror("Could not generate client version response.");
                return -1;
            } else {
                /* send client version (note that it must match the game server major version) */
                if(sendLineToServer(sockfd, buffer, concatStr) < 0) {
                    perror("Failed to send client version to server.");
                    return -1;
                }
                /* clear helper variable after use */
                memset(concatStr, 0, strlen(concatStr));
            }

        } else if(stringCompare(buffer, "+ Client version accepted - please send Game-ID to join")) {

            printf("MNM Gameserver accepted client version %s\n", gameDataPointer -> clientVersion); 

            /* genererate & send game ID response */
            if (stringConcat("ID ", gameDataPointer -> gameID, concatStr) < 0) {
                perror("Could not generate game ID response.");
                return -1;
            } else {
                /* send game ID */
                if(sendLineToServer(sockfd, buffer, concatStr) < 0) {
                    perror("Failed to send game ID to server.");
                    return -1;
                }
                /* clear helper variable after use */
                memset(concatStr, 0, strlen(concatStr));
            }

        } else if(stringCompare(buffer, "+ PLAYING")) {

            /* store, check & print gamekind */
            if(sscanf(buffer, "%*s%*s%s", gameDataPointer -> gameKindName) != 1) {
                perror("Could not store game data.");
                return -1;
            } else if(!stringCompare(gameDataPointer -> gameKindName, "Checkers")) {
                perror("Gamekind must be \"Checkers\".");
                return -1;
            } else {
                printf("Playing \"%s\".\n", gameDataPointer -> gameKindName);
            }

            /* read in the game name; note that this is no separate 
            else-if case bc the response is of form "+ <<Game-Name>>"
            which is difficult to parse */
            if(receiveLineFromServer(sockfd, buffer, BUFFER_SIZE) < 0) {
                perror("Failed to receive line from server.");
                return -1;
            }

            /* store & print game name */
            if(sscanf(buffer, "%*s %[^\n]", gameDataPointer -> gameName) != 1) {
                perror("Could not store game data.");
                return -1;
            } else {
                printf("Joining game \"%s\".\n", gameDataPointer -> gameName);
            }

            /* determine desired player number */
            if (gameDataPointer -> desPlayerNumber == -1) {
                desPlayerNumberAsStr[0] = '\0';  /* "empty" string */
            } else {
                sprintf(desPlayerNumberAsStr, "%i", gameDataPointer -> desPlayerNumber); 
            }

            /* generate response regarding desired player number */
            if (stringConcat("PLAYER ", desPlayerNumberAsStr, concatStr) < 0) {
                perror("Could not generate response regarding desired player number.");
                return -1;
            } else {
                /* send desired player number */
                if(sendLineToServer(sockfd, buffer, concatStr) < 0) {
                    perror("Failed to send desired player number to server.");
                    return -1;
                }
                /* clear helper variable */
                memset(concatStr, 0, strlen(concatStr));
            }

        } else if(stringCompare(buffer, "+ YOU")) {

            /* store & print player number and player name */
            if(sscanf(buffer, "%*s %*s %i %[^\n]", &(gameDataPointer -> thisPlayerNumber), gameDataPointer -> thisPlayerName) != 2) {
                perror("Could not store game data.");
                return -1;
            } else {
                printf("You are Player %i (%s).\n", gameDataPointer -> thisPlayerNumber, gameDataPointer -> thisPlayerName);
            }
        
        } else if(stringCompare(buffer, "+ TOTAL")) {

            /* store & print total player count */
            if(sscanf(buffer, "%*s %*s %i", &(gameDataPointer -> playerCount)) != 1) {
                perror("Could not store game data.");
                return -1;
            } else {
                printf("%i players are playing.\n", gameDataPointer -> playerCount);
            }

            /* read in the opponents player number, name and whether they
            are ready; note that this is no separate else-if case bc the 
            response is of form "+ <<Player-Number>> <<Player-Name>> <<Ready>>"
            which is difficult to parse */
            if(receiveLineFromServer(sockfd, buffer, BUFFER_SIZE) < 0) {
                perror("Failed to receive line from server.");
                return -1;
            }

            /* pointer to the first occurence of space character (+2 bc of preceding '+ ') */
            if((pstart = strchr(buffer + 2, ' ')) == NULL) {
                perror("strchr: Matching character not found.");
                return -1;
            }

            /* infer the index of the space before the player name starts */
            istart = pstart - buffer;
            
            /* pointer to the last occurence of space character */
            if((pend = strrchr(buffer, ' ')) == NULL) {
                perror("strrchr: Matching character not found.");
                return -1;
            }

            /* infer the index of the space after the player name ends */
            iend = pend - buffer;

            /* store the name of the opponent (+1 to skip preceding space) */
            j = 0;
            for(int i = istart + 1; i < iend; i++) {
                (gameDataPointer -> oppPlayerName)[j] = buffer[i];
                j++;
            }

            /* add the null terminator at the end */
            (gameDataPointer -> oppPlayerName)[j] = '\0';

            /* store the player number of the opponent */
            if(sscanf(buffer + istart - 1, "%i", &(gameDataPointer -> oppPlayerNumber)) != 1) {
                perror("Could not store game data.");
                return -1;
            }

            /* store ready status of the opponent */
            if(sscanf(buffer + iend + 1, "%i", &(gameDataPointer -> oppPlayerReady)) != 1) {
                perror("Could not store game data.");
                return -1;
            }

            /* print opponents player number, name and whether they are ready */
            if(gameDataPointer -> oppPlayerReady == 1) {
                printf("Player %i (%s) is ready.\n", gameDataPointer -> oppPlayerNumber, gameDataPointer -> oppPlayerName);
            } else {
                printf("Player %i (%s) is not ready.\n", gameDataPointer -> oppPlayerNumber, gameDataPointer -> oppPlayerName);
            }
            
        } else if(stringCompare(buffer, "+ ENDPLAYERS")) {

            endOfPrologReached = true;
        
        } else {

            /* for unexpected things */
            perror("Something went wrong while setting up the game.");
            return -1;
        }
    }

    printf("Prologue phase successful. Starting the game now.\n");

    return 0;
}