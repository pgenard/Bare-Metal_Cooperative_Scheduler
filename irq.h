#ifndef IRQ_H
#define IRQ_H

struct irq_frame {
    unsigned long x[31];
    unsigned long intid;
};

_Static_assert(sizeof(struct irq_frame) == 256,
               "irq_frame size must match assembly");

typedef void (*irq_handler_t)(struct irq_frame *frame);

void irq_handler(struct irq_frame *frame);
void irq_register(unsigned int intid, irq_handler_t handler);

#endif

