CROSS = ./aarch64-none-elf-

CC = $(CROSS)gcc
AS = $(CROSS)as
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS = -mcpu=cortex-a53 -ffreestanding
ASFLAGS = -mcpu=cortex-a53
LDFLAGS = -T linker.ld

TARGET = hello

C_SRCS = scheduler.c systime.c irq.c gpt.c uart_imx8.c string.c main.c
ASM_SRCS = startup.s vectors.s exceptions.s

C_ASM = $(C_SRCS:.c=.s)
C_OBJS = $(C_SRCS:.c=.o)
ASM_OBJS = $(ASM_SRCS:.s=.o)

OBJS = $(C_OBJS) $(ASM_OBJS)

all: $(TARGET).bin

$(C_ASM): %.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

$(C_OBJS): %.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(ASM_OBJS): %.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET).elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

run: $(TARGET).bin
	qemu-system-aarch64 -M imx8mp-evk -kernel $(TARGET).bin -serial stdio

clean:
	rm -v $(C_ASM) $(OBJS) $(TARGET).elf $(TARGET).bin

.PHONY: all run clean
