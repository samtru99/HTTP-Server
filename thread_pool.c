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
    while(1)
    {
        sem_wait(&info->semaphore);
        Task *task;
        queue_pop(info->Q, &task);
        executeTask(task);        
    }
}
