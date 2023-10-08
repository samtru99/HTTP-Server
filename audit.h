#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>

void audit_log(char *op, int status_code, char **file_name, int fd, int R_ID);
