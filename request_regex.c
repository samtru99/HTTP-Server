#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <regex.h>

int request_file_regex(char *request, char **file_name, char **operation) {
    //Create all regex containers
    regex_t operation_regex;
    regex_t txt_file_regex;
    regex_t http_file_regex;
    //Create the regex
    char *operation_reg = "^((HEAD)|(PUT)|(GET))";
    char *txt_file = "([a-zA-Z]+\\.txt)";
    char *http_file = "HTTP/1.1";

    int nmatch = 2;
    regmatch_t file_name_arr[2];
    regmatch_t op_arr[2];
    regmatch_t http_arr[2];
    int rc;

    //Store the regex in their containers
    if (0 != (rc = regcomp(&operation_regex, operation_reg, REG_EXTENDED))) {
        return 0;
        //exit(EXIT_FAILURE);
    }
    if (0 != (rc = regcomp(&txt_file_regex, txt_file, REG_EXTENDED))) {
        return 0;
        //exit(EXIT_FAILURE);
    }
    if (0 != (rc = regcomp(&http_file_regex, http_file, REG_EXTENDED))) {
        return 0;
        //exit(EXIT_FAILURE);
    }
    //Check if regex exist in the input
    if (0 != (rc = regexec(&operation_regex, request, nmatch, op_arr, 0))) {
        return 0;
    }

    if (0 != (rc = regexec(&txt_file_regex, request, nmatch, file_name_arr, 0))) {
        return 2;
    }
    if (0 != (rc = regexec(&http_file_regex, request, nmatch, http_arr, 0))) {
        return 0;
    }

    if (request[0] == 'H') {
        *operation = (char *) malloc((4) * sizeof(char));
        (*operation) = "HEAD";
    } else {
        *operation = (char *) malloc((3) * sizeof(char));
        if (request[0] == 'P') {
            (*operation) = "PUT";
        } else {
            (*operation) = "GET";
        }
    }
    //this could cause issues
    *file_name = (char *) malloc(
        (file_name_arr->rm_eo - file_name_arr->rm_so)
        + 1 * sizeof(char)); //need to add terminatior to the name since it's a string
    int string_cout = 0;
    for (int i = file_name_arr->rm_so; i < file_name_arr->rm_eo; i++) {
        (*file_name)[string_cout++] = request[i];
    }
    //(*file_name)[string_cout] = '\0';
    return 1;
}
