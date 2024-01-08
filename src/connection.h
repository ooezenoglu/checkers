#ifndef connection
#define connection

#include "helpers.h"
#include "stringUtils.h"
#include "sharedMemory.h"

#define MNM_GAMESERVER "+ MNM Gameserver"
#define VERSION "VERSION "
#define CLIENT_ACCEPTED "+ Client version accepted"
#define ID "ID "
#define PLAYING "+ PLAYING"
#define PLAYER "PLAYER"
#define YOU "+ YOU"
#define TOTAL "+ TOTAL"
#define ENDPLAYERS "+ ENDPLAYERS"

void connectToServer();
void receiveLineFromServer(char *buffer);
void sendLineToServer(const char *line);
void performConnection();

#endif