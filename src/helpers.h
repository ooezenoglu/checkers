#ifndef helpers
#define helpers

struct gameInfo {
    char serverVersion[10];
    char clientVersion[10];
    char gameID[15];
    char gamekindName[20];
    char gameName[40];
    int desPlayerNumber;
    int thisPlayerNumber;
    char thisPlayerName[20];
    int playerCount;
    int oppPlayerNumber;
    char oppPlayerName[20];
    int oppPlayerReady;
};

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameDataPointer);
int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, struct gameInfo *gameDataPointer);
int receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
int sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
int stringConcat(const char *leftString, const char *rightString, char *dest);
// char **stringTokenizer(char *s, int *len);

#endif