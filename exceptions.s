.section .text

.extern irq_count
	
.global sync_sp0
.global irq_sp0
.global fiq_sp0
.global serr_sp0

.global sync_spx
.global irq_spx
.global fiq_spx
.global serr_spx

.global sync_lower64
.global irq_lower64
.global fiq_lower64
.global serr_lower64

.global sync_lower32
.global irq_lower32
.global fiq_lower32
.global serr_lower32

.macro HANDLER name

\name:

    mrs x0, ESR_EL2
    mrs x1, ELR_EL2
    mrs x2, FAR_EL2
    mrs x3, SPSR_EL2

1:
    wfe
    b 1b

.endm

HANDLER sync_sp0
	
.global irq_sp0
irq_sp0:
    mov x0, #'0'
    bl uart_putchar
    eret

	
HANDLER fiq_sp0
HANDLER serr_sp0

sync_spx:
    mov x0, #'S'
    bl uart_putchar
1:  b 1b

.equ IRQ_FRAME_SIZE, 256

.global irq_spx
.type irq_spx, "function"

irq_spx:
    sub sp, sp, #IRQ_FRAME_SIZE

    stp x0,  x1,  [sp, #0]
    stp x2,  x3,  [sp, #16]
    stp x4,  x5,  [sp, #32]
    stp x6,  x7,  [sp, #48]
    stp x8,  x9,  [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]

    str x30, [sp, #240]

    /*
     * Acknowledge interrupt.
     * Pass INTID to C in x0.
     */
    // mrs x0, ICC_IAR1_EL1

    /*
     * Save INTID in the frame.
     */
    // str x0, [sp, #248]

    /* Read interrupt acknowledge register */
    mrs x1, ICC_IAR1_EL1

    /* Save INTID */
    str x1, [sp, #248]

    /* Pass struct irq_frame * to C */
    mov x0, sp
	
    bl irq_handler

    /*
     * Recover INTID for EOI.
     */
    ldr x0, [sp, #248]

    msr ICC_EOIR1_EL1, x0
    isb

    /*
     * Restore interrupted context.
     */
    ldp x0,  x1,  [sp, #0]
    ldp x2,  x3,  [sp, #16]
    ldp x4,  x5,  [sp, #32]
    ldp x6,  x7,  [sp, #48]
    ldp x8,  x9,  [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]

    ldr x30, [sp, #240]

    add sp, sp, #IRQ_FRAME_SIZE

    eret

serr_spx:
    mov x0, #'E'
    bl uart_putchar
1:  b 1b

fiq_spx:
    mov x0, #'F'
    bl uart_putchar
1:  b 1b

HANDLER sync_lower64
HANDLER irq_lower64
HANDLER fiq_lower64
HANDLER serr_lower64

HANDLER sync_lower32
HANDLER irq_lower32
HANDLER fiq_lower32
HANDLER serr_lower32
