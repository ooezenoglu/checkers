#ifndef helpers
#define helpers

void parseCommandLineArgs(int argc, char *argv[], char **gameID, int *playerCount);
int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, const char *gameID);
void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
void sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
char *stringConcat(const char *leftString, const char *rightString);

#endif