ENTRY(_start)

MEMORY
{
    RAM (rwx) : ORIGIN = 0x40000000, LENGTH = 512M
}

SECTIONS
{
    . = ORIGIN(RAM);

    .text.boot :
    {
        *(.text.boot)
    } > RAM

    . = ALIGN(2048);

    .vectors :
    {
        *(.vectors)
    } > RAM

    .text :
    {
        *(.text*)
        *(.rodata*)
    } > RAM

    __sidata = LOADADDR(.data);

    .data :
    {
        __sdata = .;
        *(.data*)
        __edata = .;
    } > RAM

    .bss (NOLOAD):
    {
        __sbss = .;
        *(.bss*)
        *(COMMON)
        __ebss = .;
    } > RAM

    . = ALIGN(16);

    . += 16384;

    __stack_top = .;
}
