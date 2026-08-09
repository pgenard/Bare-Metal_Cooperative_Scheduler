.section .text.boot
.global _start

_start:

    //
    // Setup EL2 stack
    //
    ldr x0, =__stack_top
    mov sp, x0

    //
    // Install exception vectors
    //
    adr x0, vectors_el2
    msr VBAR_EL2, x0
    isb

    //
    // Copy initialized data
    //
    ldr x0, =__sidata
    ldr x1, =__sdata
    ldr x2, =__edata

1:
    cmp x1, x2
    b.ge 2f

    ldr x3, [x0], #8
    str x3, [x1], #8
    b 1b

2:

    //
    // Clear .bss
    //
    ldr x1, =__sbss
    ldr x2, =__ebss
    mov x0, xzr

3:
    cmp x1, x2
    b.ge 4f

    str x0, [x1], #8
    b 3b

4:
    bl main

hang:
    wfe
    b hang
