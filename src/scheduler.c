#include "../include/scheduler.h"
#include "../include/screen.h"
#include "../include/string.h"
#include <time.h>

static Task tasks[MAX_TASKS];
static int next_id = 1;

void sched_init(void)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = 0;
    }
}

void sched_start_task(void)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].active) {
            tasks[i].active = 1;
            tasks[i].start_time = time(NULL);
            tasks[i].elapsed_time = 0;
            tasks[i].id = next_id++;

            scr_print("Started task ID: ");
            char buf[16];
            str_itoa(tasks[i].id, buf, 16);
            scr_println(buf);
            scr_println("[Tasks running in background]");
            return;
        }
    }

    scr_println("No free task slots");
}

void sched_update(void)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active) {
            tasks[i].elapsed_time = time(NULL) - tasks[i].start_time;
        }
    }
}

void sched_list(void)
{
    int found = 0;

    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active) {
            scr_print("Task ID: ");

            char buf[16];
            str_itoa(tasks[i].id, buf, 16);
            scr_print(buf);

            scr_print(" | elapsed time (s): ");

            str_itoa((int)tasks[i].elapsed_time, buf, 16);
            scr_println(buf);

            found = 1;
        }
    }

    if (!found) {
        scr_println("No active tasks");
    }
}

void sched_kill(int id)
{
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active && tasks[i].id == id) {
            tasks[i].active = 0;
            scr_println("Task killed");
            return;
        }
    }

    scr_println("Task not found");
}