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

void get(char *file_name, int accepted_socket) {
    int file_fd = open(file_name, O_RDONLY);
    if (errno == EACCES) {
        write(accepted_socket, "HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nForbidden\n", 47);
        return;
    }
    if (file_fd == -1) {
        write(accepted_socket, "HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nNot Found\n", 47);
        return;
    }

    int status = fstat(file_fd, &file_info);
    if (status == -1) {
        write(accepted_socket, "HTTP/1.1 403\r\nContent-Length: 22\r\n\r\nInternal Server Error\n",
            59);
    }
    write(accepted_socket, "HTTP/1.1 200 OK\r\nContent-Length: ", 33); //blocked for many headers
    int buf = file_info.st_size;
    int size = 0;
    while (buf != 0) {
        buf /= 10;
        size++;
    }
    //convert
    char *file_byte_len = (char *) malloc(size + 1 * sizeof(char));
    file_byte_len[size + 1] = '\0';
    sprintf(file_byte_len, "%ld", file_info.st_size);
    write(accepted_socket, file_byte_len, size); //block for many headers
    write(accepted_socket, "\r\n\r\n", 4); //for many headers
    free(file_byte_len);
    char buffer[file_info.st_size];
    int bytes_read = read(file_fd, buffer, file_info.st_size);
    buffer[file_info.st_size + 1] = '\0';
    write(accepted_socket, buffer, bytes_read);

    write(accepted_socket, "\n", 2);
    int close_fd = close(file_fd);
    if (close_fd == -1) {
        write(accepted_socket, "HTTP/1.1 403\r\nContent-Length: 22\r\n\r\nInternal Server Error\n",
            59);
    }
}
