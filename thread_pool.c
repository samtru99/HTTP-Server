#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include "Task.h"
#include "queue.h"
#include "executeTask.h"

/*
    perhaps need a semophore in the loop and wait for a 
    request to get queued

    need to pass a semaphore and queue


*/
void* startThread(void* args)
{
    Sem_n_Queue* info = (Sem_n_Queue*)args;
    while(!*info->exit_cond)
    {
        if(atomic_load_explicit(&info->numOfWork, memory_order_relaxed) > 0)
        {
            atomic_fetch_sub_explicit(&info->numOfWork,1,memory_order_relaxed);
            Task *task;
            queue_pop(info->Q, &task);
            executeTask(task);
        }
        /*
        sem_wait(&info->semaphore);
        Task *task;
        queue_pop(info->Q, &task);
        executeTask(task);
        */
        //printf("val = %d ", *info->exit_cond);
        
    }
    write(STDOUT_FILENO, "EXIT \n ", 8);
    pthread_exit(NULL);
}
