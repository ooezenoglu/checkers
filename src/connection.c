#include "connection.h"

void connectToServer() {

    struct addrinfo hints, *addrInfoList;
    struct sockaddr_in servAddr;
    struct epoll_event sockEV;
    
    /* zero out bytes of hints sruct */
    memset(&hints, 0, sizeof(hints));

    /* specify criteria for relevant entries */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    /* determine server IP address */
    if(getaddrinfo(gameInfo -> hostName, NULL, &hints, &addrInfoList) != 0) {
        errNdie("Failed to get host address information.");
    }

    /* store server info */
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(gameInfo -> port);
    servAddr.sin_addr = *(&((struct sockaddr_in *) addrInfoList -> ai_addr) -> sin_addr); /* grab the first IP address from the list */

    freeaddrinfo(addrInfoList);

    /* create TCP/IP socket */
    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        errNdie("Failed to create socket.");
    }

    /* connect to server */
    if(connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) != 0) {
        errNdie("Failed to establish connection with the server.\n");
    }

    /* monitor incoming data in the socket */
    sockEV.events = EPOLLIN;
    sockEV.data.fd = sockfd;

    /* add socket file descriptor to the interest list of the epoll instance */
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &sockEV) != 0) {
        errNdie("Failed to add the socket file descriptor to the interest list.");
    }

    /* debugging */
    // printf("Successfully established connection with %s via %s\n", host, inet_ntoa(servAddr.sin_addr));
}

void receiveLineFromServer(char *buffer) {

    /* clear buffer before use */
    memset(buffer, 0, strlen(buffer));

    /* read-in data until the first newline character is reached */
    for(int i = 0; i < BUFFER_SIZE; i++) {

        if(recv(sockfd, &buffer[i], 1, 0) != 1) {
            errNdie("Failed to receive line from server.");
        }

        if (buffer[i] == '\n') {
            buffer[i+1] = '\0';
            break;
        }
    }

    /* debugging */
    // printf("S: %s\n", buffer);
}

void sendLineToServer(const char *line) {

    char buffer[BUFFER_SIZE] = { 0 };

    /* write into the buffer and finish with newline character */
    if(sprintf(&buffer[0], "%s\n", line) < 0) {
        errNdie("Failed to write line to buffer.");
    }

    if(send(sockfd, buffer, strlen(buffer), 0) == -1) {
        errNdie("Failed to send line to server.");
    }

    /* debugging */
    // printf("C: %s\n", buffer);
}