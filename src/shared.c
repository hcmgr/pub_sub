//shared.c//
//------------//
//This file contains some static methods that both the client and 
//server make use of
//------------//

#include "shared.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define EMPTY_STRING ""
#define INVALID_NUM -1

bool has_space_colon_newline(char* str) {
    //is the empty string
    if (!strcmp(str, EMPTY_STRING)) {
        return true;
    }
    while (str[0]) {
        //contains space, colon or newline
        if (str[0] == ' ' || str[0] == ':' || str[0] == '\n') {
            return true;
        }
        str++;
    }
    return false;
}

char* add_new_line(char* str) {
    char* buffer = malloc(sizeof(char) * (strlen(str) + 1));
    int i = 0;
    while (str[0]) {
        buffer[i++] = str[0];
        str++;
    }
    buffer[i] = '\n';
    return buffer;
}

int string_array_length(char** arr) {
    int count = 0;
    int i = 0;
    while (arr[i]) {
        i++;
        count++;
    }
    return count;
}

int string_to_int(char* str) {
    //handle portnum = 0 case
    if (str[0] == '0') {
        return 0;
    }
    //handle all other cases
    //checks all characters are digits
    for (int i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i])) {
            return INVALID_NUM;
        }
    }
    return atoi(str);
}

char** split_line_max(char** toks, int maxToks) { 
    //get combined length of extra strings
    int arrLen = string_array_length(toks);
    int combinedStrLen = 0;
    for (int i = maxToks - 1; i < arrLen; i++) {
        combinedStrLen += strlen(toks[i]);
    }
    //build up string made up of the extra strings
    char* combinedStr = calloc((combinedStrLen + 1), sizeof(char));
    for (int i = maxToks - 1; i < arrLen; i++) {
        strcat(combinedStr, strdup(toks[i]));
        if (i != arrLen - 1) {
            strcat(combinedStr, " ");
        }
    }
    //add initial tokens and combined str into new string array
    char** newToks = calloc(maxToks + 1, sizeof(char*));
    for (int i = 0; i < maxToks - 1; i++) {
        if (i < arrLen) {
            newToks[i] = strdup(toks[i]);
        }
    }
    if (maxToks - 1 < arrLen) {
        newToks[maxToks - 1] = combinedStr;
    }
    free(toks);
    return newToks;
}
