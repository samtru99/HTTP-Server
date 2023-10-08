#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int put_op(char buffer[2050], char *file_name, int savedMsgBody, int content_length,
    bool needMsgBody, int accept_socketfd, int num_of_bytes_read);
