#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "helpers.h"

int connectToServer(const char *host, const int port) {

    int sockfd;
    struct addrinfo hints, *addrInfoList;
    struct sockaddr_in servAddr;
    
    /* zero out bytes of hints sruct */
    memset(&hints, 0, sizeof(hints));

    /* specify criteria for relevant entries */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    /* determine server IP address */
    if(getaddrinfo(host, NULL, &hints, &addrInfoList) != 0) {
        perror("Failed to get host address information.");
        return -1;
    }

    /* store server info */
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr = *(&((struct sockaddr_in *) addrInfoList -> ai_addr) -> sin_addr); /* grab the first IP address from the list */

    freeaddrinfo(addrInfoList);

    /* create TCP/IP socket */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket.");
        return -1;
    }

     /* connect to server */
    if(connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) == 0) {
        /* debugging */
        // printf("Successfully established connection with %s via %s\n", host, inet_ntoa(servAddr.sin_addr));
        return sockfd;
    }
     
    perror("Failed to establish connection with the server. Closing socket...\n");
    close(sockfd);
    
    return -1;
}
