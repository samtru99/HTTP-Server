#include <stdlib.h>
#include <string.h>
#include <stdio.h>
void extraction(char buffer[2050], char **string, int start, int end) {
    int request_count = 0;
    for (int getInfo = start; getInfo != (end); getInfo++) {
        (*string)[request_count++] = buffer[getInfo];
    }
    //(*string)[request_count] = '\0';
}
