#ifndef helpers
#define helpers

void parse_commandline_args(int argc, char *argv[], char **gameID, int *playerCount);
void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
void sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
char *stringConcat(const char *leftString, const char *rightString);

#endif