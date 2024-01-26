#ifndef stringUtils
#define stringUtils

#include "helpers.h"

bool startsWith(const char *s1, const char *s2);
bool stringEquals(const char *s1, const char *s2);
void stringConcat(const char *leftString, const char *rightString, char *delim, char *dest);
void stringTokenizer(char *src, char *delim, char **res, int *len, int maxTokens);

#endif