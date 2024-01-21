#include "Task.h"
#include "queue.h"
void executeTask(Task* task)
{
    //task->task_function
    task->task_function(task->socket, task->logfile, task->u_id, task->audit_queue);
}

