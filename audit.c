#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>

void audit_log(char *op, int status_code, char **file_name, int fd, int R_ID) {
    int op_len = 0;
    if (op[0] == 'G' || op[0] == 'P') {
        op_len = 3;
    } else {
        op_len = 4;
    }
    //Writing the operation
    write(fd, op, op_len);
    write(fd, ",/", 2);

    //writing the file name
    int len_of_file_name = 0;
    while ((*file_name)[len_of_file_name] != '\0') {
        len_of_file_name++;
    }
    write(fd, *file_name, len_of_file_name);
    write(fd, ",", 1);

    //writing the status code
    char *str_status_code = (char *) malloc(4 * sizeof(char));
    sprintf(str_status_code, "%d", status_code);

    write(fd, str_status_code, 3);
    free(str_status_code);

    write(fd, ",", 1);
    //writing the request ID
    int size = 1;
    while (R_ID > 10) {
        R_ID /= 10;
        size++;
    }
    char *str_R_ID = (char *) malloc((size + 1) * sizeof(char));
    sprintf(str_R_ID, "%d", R_ID);
    write(fd, str_R_ID, size);

    //Write the newline
    write(fd, "\n", 1);
    free(str_R_ID);
}
