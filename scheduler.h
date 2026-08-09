#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "systime.h"

typedef void (*task_entry_t)(void);

typedef struct {
  task_entry_t entry;
  systime_t period;
  systime_t last_run;
} task_desc_t;

int scheduler_add_task(task_entry_t entry, systime_t period);
void scheduler_run(void);
void scheduler_init(void);

#endif
