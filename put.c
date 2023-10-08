#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "extract_header.h"
#include <err.h>
#include <stdbool.h>
#include <stdio.h>

int put_op(char buffer[2050], char *file_name, int savedMsgPos, int content_length,
    bool needMsgBody, int accept_socketfd, int num_of_bytes_read) {
    int status_code = 200;
    int file_fd = open(file_name, O_WRONLY | O_TRUNC);
    if (file_fd == -1) {
        status_code = 201;
        file_fd = open(file_name, O_WRONLY | O_CREAT, 0700);
    }

    if (needMsgBody == false) {
        char *message = (char *) malloc((num_of_bytes_read - savedMsgPos) * sizeof(char));
        extraction(buffer, &message, savedMsgPos, num_of_bytes_read);
        //message[(num_of_bytes_read - savedMsgPos)] = '\0'; //og was + 1
        write(file_fd, message, (num_of_bytes_read - savedMsgPos));
        free(message);
        content_length -= ((num_of_bytes_read - savedMsgPos));
        if (content_length == 0) {
            int close_file_fd = close(file_fd);
            if (close_file_fd == -1) {
                write(accept_socketfd,
                    "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
                return 500;
            }
            return status_code;
        }
    }
    while (content_length != 0) {
        int bytes_read = read(accept_socketfd, buffer, 2050);
        write(file_fd, buffer, bytes_read);
        content_length -= bytes_read;
    }

    int close_file_fd = close(file_fd);
    if (close_file_fd == -1) {
        write(accept_socketfd, "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n",
            59);
        return 500;
    }

    return status_code;
}
