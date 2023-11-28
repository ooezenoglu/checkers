#ifndef prolog
#define prolog

int connectToServer(const char *host, const int port);
int performConnection(int sockfd, const char *gameID);

#endif