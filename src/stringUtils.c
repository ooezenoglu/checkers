#include "stringUtils.h"

bool startsWith(const char *s1, const char *s2) {

    int n = (strlen(s1) <= strlen(s2)) ? strlen(s1) : strlen(s2);

    return strncmp(s1, s2, n) == 0;
}

bool stringEquals(const char *s1, const char *s2) {

    int s1Length = strlen(s1);
    int s2Length = strlen(s2);

    if(s1Length != s2Length) {
        return false;
    }

    return strncmp(s1, s2, s1Length) == 0;
}

void stringConcat(const char *leftString, const char *rightString, char *delim, char *dest) {

    if(leftString == NULL && rightString != NULL) {
        /* copy the right string + /0 to the destination */
        if(memcpy(dest, rightString, strlen(rightString) + 1) == NULL) {
            errNdie("Failed to copy string.");
        }
    }

    if(leftString != NULL && rightString == NULL) { 
        /* copy the left string + /0 to the destination */
        if(memcpy(dest, leftString, strlen(leftString) + 1) == NULL) {
            errNdie("Failed to copy string.");
        };
    }

    if(leftString == NULL && rightString == NULL) {
        errNdie("Both string arguments are NULL.");
    }

    /* copy the left string to the destination */
    if(memcpy(dest, leftString, strlen(leftString)) == NULL) {
        errNdie("Failed to copy string.");
    }

    if(delim != NULL) {
        /* concatenate the delimiter to the destination */
        if(strncat(dest, delim, strlen(delim)) == NULL) {
            errNdie("Failed to concatenate strings.");
        }
    }

    /* concatenate the right string and its null terminator to the destination */
    if(strncat(dest, rightString, strlen(rightString) + 1) == NULL) {
        errNdie("Failed to concatenate strings.");
    }
}

void stringTokenizer(char *src, char *delim, char **res, int *len, int maxTokens) {

    /* initialize result */
    for (int i = 0; i < maxTokens; i++) {
        res[i] = NULL;
    }

    char *token;
    int i = 0;

    token = strtok(src, delim);

    while (token != NULL) {
        res[i] = token;
        i++;
        token = strtok(NULL, delim);
    }

    *len = i;

    /* debugging */
    // for(int j = 0; j < *len; j++) {
    //     printf("TOKEN: %s\n", res[j]);
    // }
}