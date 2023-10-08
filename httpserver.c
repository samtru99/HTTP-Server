#include "bind.h"
#include "extract_header.h"
#include "request_regex.h"
#include "put.h"
#include "get.h"
#include "head.h"
#include "audit.h"
#include "verify_header.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <regex.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
//uint16_t range: 0 to 65535: Update need to have it check between 1025 to 65535
uint16_t convert(char *value, int length) {
    char str_num[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    uint16_t int_nums[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint16_t new_val = 0;
    for (int i = 0; i < length; i++) {
        int find_int_num = 0;
        while (value[i] != str_num[find_int_num]) {
            find_int_num++;
        }
        int multiplier_ct = i;
        uint16_t base_num = 1;
        while (multiplier_ct != length - 1) {
            base_num *= 10;
            multiplier_ct++;
        }
        new_val += (base_num * int_nums[find_int_num]);
    }
    return new_val;
}

int main(int argc, char *argv[]) {

    /*
    Create the signal handler stuff 
    */
    //Check for right amount of arguments
    /*
    if (argc != 2) {
        errx(1, "wrong arguments: ./httpserver port_num\nusage: ./httpserver <port>");
        return 0;
    }
    */
    /*
        PARSE THROUGH COMMAND LINE AND COLLECT INFO
    */
    int audit_fd = 0;
    int threads = 4;
    int options;
    bool close_audit = false;
    while ((options = getopt(argc, argv, "t:l:")) != -1) {
        switch (options) {
        case 't': threads = atoi(optarg); break;
        case 'l':
            audit_fd = open(optarg, O_WRONLY | O_TRUNC);
            if (audit_fd == -1) {
                close_audit = true;
                audit_fd = open(optarg, O_WRONLY | O_CREAT, 0700);
            }
            break;
        default:
            errx(1, "invalid argument: ./httpserver [-t threads] [-l logfile] port_num\nusage: "
                    "./httpserver <port>");
        }
    }
    if (strlen(argv[argc - 1]) > 5) {
        errx(1, "invalid argument: ./httpserver port_num\nusage: ./httpserver <port>");
        return 0;
    }

    /*
        Check if port contains any in correct characters 
    */

    bool valid = true;
    int str_len = 0;
    for (int i = 0; argv[argc - 1][i] != '\0'; i++) {
        if (argv[argc - 1][i] < 48 || argv[argc - 1][i] > 57) {
            valid = false;
        }
        str_len++;
    }

    //return 0;
    if (valid == false) {
        errx(1, "invalid port number: %s ", argv[argc - 1]);
    }
    //Convert port number and check for success
    uint16_t converted_port_num = convert(argv[argc - 1], str_len);
    if (converted_port_num < 1025) {
        errx(1, "invalid port number: %s ", argv[1]);
    }
    //Create socketfd and check for success

    int socketfd = create_listen_socket(converted_port_num);
    if (socketfd < 0) {
        errx(1, "invalid socket number: %s ", argv[1]);
    }

    char buffer[2050] = { 0 };
    int status_code = 0;
    int request_id = 0;
    char *operation;
    char *file_name;
    int content_length = 0;
    while (true) //could be while not ctrl - c
    {
        //Accept the new socket and check for success
        int accept_socketfd = accept(socketfd, NULL, NULL);
        if (accept_socketfd == -1) {
            write(accept_socketfd,
                "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
        }

        bool needMsgBody = false;
        //wait for input
        int num_of_bytes_read = read(accept_socketfd, buffer, 2050);
        if (num_of_bytes_read == 0) {
            write(accept_socketfd, "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
            int close_AS = close(accept_socketfd);
            if (close_AS == -1) {
                write(accept_socketfd,
                    "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
            }
            continue;
        }
        int start = 0;
        int end = 0;
        int saved_Pos_for_Msg = 0;
        bool request_info = false;
        bool skip_operation = false;
        bool dont_free = false;
        if ((buffer[0]) < 65 || (buffer[0] > 90)) {
            write(accept_socketfd, "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
            int close_AS = close(accept_socketfd);
            if (close_AS == -1) {
                write(accept_socketfd,
                    "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
            }
            continue;
        }

        for (int i = 0; i < (num_of_bytes_read); i++) //break once you see \r\n\r\n
        {
            if (((buffer[i] == 13) && (buffer[i + 1] == '\n') && (buffer[i + 2] == 13)
                    && (buffer[i + 3] == '\n'))) //where you find \r\n - end of request body
            {
                if (request_info == false) {
                    request_info = true;
                    end = i + 1;
                    char *request_line_buffer;
                    request_line_buffer = (char *) malloc(((i - start) + 1) * sizeof(char));
                    extraction(buffer, &request_line_buffer, start, end);
                    request_line_buffer[((i - start))] = '\0';
                    int valid_request
                        = request_file_regex(request_line_buffer, &file_name, &operation);
                    if (valid_request == 0) {
                        write(accept_socketfd,
                            "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                        skip_operation = true;
                        free(request_line_buffer);
                        break;
                    }
                    if (valid_request == 2) {

                        write(accept_socketfd,
                            "HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nForbidden\n", 47);
                        skip_operation = true;
                        free(request_line_buffer);
                        dont_free = true;
                        break;
                    }
                    break;
                }
                char *next_header_line;
                next_header_line = (char *) malloc(((i + 1) - start) * sizeof(char)); //i+2?
                extraction(buffer, &next_header_line, start, i);
                bool header_valid = check_header(&next_header_line, &content_length, &request_id);
                if (header_valid == false) {
                    write(accept_socketfd,
                        "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                    skip_operation = true;
                    free(next_header_line);
                    break;
                }
                free(next_header_line);
                start = end + 1;
                if (i == num_of_bytes_read - 4) {
                    needMsgBody = true;
                } else {
                    saved_Pos_for_Msg = i + 4;
                }
                break;
            } else if ((buffer[i] == 13 && buffer[i + 1] == '\n')) //finding each section
            {
                if (request_info == false) {
                    request_info = true;
                    end = i + 1;
                    char *request_line_buffer;
                    request_line_buffer = (char *) malloc(((i - start) + 1) * sizeof(char));
                    extraction(buffer, &request_line_buffer, start, end);
                    request_line_buffer[((i - start))] = '\0';
                    int valid_request
                        = request_file_regex(request_line_buffer, &file_name, &operation);
                    if (valid_request == 0) {
                        write(accept_socketfd,
                            "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                        skip_operation = true;
                        free(request_line_buffer);
                        break;
                    }
                    if (valid_request == 2) {
                        write(accept_socketfd,
                            "HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nForbidden\n", 47);
                        skip_operation = true;
                        free(request_line_buffer);
                        dont_free = true;
                        continue;
                    }
                    start = end + 1;
                    free(request_line_buffer);
                } else {
                    char *next_header_line;
                    next_header_line = (char *) malloc(
                        (((i + 1) - start) + 1) * sizeof(char)); //i+2? should go into extract
                    end = i + 1;
                    extraction(buffer, &next_header_line, start, end); //org was start - 1
                    bool header_valid;
                    header_valid = check_header(&next_header_line, &content_length, &request_id);
                    if (header_valid == false) {
                        write(accept_socketfd,
                            "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                        skip_operation = true;
                        free(next_header_line);
                        break;
                    }
                    start = end + 1;
                    free(next_header_line);
                }
            }
        }
        if (skip_operation == false) {
            if (strcmp(operation, "PUT") == 0) //PUT Operation
            {
                if (content_length == -1) {
                    write(accept_socketfd,
                        "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                } else {
                    status_code = put_op(buffer, file_name, saved_Pos_for_Msg, content_length,
                        needMsgBody, accept_socketfd, num_of_bytes_read);
                    if (status_code == 200) {
                        write(accept_socketfd, "HTTP/1.1 200\r\nContent-Length: 2\r\n\r\nOK\n", 38);
                    } else {
                        write(
                            accept_socketfd, "HTTP/1.1 201\r\nContent-Length: 7\r\nCreated\n", 41);
                    }
                }

            } else if (strcmp(operation, "GET") == 0) {
                status_code = get(file_name, accept_socketfd);
            } else if (strcmp(operation, "HEAD") == 0) {
                status_code = head(file_name, accept_socketfd);
                //do we still perform the audit on bad files?
            } else {
                write(
                    accept_socketfd, "HTTP/1.1 501\r\nContent-Length: 16\r\nNot Implemented\n", 41);
            }
        }

        // CLEAR AND CLOSE ALL MEMORY
        audit_log(operation, status_code, &file_name, audit_fd, request_id);

        // CLEAR AND CLOSE ALL MEMORY
        int close_AS = close(accept_socketfd);
        if (close_AS == -1) {
            write(accept_socketfd,
                "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
        }
        content_length = 0;
        if (dont_free == false) {
            free(file_name);
        }
    }
    if (close_audit == true) {
        close(audit_fd);
    }
    return 0;
}
