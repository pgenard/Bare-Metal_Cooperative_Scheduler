#include "irq.h"

#define GIC_MAX_INTERRUPTS 1024

static irq_handler_t irq_handlers[GIC_MAX_INTERRUPTS];

void irq_register(unsigned int intid, irq_handler_t handler) {
    if (intid >= GIC_MAX_INTERRUPTS)
        return;

    irq_handlers[intid] = handler;
}

void irq_handler(struct irq_frame *frame) {
    unsigned int intid = frame->intid;

    if (intid >= GIC_MAX_INTERRUPTS)
        return;

    if (irq_handlers[intid] != 0)
    {
        irq_handlers[intid](frame);
    }
}
