#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

struct stat file_info;

int head(char *file_name, int accepted_socket) {
    int file_fd = open(file_name, O_RDONLY);
    /*
    if (errno == EACCES) 
    {
        write(accepted_socket, "HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nForbidden\n", 47);
        return 403;
    }
    */
    if (file_fd == -1) {
        write(accepted_socket, "HTTP/1.1 404\r\nContent-Length: 10\r\n\r\nNot Found\n", 47);
        return 404;
    }
    int status = fstat(file_fd, &file_info);
    if (status == -1) {
        write(accepted_socket, "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n",
            59);
        return 500;
    }
    write(accepted_socket, "HTTP/1.1 200 OK\r\nContent-Length: ", 33);
    int buf = file_info.st_size;
    int size = 0;
    while (buf != 0) {
        buf /= 10;
        size++;
    }
    char *file_byte_len = (char *) malloc(size + 1 * sizeof(char));
    file_byte_len[size + 1] = '\0';
    sprintf(file_byte_len, "%ld", file_info.st_size);
    write(accepted_socket, file_byte_len, size);
    write(accepted_socket, "\r\n\r\n", 5);
    free(file_byte_len);
    return 200;
}
