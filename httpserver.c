/*
    Header Files
*/
#include "bind.h"
#include "extract_header.h"
#include "request_regex.h"
#include "put.h"
#include "get.h"
#include "head.h"
#include "audit.h"
#include "verify_header.h"
#include "processRequest.h"
#include "queue.h"
#include "Task.h"
#include "thread_pool.h"
/*
    Libraries
*/
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
#include <pthread.h>
#include <semaphore.h>
//Function to convert user input to a port socket
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

int main(int argc, char *argv[])
{
    
    /*
        Error Check User Port Number inputs
    */

    if (strlen(argv[argc - 1]) > 5) 
    {
            errx(1, "invalid argument: ./httpserver port_num\nusage: ./httpserver <port>");
            return 0;
    }
    bool valid = true;
    int str_len = 0;
    for (int i = 0; argv[argc - 1][i] != '\0'; i++) {
        if (argv[argc - 1][i] < 48 || argv[argc - 1][i] > 57) {
            valid = false;
        }
        str_len++;
    }
    if (valid == false) 
    {
        errx(1, "invalid port number: %s ", argv[argc - 1]);
    }
    /*
        1. PARSE THROUGH COMMAND LINE AND COLLECT INFO
    */
    int audit_fd = 0;
    int threads = 4;
    int options;
    bool close_audit = false;
    while ((options = getopt(argc, argv, "t:l:")) != -1) 
    {
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
    
    /*
        2. Convert Port #
    */
    uint16_t converted_port_num = convert(argv[argc - 1], str_len);
    if (converted_port_num < 1025) 
    {
        errx(1, "invalid port number: %s ", argv[1]);
    }
    /*
        3. Create socket to listen for request
    */
    int socketfd = create_listen_socket(converted_port_num);
    if (socketfd < 0) {
        errx(1, "invalid socket number: %s ", argv[1]);
    }
    /*
        4. Listen and send out request to queue
    */
    queue_t *Q = queue_new(5);
    sem_t num_of_requests;
    sem_init(&num_of_requests, 1,0);
    Sem_n_Queue* info = (Sem_n_Queue*)malloc(sizeof(Sem_n_Queue));
    info->semaphore = num_of_requests;
    info->Q = Q;
    /*
        5. Set up worker threads
    */
    pthread_t t;
    pthread_t th[threads];
    //pthread_create(&t, NULL,&startThread, (void*)info );
    
    for(int i = 0; i < 1; i++)
    {
        if(pthread_create(&th[i], NULL, &startThread, (void*)info) != 0)
        {
            perror("Failed to create the thead");
        }
    }
    //pthread_join(t, NULL);
    /*
        6. Wait and Listen
    */

    
    while (true)
    {
        //Listen to Socket
        printf("waiting for connection\n");
        int accept_socketfd = accept(socketfd, NULL, NULL);
        //Create new task to send to queue
        Task t = {
            .task_function = &process_request,
            .socket = accept_socketfd,
            .logfile = audit_fd
        };
        //Push to queue
        queue_push(Q, &t);
        printf("alerting\n");
        sem_post(&num_of_requests);
    }
    /*
    for(int i = 0; i < threads; i++)
    {
        if(pthread_join(th[i], NULL) != 0)
        {
            perror("Failed to join the thread");
        }
    }
    */
    /*
        let all task finish
    */

   
    return 0;
}
