.section .vectors, "ax"
.align 11

.global vectors_el2

.macro VECTOR handler
    b \handler
    .space 128 - 4
.endm

vectors_el2:

    // Current EL using SP0
    VECTOR sync_sp0
    VECTOR irq_sp0
    VECTOR fiq_sp0
    VECTOR serr_sp0

    // Current EL using SPx
    VECTOR sync_spx
    VECTOR irq_spx
    VECTOR fiq_spx
    VECTOR serr_spx

    // Lower EL AArch64
    VECTOR sync_lower64
    VECTOR irq_lower64
    VECTOR fiq_lower64
    VECTOR serr_lower64

    // Lower EL AArch32
    VECTOR sync_lower32
    VECTOR irq_lower32
    VECTOR fiq_lower32
    VECTOR serr_lower32
