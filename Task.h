#ifndef TASK_H
#define TASK_H
#include "queue.h"
#include <semaphore.h>

typedef struct Task {
    void(*task_function)(int,int);
    int socket;
    int logfile;
}Task;

typedef struct Sem_n_Queue {
    sem_t semaphore;
    queue_t *Q;
} Sem_n_Queue;

#endif
