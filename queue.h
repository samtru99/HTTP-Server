#include <stdbool.h>

// queue has FIFO properties and should support multi producer and multi consumer
typedef struct queue queue_t;

//Allocates a queue with size 'size'
queue_t *queue_new(int size);

//frees a queue (should assume the queue is empty)
void queue_delete(queue_t **q);

//add an element to queue
//blocks if queue is full
bool queue_push(queue_t *q, void *elem);

//remove an element from the queue
//blocks if the queue is empty
bool queue_pop(queue_t *q, void **elem);

void print_queue(queue_t *q);

bool queue_empty(queue_t *q);
//Peek at the front of queue (For Audit Log)
bool queue_top(queue_t *q,void **elem);

//Pop front element (For Audit Log)
bool audit_queue_pop(queue_t *q);

