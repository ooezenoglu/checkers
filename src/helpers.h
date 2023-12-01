#ifndef helpers
#define helpers

struct gameInfo {
    char *serverVersion;
    char *clientVersion;
    char *gameID;
    char *gamekindName;
    char *gameName;
    int desPlayerNumber;
    int thisPlayerNumber;
    char *thisPlayerName;
    int playerCount;
    int otherPlayerNumber;
    char *otherPlayerName;
    bool otherPlayerReady;
};

void parseCommandLineArgs(int argc, char *argv[], struct gameInfo *gameDataPointer);
int connectToServer(const char *host, const int port);
int performConnection(const int sockfd, struct gameInfo *gameDataPointer);
void receiveLineFromServer(const int sockfd, char *buffer, const int bufferSize);
void sendLineToServer(const int sockfd, char *buffer, const char *line);
bool stringCompare(const char *s1, const char *s2);
char *stringConcat(const char *leftString, const char *rightString);

#endif