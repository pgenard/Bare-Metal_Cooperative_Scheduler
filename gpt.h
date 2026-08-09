#ifndef GPT_H
#define GPT_H

#include "irq.h"

#define GPT_TICKS_PER_SECOND 16000000

extern volatile unsigned int gpt_irq_received;
extern volatile unsigned int gpt_seconds;
extern volatile unsigned int gpt_start_cnt;

void gpt_init(void);
void gpt_test(void);
unsigned int gpt_get_counter(void);
void gpt_set_compare(void);
void gpt_enable_irq(void);
void gpt_irq_handler(struct irq_frame*);
unsigned int gpt_get_count(void);
unsigned int gpt_read_cnt(void);
void gpt_start_one_shot(unsigned int);
void gpt_delay_seconds(unsigned int);
void gpt_start_periodic(unsigned int);
void gpt_test_compare(unsigned int);

#endif
