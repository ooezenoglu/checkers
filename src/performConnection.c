#include "helpers.h"

#define BUFFER_SIZE 255

int performConnection(const int sockfd, struct gameInfo *gameDataPointer) {

    char buffer[BUFFER_SIZE] = { 0 };
    char concatStr[BUFFER_SIZE] = { 0 };
    bool endOfPrologReached = false;
    char desPlayerNumberAsStr[12];
    struct player *oppInfo;

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

        } else if(startsWith(buffer, "+ MNM Gameserver")) {

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

        } else if(startsWith(buffer, "+ Client version accepted - please send Game-ID to join")) {

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

        } else if(startsWith(buffer, "+ PLAYING")) {

            /* store, check & print gamekind */
            if(sscanf(buffer, "%*s%*s%s", gameDataPointer -> gameKindName) != 1) {
                perror("Could not store game data.");
                return -1;
            } else if(!startsWith(gameDataPointer -> gameKindName, "Checkers")) {
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
            if (gameDataPointer -> requestedPlayerNumber == -1) {
                desPlayerNumberAsStr[0] = '\0';  /* "empty" string */
            } else {
                sprintf(desPlayerNumberAsStr, "%i", gameDataPointer -> requestedPlayerNumber); 
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

        } else if(startsWith(buffer, "+ YOU")) {

            /* store & print player number and player name */
            if(sscanf(buffer, "%*s %*s %i %[^\n]", &(gameDataPointer -> thisPlayerNumber), gameDataPointer -> thisPlayerName) != 2) {
                perror("Could not store game data.");
                return -1;
            } else {
                printf("You are Player %i (%s).\n", gameDataPointer -> thisPlayerNumber, gameDataPointer -> thisPlayerName);
            }
        
        } else if(startsWith(buffer, "+ TOTAL")) {

            /* store & print total player count */
            if(sscanf(buffer, "%*s %*s %i", &(gameDataPointer -> nPlayers)) != 1) {
                perror("Could not store game data.");
                return -1;
            } else {
                printf("%i players are playing.\n", gameDataPointer -> nPlayers);
            }

            /* create a shared memory segment for the opponents */
            gameDataPointer -> shmidOpponents = SHMAlloc(gameDataPointer -> nPlayers*sizeof(struct player));

            /* attach opponent info to Connector process */
            oppInfo = (struct player*) SHMAttach(gameDataPointer -> shmidOpponents);

            /* store the opponent data */
            for(int i = 0; i < gameDataPointer -> nPlayers-1; i++) {

                /* read in the opponents player number, name and whether they
                are ready; note that this is no separate else-if case bc the 
                response is of form "+ <<Player-Number>> <<Player-Name>> <<Ready>>"
                which is difficult to parse */
                if(receiveLineFromServer(sockfd, buffer, BUFFER_SIZE) < 0) {
                    perror("Failed to receive line from server.");
                    return -1;
                }

                char temp[BUFFER_SIZE];

                /* extract player number and store remaining info in temp */
                sscanf(buffer, "+ %i %[^\n]", &(oppInfo[i].playerNumber), temp);
                
                oppInfo[i].isReady = temp[strlen(temp)-1] - '0';

                memcpy(oppInfo[i].playerName, temp, strlen(temp) - 2); /* - 2 to not copy space and player number */

                 /* print opponents player number, name and whether they are ready */
                if(oppInfo[i].isReady == 1) {
                    printf("Player %i (%s) is ready.\n", oppInfo[i].playerNumber, oppInfo[i].playerName);
                } else {
                    printf("Player %i (%s) is not ready.\n", oppInfo[i].playerNumber, oppInfo[i].playerName);
                }
            }
                
        } else if(startsWith(buffer, "+ ENDPLAYERS")) {

            endOfPrologReached = true;
        
        } else {

            /* for unexpected things */
            perror("Something went wrong while setting up the game.");
            return -1;
        }
    }

    SHMDetach(oppInfo);

    printf("Prologue phase successful. Starting the game now.\n");

    return 0;
}