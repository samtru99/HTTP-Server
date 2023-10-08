#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <regex.h>

int request_file_regex(char *request, char **file_name, char **operation);
