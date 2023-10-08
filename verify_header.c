#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <regex.h>
#include <math.h>
#include <stdint.h>

bool check_header(char **input, int *content_length, int *request_id) {
    regex_t header_regex;
    regex_t content_len_regex;
    regex_t request_id_regex;
    int errors_space = 0;
    for (int i = 0; (*input)[i] != '\0'; i++) {
        if ((*input)[i] == ' ') {
            errors_space++;
        }
    }
    if (errors_space > 2) {
        return false;
    }
    char *header_format = "^[A-Za-z0-9-]+: [.*A-Za-z0-9]"; //true
    char *CL_format = "[C|c]ontent-[L|l]ength: [0-9]+";
    char *RquestID_format = "[R|r]equst-[I|i]d: [0-9]+";
    int rc;
    size_t nmatch = 2;
    regmatch_t pmatch[2];
    //Compile the regex
    if (0 != (rc = regcomp(&header_regex, header_format, REG_EXTENDED))) {
        //exit(EXIT_FAILURE);
        return false;
    }
    if (0 != (rc = regcomp(&content_len_regex, CL_format, REG_EXTENDED))) {
        //exit(EXIT_FAILURE);
        return false;
    }
    if (0 != (rc = regcomp(&request_id_regex, RquestID_format, REG_EXTENDED))) {
        //exit(EXIT_FAILURE);
        return false;
    }

    /*
        Check if good format
    */
    if (0 != (rc = regexec(&header_regex, *input, nmatch, pmatch, 0))) {
        return false;
    }
    //Check if it's the content length and get the value
    if (0 == (rc = regexec(&content_len_regex, *input, nmatch, pmatch, 0))) {
        char str_num[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        uint16_t int_nums[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int length = 0;
        for (int i = pmatch->rm_eo - 1; (*input)[i] != ' '; i--) {
            int find_int_num = 0;
            while ((*input)[i] != str_num[find_int_num]) {
                find_int_num++;
            }
            int base_num = pow(10, length);
            length++;
            *content_length += (base_num * int_nums[find_int_num]);
        }
        return true;
    }

    //Check if it's the Request ID and retrieve it
    if (0 == (rc = regexec(&request_id_regex, *input, nmatch, pmatch, 0))) {
        char str_num[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        uint16_t int_nums[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int length = 0;
        for (int i = pmatch->rm_eo - 1; (*input)[i] != ' '; i--) {
            int find_int_num = 0;
            while ((*input)[i] != str_num[find_int_num]) {
                find_int_num++;
            }
            int base_num = pow(10, length);
            length++;
            *request_id += (base_num * int_nums[find_int_num]);
        }
        return true;
    }
    return true;
}
