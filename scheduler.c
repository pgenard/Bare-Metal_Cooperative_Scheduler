#include "scheduler.h"

#define MAX_TASKS 10

static task_desc_t tasks[MAX_TASKS];
static unsigned int num_tasks = 0;

static void scheduler_idle(void) {
  asm volatile("wfi");
}

int scheduler_add_task(task_entry_t entry, systime_t period) {
    if (num_tasks >= MAX_TASKS) {
        return -1;
    }

    tasks[num_tasks].entry = entry;
    tasks[num_tasks].period = period;
    tasks[num_tasks].last_run = systime_get();

    num_tasks++;

    return 0;
}

void scheduler_run(void) {
    while (1) {
        systime_t now = systime_get();

        for (unsigned int i = 0; i < num_tasks; i++) {
            task_desc_t *task = &tasks[i];

            if (now - task->last_run >= task->period) {
                task->last_run = now;
                task->entry();
            }
        }
    }
}

void scheduler_init(void) {
  num_tasks = 0;
}
