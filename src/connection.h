#ifndef connection
#define connection

#include "helpers.h"
#include "stringUtils.h"
#include "sharedMemory.h"

int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, struct gameInfo *gameDataPointer);
void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
void sendLineToServer(const int sockfd, char *buffer, const char *line);

#endif