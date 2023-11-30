#ifndef prolog
#define prolog

int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, const char *gameID);

#endif