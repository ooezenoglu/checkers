#ifndef helpers
#define helpers

struct gameInfo {
    char gameID[15];
    int desPlayerNumber;
    char configFile[40];
    char hostName[40];
    char gameKindName[20];
    uint16_t portNumber;
    char serverVersion[10];
    char clientVersion[10];
    int connectorPID;
    int thinkerPID;
    char gameName[40];
    int thisPlayerNumber;
    char thisPlayerName[20];
    int playerCount;
    int shmidOpponents;
};

struct player {
    int playerNumber;
    char playerName[20];
    int isReady;
};

int parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameDataPointer);
int readConfigFile(struct gameInfo *gameDataPointer);
int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, struct gameInfo *gameDataPointer);
int receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
int sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
int stringConcat(const char *leftString, const char *rightString, char *dest);
int stringTokenizer(char *src, char *delim, char **res, int *len);
void printWaitDetails(int wstatus);

#endif