#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <regex.h>
#include <math.h>

bool check_header(char **input, int *content_length, int *request_id);
