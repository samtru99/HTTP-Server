#ifndef TASK_H
#define TASK_H
#include "queue.h"
#include <semaphore.h>
#include <pthread.h>

typedef struct Task {
    void(*task_function)(int,int,int, *queue_t, *pthread_mutex_t);
    queue_t *audit_queue;
    int u_id;
    int socket;
    int logfile;
    pthread_mutex_t audit_mutex;
}Task;

typedef struct Sem_n_Queue {
    sem_t *semaphore;
    queue_t *Q;
} Sem_n_Queue;

#endif
