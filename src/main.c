#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "helpers.h"

#define GAMEKINDNAME "Checkers"
#define PORTNUMBER 1357
#define HOSTNAME "sysprak.priv.lab.nm.ifi.lmu.de"


int main(int argc, char *argv[]) {

    char *gameID;
    int playerCount, sockfd;
    struct addrinfo hints, *addrInfoList, *entry;
    struct sockaddr_in servAddr;
    struct in_addr *IPAddr;

    parse_commandline_args(argc, argv, gameID, &playerCount);

    /* create IP socket that uses TCP (i.e., stream socket) */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    
    /* zero out bytes of hints sruct */
    memset(&hints, 0, sizeof(hints));

    /* specify criteria for relevant entries */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    /* determine server information (concretely the IP address) */
    if(getaddrinfo(HOSTNAME, NULL, &hints, &addrInfoList) != 0) {
        perror("Failed to get host address information");
        exit(EXIT_FAILURE);
    }

    /* for each entry that getaddrinfo found, */
    for(entry = addrInfoList; entry != NULL; entry = entry -> ai_next) {
      
        IPAddr = &((struct sockaddr_in *) entry -> ai_addr) -> sin_addr;

        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(PORTNUMBER);        
        servAddr.sin_addr = *IPAddr;

        /* connect to server */
        if(connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) == 0) {
            printf("Successfully connected to %s!\n", inet_ntoa(servAddr.sin_addr));

            /* TODO: call performConnection() */

            break;

        } else {
            printf("Not able to connect to %s!\n", inet_ntoa(servAddr.sin_addr));
            printf("Trying to connect via different IP...\n");
            continue;
        }
    }
    
    /* clean-up */
    freeaddrinfo(addrInfoList);
    close(sockfd);
    exit(EXIT_SUCCESS);
}