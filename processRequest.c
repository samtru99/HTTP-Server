#include "bind.h"
#include "extract_header.h"
#include "request_regex.h"
#include "put.h"
#include "get.h"
#include "head.h"
#include "audit.h"
#include "verify_header.h"
#include "queue.h"
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
void process_request(int accept_socketfd, int audit_fd, int unique_id, queue_t *audit_queue)
{
    //Variables for the Request
    char buffer[2050] = { 0 };
    int status_code = 0;
    int request_id = 0;
    char *operation;
    char *file_name;
    int content_length = 0; 
    bool needMsgBody = false;
    
    //Variables for Processing the Request
    int start = 0;
    int end = 0;
    int saved_Pos_for_Msg = 0;
    bool request_info = false;
    bool skip_operation = false;
    bool dont_free = false;

    //Read in bytes from the socket
    int num_of_bytes_read = read(accept_socketfd, buffer, 2050);

    //Error Check
    if (num_of_bytes_read == 0) 
    {
        write(accept_socketfd, "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
        int close_AS = close(accept_socketfd);
        if (close_AS == -1) 
        {
            write(accept_socketfd, "HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
        }
        return;
    }
    if ((buffer[0]) < 65 || (buffer[0] > 90)) 
    {
        write(accept_socketfd, "HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
        int close_AS = close(accept_socketfd);
        if (close_AS == -1) 
        {
            write(accept_socketfd,"HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
        }
        return;
    }

    //Parse and Process the bytes
    for (int i = 0; i < (num_of_bytes_read); i++) //break once you see \r\n\r\n
    {
        if (((buffer[i] == 13) && (buffer[i + 1] == '\n') && (buffer[i + 2] == 13)
                && (buffer[i + 3] == '\n'))) //where you find \r\n - end of request body
        {
            if (request_info == false) 
            {
                request_info = true;
                end = i + 1;
                char *request_line_buffer;
                request_line_buffer = (char *) malloc(((i - start) + 1) * sizeof(char));
                extraction(buffer, &request_line_buffer, start, end);
                request_line_buffer[((i - start))] = '\0';
                int valid_request = request_file_regex(request_line_buffer, &file_name, &operation);
                if (valid_request == 0) 
                {
                    write(accept_socketfd,"HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                    skip_operation = true;
                    free(request_line_buffer);
                    break;
                }
                if (valid_request == 2) 
                {
                    write(accept_socketfd,"HTTP/1.1 403\r\nContent-Length: 10\r\n\r\nForbidden\n", 47);
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
            if (header_valid == false) 
            {
                write(accept_socketfd,"HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                skip_operation = true;
                free(next_header_line);
                break;
            }
            free(next_header_line);
            start = end + 1;
            if (i == num_of_bytes_read - 4) 
            {
                needMsgBody = true;
            } 
            else 
            {
                saved_Pos_for_Msg = i + 4;
            }
            break;
        } 
        else if ((buffer[i] == 13 && buffer[i + 1] == '\n')) //finding each section
        {
            if (request_info == false) 
            {
                request_info = true;
                end = i + 1;
                char *request_line_buffer;
                request_line_buffer = (char *) malloc(((i - start) + 1) * sizeof(char));
                extraction(buffer, &request_line_buffer, start, end);
                request_line_buffer[((i - start))] = '\0';
                int valid_request = request_file_regex(request_line_buffer, &file_name, &operation);
                if (valid_request == 0) 
                {
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
            } 
            else 
            {
                char *next_header_line;
                next_header_line = (char *) malloc(
                    (((i + 1) - start) + 1) * sizeof(char)); //i+2? should go into extract
                end = i + 1;
                extraction(buffer, &next_header_line, start, end); //org was start - 1
                bool header_valid;
                header_valid = check_header(&next_header_line, &content_length, &request_id);
                if (header_valid == false) 
                {
                    write(accept_socketfd,"HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
                    skip_operation = true;
                    free(next_header_line);
                    break;
                }
                start = end + 1;
                free(next_header_line);
            }
        }
    }
    /*
        Perform the operation
    */
    if (skip_operation == false) 
    {
        if (strcmp(operation, "PUT") == 0)
        {
            if (content_length == -1) 
            {
                write(accept_socketfd,"HTTP/1.1 400\r\nContent-Length: 12\r\n\r\nBad Request\n", 49);
            } 
            else 
            {
                status_code = put_op(buffer, file_name, saved_Pos_for_Msg, content_length,needMsgBody, accept_socketfd, num_of_bytes_read);
                if (status_code == 200) 
                {
                    write(accept_socketfd, "HTTP/1.1 200\r\nContent-Length: 2\r\n\r\nOK\n", 38);
                } 
                else 
                {
                    write(accept_socketfd, "HTTP/1.1 201\r\nContent-Length: 7\r\nCreated\n", 41);
                }
            }
        } 
        else if (strcmp(operation, "GET") == 0) 
        {
            status_code = get(file_name, accept_socketfd);
        } 
        else if (strcmp(operation, "HEAD") == 0) 
        {
            status_code = head(file_name, accept_socketfd);
            //do we still perform the audit on bad files?
        } 
        else 
        {
            write(accept_socketfd, "HTTP/1.1 501\r\nContent-Length: 16\r\nNot Implemented\n", 41);
        }
    }
    /*
        Record the outcome in the log file
    */

    int *queue_top_val;
    while(unique_id != queue_top(audit_queue)) {
        printf("waiting\n");
    };
    //mutex lock
    audit_log(operation, status_code, &file_name, audit_fd, request_id);
    //mutex pop
    //mutex unlock
    /*
        Clear and close all memory
    */
    int close_AS = close(accept_socketfd);
    if (close_AS == -1) 
    {
        write(accept_socketfd,"HTTP/1.1 500\r\nContent-Length: 22\r\n\r\nInternal Server Error\n", 59);
    }
    content_length = 0;
    if (dont_free == false) 
    {
        free(file_name);
    }

}
