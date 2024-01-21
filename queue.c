#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct queue {
    pthread_mutex_t queue_op;
    pthread_cond_t queue_full;
    pthread_cond_t queue_empty;
    int head;
    int tail;
    int size;
    int counter;
    void **buffer;
} queue_t;

queue_t *queue_new(int size) {
    queue_t *Q = malloc(sizeof(queue_t));
    Q->head = 0;
    Q->tail = 0;
    Q->counter = 0;
    Q->size = size;
    Q->buffer = (void *) malloc((size) * sizeof(void *));
    pthread_mutex_init(&Q->queue_op, NULL);
    pthread_cond_init(&Q->queue_full, NULL);
    pthread_cond_init(&Q->queue_empty, NULL);
    return Q;
}

void queue_delete(queue_t **q) {
    if ((*q) == NULL) {
        return;
    }
    pthread_mutex_destroy(&(*q)->queue_op);
    pthread_cond_destroy(&(*q)->queue_empty);
    pthread_cond_destroy(&(*q)->queue_full);
    free((*q)->buffer);
    free((*q));
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) 
{
    printf("in push\n");
    if (!q) {
        return false;
    }
    //printf("at push\n");
    pthread_mutex_lock(&q->queue_op);
    while (q->counter == q->size) {
        printf("full \n");
        pthread_cond_wait(&q->queue_full, &q->queue_op);
    }

    q->buffer[q->head] = (void *) elem;
    q->head = (q->head + 1) % q->size;
    q->counter += 1;
    pthread_mutex_unlock(&q->queue_op);
    pthread_cond_signal(&q->queue_empty);
    printf("pushed\n");
    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    printf("in pop\n");
    // (*elem) == NULL
    if (!q) {
        return false;
    }
    pthread_mutex_lock(&q->queue_op);
    while (q->counter == 0) {
        printf("waiting\n");
        pthread_cond_wait(&q->queue_empty, &q->queue_op);
    }
    *elem = q->buffer[q->tail];
    q->buffer[q->tail] = NULL;
    q->tail = (q->tail + 1) % q->size;
    q->counter -= 1;
    pthread_mutex_unlock(&q->queue_op);
    pthread_cond_signal(&q->queue_full);
    return true;
}

int queue_top(queue_t *q)
{
    //printf("in queue top\n");
    if(!q) {
        return false;
    }
    int val;
    pthread_mutex_lock(&q->queue_op);
    val = q->buffer[q->tail];
    pthread_mutex_unlock(&q->queue_op);
    return val;
}
