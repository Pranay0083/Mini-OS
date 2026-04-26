#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <time.h>

#define MAX_TASKS 4

typedef struct Task {
    int id;
    int active;
    time_t start_time;
    time_t elapsed_time;
} Task;

void sched_init(void);
void sched_update(void);
void sched_start_task(void);
void sched_list(void);
void sched_kill(int id);

#endif