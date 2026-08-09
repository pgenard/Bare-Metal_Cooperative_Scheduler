#include "gpt.h"
#include "uart_imx8.h"
#include "irq.h"

#define GPT1_BASE 0x302D0000UL

#define GPT_SR 0x08
#define GPT_IR 0x0C
#define GPT_OCR1 0x10

#define GPT_SR_OF1 (1U << 0)
#define GPT_IR_OF1IE (1U << 0)

#define GPT_REG(offset) (*(volatile unsigned int *) (GPT1_BASE + (offset)))

#define GPT_CR (*(volatile unsigned int *) (GPT1_BASE + 0x00))
#define GPT_PR (*(volatile unsigned int *) (GPT1_BASE + 0x04))
#define GPT_CNT (*(volatile unsigned int *) (GPT1_BASE + 0x24))

#define GPT_CR_EN (1 << 0)
#define GPT_CR_ENMOD (1 << 1)
#define GPT_CR_SWR (1 << 15)
#define GPT_CR_FRR (1 << 9)

#define GPT_CR_CLKSRC_SHIFT 6
#define GPT_CR_CLKSRC_MASK (0x7 << GPT_CR_CLKSRC_SHIFT)

#define GPT_CLK_IPG 1

volatile unsigned int gpt_irq_received = 0;

volatile unsigned int gpt_seconds = 0;
volatile unsigned int gpt_start_cnt = 0;
volatile unsigned int gpt_ticks = 0;

static unsigned int gpt_periodic_ticks;
static unsigned int gpt_periodic;

void gpt_test_compare(unsigned int ticks) {
    GPT_REG(GPT_SR) = GPT_SR_OF1;
    GPT_REG(GPT_OCR1) = GPT_REG(GPT_CNT) + ticks;
}

void gpt_start_periodic(unsigned int ticks) {
    gpt_periodic_ticks = ticks;
    gpt_periodic = 1;

    GPT_REG(GPT_IR) = 0;
    GPT_REG(GPT_SR) = GPT_SR_OF1;

    GPT_REG(GPT_OCR1) = GPT_REG(GPT_CNT) + ticks;

    GPT_REG(GPT_IR) = GPT_IR_OF1IE;
}

void gpt_init(void)
{
    GPT_CR = 0;

    GPT_CR = GPT_CR_SWR;
    while (GPT_CR & GPT_CR_SWR);

    /* Clear all pending GPT status */
    GPT_REG(GPT_SR) = 0x3F;

    GPT_PR = 0;

    GPT_CR =
        GPT_CR_ENMOD |
      // GPT_CR_FRR |
        (5 << GPT_CR_CLKSRC_SHIFT);
    /* GPT_CR =
    GPT_CR_ENMOD |
    (5 << GPT_CR_CLKSRC_SHIFT); */

    /*
     * Clear again after OCR programming
     */
    GPT_REG(GPT_SR) = 1;

    /*
     * Disable OCR1 interrupt
     */
    GPT_REG(GPT_IR) = 0;

    /*
     * Start counting
     */
    GPT_CR |= GPT_CR_EN;
}

/* void gpt_irq_handler(struct irq_frame *frame) {
  (void) frame;
  GPT_REG(GPT_SR) = 1;
  gpt_irq_received = 1;
  GPT_REG(GPT_IR) = 0;
} */

/* void gpt_irq_handler(struct irq_frame *frame) {
    (void) frame;

    GPT_REG(GPT_SR) = GPT_SR_OF1;

    if (gpt_periodic) {
        GPT_REG(GPT_OCR1) += gpt_periodic_ticks;
    } else {
        gpt_irq_received = 1;
        GPT_REG(GPT_IR) = 0;
    }
} */

void gpt_irq_handler(struct irq_frame *frame) {
  (void) frame;

  GPT_REG(GPT_SR) = 1;
  gpt_irq_received = 1;
  GPT_REG(GPT_IR) = 0;
}

void gpt_enable_irq(void) {
  GPT_REG(GPT_IR) = 1;
}

unsigned int gpt_read_cnt(void) {
  return GPT_CNT;
}

void gpt_start_one_shot(unsigned int ticks) {
    gpt_irq_received = 0;

    GPT_REG(GPT_IR) = 0;
    GPT_REG(GPT_SR) = 1;
    GPT_REG(GPT_OCR1) = GPT_CNT + ticks;
    GPT_REG(GPT_IR) = 1;
}

void gpt_delay_seconds(unsigned int seconds) {
  gpt_start_one_shot(seconds * GPT_TICKS_PER_SECOND);

  while (!gpt_irq_received) {
    asm volatile("wfe");
  }
}
