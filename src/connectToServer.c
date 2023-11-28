#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "prolog.h"

int connectToServer(const char *host, const int port) {

    int sockfd;
    struct addrinfo hints, *addrInfoList, *entry;
    struct sockaddr_in servAddr;
    struct in_addr *IPAddr;

    /* create TCP/IP socket */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed to create socket");
        return -1;
    }
    
    /* zero out bytes of hints sruct */
    memset(&hints, 0, sizeof(hints));

    /* specify criteria for relevant entries */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    /* determine server information (concretely the IP address) */
    if(getaddrinfo(host, NULL, &hints, &addrInfoList) != 0) {
        printf("Failed to get host address information");
        return -1;
    }

    /* connect to server using one of the IP-addresses from the list */
    for(entry = addrInfoList; entry != NULL; entry = entry -> ai_next) {
      
        IPAddr = &((struct sockaddr_in *) entry -> ai_addr) -> sin_addr;

        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(port);        
        servAddr.sin_addr = *IPAddr;

        /* connect to server */
        if(connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) == 0) {
            printf("Successfully established connection with %s via %s\n", host, inet_ntoa(servAddr.sin_addr));

            return sockfd;

        } else {
            printf("Failed to establish connection with %s via %s\n", host, inet_ntoa(servAddr.sin_addr));
            printf("Trying a different IP-address...\n");
            continue;
        }
    }
    
    printf("Failed to establish connection with %s\n", host);
    printf("Closing socket...\n");

    /* clean-up */
    freeaddrinfo(addrInfoList);
    close(sockfd);

    return -1;
}
