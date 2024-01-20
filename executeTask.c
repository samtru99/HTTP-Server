#include "Task.h"

void executeTask(Task* task)
{
    //task->task_function
    task->task_function(task->socket, task->logfile);
}

