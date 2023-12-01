#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "helpers.h"

#define BUFFER_SIZE 255

int performConnection(const int sockfd, struct gameInfo *gameDataPointer) {

    char buffer[BUFFER_SIZE];
    bool endOfPrologReached = false;

    while(!endOfPrologReached) {

        receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        if (buffer[0] == '-') {

            printf("NACK received; Disconecting from server...");
            return -1; 

        } else if(stringCompare(buffer, "+ MNM Gameserver")) {

            /* TODO possibly extract and store gameserver version*/

            /* note: client major version must match game server major version */
            sendLineToServer(sockfd, buffer, stringConcat("VERSION ", gameDataPointer -> clientVersion));

        } else if(stringCompare(buffer, "+ Client version accepted - please send Game-ID to join")) {

            char* gameLineID = stringConcat("ID ", gameDataPointer -> gameID);
            sendLineToServer(sockfd, buffer, gameLineID);
            free(gameLineID);

        } else if(stringCompare(buffer, "+ PLAYING")) {

            /* TODO extract and store gamekind name */

            /* read in the game name; note that this is no separate 
            else-if case bc the response is of form "+ <<Game-Name>>"
            which is difficult to parse */
            receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

            /* TODO possibly extract and store game name */

            /* note: don't pass a player count */
            sendLineToServer(sockfd, buffer, stringConcat("PLAYER ", gameDataPointer -> desPlayerNumber));

        } else if(stringCompare(buffer, "+ YOU")) {

            /* TODO extract and store player number and name */
            
        } else if(stringCompare(buffer, "+ TOTAL")) {

            /* TODO extract and store player count */

            /* read in the player number and name of second player; 
            note that this is no separate else-if case bc the response 
            is of form "+ <<Player-Number>> <<Player-Name>> <<Ready>>"
            which is difficult to parse */
            receiveLineFromServer(sockfd, buffer, BUFFER_SIZE);

        } else if(stringCompare(buffer, "+ ENDPLAYERS")) {

            endOfPrologReached = true;
        
        } else {

            /* for unexpected things */
            printf("Something went wrong while setting up the game");
            return -1;
        }
    }

    return 1;
    
}