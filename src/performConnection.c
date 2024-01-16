#include "connection.h"

void performConnection() {

    char buffer[BUFFER_SIZE] = { 0 };
    char concatStr[BUFFER_SIZE] = { 0 };
    char desPlayerNumberAsStr[12];

    while(1) {

        receiveLineFromServer(buffer);

        if (startsWith(buffer, NACK)) {
            
            stringConcat("NACK received: ", buffer + 2, NULL, concatStr); /* + 2 to skip preceding "- " */

            /* remove newline at the end of the string bc perror adds more stuff to it */
            concatStr[strlen(concatStr)-1] = '\0';

            errNdie(concatStr);

        } else if(startsWith(buffer, MNM_GAMESERVER)) {

            /* store & print gameserver version */
            if(sscanf(buffer, "%*s%*s%*s%s", gameInfo -> serverVersion) != 1) { 
                errNdie("Could not store game data.");
            } else {
                printf("Connecting with the MNM Gameserver %s...\n", gameInfo -> serverVersion);
            }

            /* read & print message of the day */
            receiveLineFromServer(buffer);
            printf("The message of the day is: %s", buffer+2);

            /* generate & send client version response */
            stringConcat(VERSION, gameInfo -> clientVersion, NULL, concatStr);

            /* send client version (note that it must match the game server major version) */
            sendLineToServer(concatStr);
            
            /* clear helper variable after use */
            memset(concatStr, 0, strlen(concatStr));

        } else if(startsWith(buffer, CLIENT_ACCEPTED)) {

            printf("MNM Gameserver accepted client version %s.\n", gameInfo -> clientVersion); 

            /* genererate & send game ID response */
            stringConcat(ID, gameInfo -> gameID, NULL, concatStr);

            /* send game ID */
            sendLineToServer(concatStr);

            /* clear helper variable after use */
            memset(concatStr, 0, strlen(concatStr));

        } else if(startsWith(buffer, PLAYING)) {

            /* store, check & print gamekind */
            if(sscanf(buffer, "%*s%*s%s", gameInfo -> gameKindName) != 1) {
                errNdie("Could not store game data.");
            } else if(!startsWith(gameInfo -> gameKindName, "Checkers")) {
                errNdie("Gamekind must be \"Checkers\".");
            } else {
                printf("Playing \"%s\".\n", gameInfo -> gameKindName);
            }

            /* read in the game name; note that this is no separate 
            else-if case bc the response is of form "+ <<Game-Name>>"
            which is difficult to parse */
            receiveLineFromServer(buffer);

            /* store & print game name */
            if(sscanf(buffer, "%*s %[^\n]", gameInfo -> gameName) != 1) {
                errNdie("Could not store game data.");
            } else {
                printf("Joining game \"%s\".\n", gameInfo -> gameName);
            }

            /* determine requested player number */
            if (gameInfo -> requestedPlayerNumber == -1) {
                desPlayerNumberAsStr[0] = '\0';  /* "empty" string */
            } else {
                sprintf(desPlayerNumberAsStr, "%i", gameInfo -> requestedPlayerNumber); 
            }

            /* generate response regarding requested player number */
            stringConcat(PLAYER, desPlayerNumberAsStr, NULL, concatStr);

            /* send desired player number */
            sendLineToServer(concatStr);

            /* clear helper variable */
            memset(concatStr, 0, strlen(concatStr));

        } else if(startsWith(buffer, YOU)) {

            /* store & print player number and player name */
            if(sscanf(buffer, "%*s %*s %i %[^\n]", &(gameInfo -> thisPlayerNumber), gameInfo -> thisPlayerName) != 2) {
                errNdie("Could not store game data.");
            } else {
                printf("You are Player %i (%s).\n", gameInfo -> thisPlayerNumber, gameInfo -> thisPlayerName);
            }
        
        } else if(startsWith(buffer, TOTAL)) {

            /* store & print total player count */
            if(sscanf(buffer, "%*s %*s %i", &(gameInfo -> nPlayers)) != 1) {
                errNdie("Could not store game data.");
            } else {
                printf("%i players are playing.\n", gameInfo -> nPlayers);
            }

            /* create a shared memory segment for the opponents */
            gameInfo -> shmidOpponents = SHMAlloc(gameInfo -> nPlayers*sizeof(struct player));

            /* attach opponent info to Connector process */
            oppInfo = (struct player*) SHMAttach(gameInfo -> shmidOpponents);
            SHMInfo.connectorAttachedOppInfo = true;

            /* store the opponent data */
            for(int i = 0; i < gameInfo -> nPlayers-1; i++) {

                /* read in the opponents player number, name and whether they
                are ready; note that this is no separate else-if case bc the 
                response is of form "+ <<Player-Number>> <<Player-Name>> <<Ready>>"
                which is difficult to parse */
                receiveLineFromServer(buffer);

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

            /* send a signal to the Thinker that the opponent info is now attachable */
            kill(gameInfo -> thinkerPID, SIGUSR2);
                
        } else if(startsWith(buffer, ENDPLAYERS)) {
            
            break;

        } else {

            /* for unexpected things */
            errNdie("Something went wrong while setting up the game.");
        }
    }

    printf("Prologue phase successful. Starting the game now.\n");
}