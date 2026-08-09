# Bare-Metal Cooperative Scheduler on Emulated i.MX8MP with QEMU.

## About

**Bare-Metal Cooperative Scheduler** demonstrates bare-metal software development on an **ARMv8-A** processor with a cooperative scheduler. The project configures the **Generic Interrupt Controller (GICv3)**, **NXP i.MX8 UART** to receive interrupts, and the **i.MX8MP General Purpose Timer (GPT)**.

A simple command-line interface illustrates how UART interrupts, exception handling, timer configuration and low-level hardware access work together in a minimal bare-metal environment.

### Features
- Bare-metal ARMv8-A startup code
- EL2 exception vector table
- GICv3 initialization and interrupt routing
- Interrupt-driven UART receive
- Circular receive buffer
- Polling-based UART transmit
- Simple serial command-line interface
- Memory inspection with debug command
- 16 MHz GPT clock source configuration
- GPT timer delay
- Software IRQ handler
- Uptime command
- Cooperative scheduler

### Hints

Our QEMU emulated platform page is available here: [i.MX8MP](https://www.qemu.org/docs/master/system/arm/imx8m.html)

Our DDR beginning seen by QEMU is mapped on *0x40000000* address.

Our UART base register is mapped on *0x30860000* address.

### Toolchain

The project uses the ARM GNU bare-metal toolchain: [ARM Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

## Compiling

### Compiler
```bash
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib string.c
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib gpt.c
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib uart_imx8.c
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib systime.c
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib scheduler.c
$ ./aarch64-none-elf-gcc -S -mcpu=cortex-a53 -ffreestanding -nostdlib main.c
 ```

### Assembler
```bash
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o startup.o startup.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o vectors.o vectors.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o exceptions.o exceptions.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o string.o string.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o gpt.o gpt.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o uart_imx8.o uart_imx8.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o systime.o systime.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o scheduler.o scheduler.s
$ ./aarch64-none-elf-as -mcpu=cortex-a53 -o main.o main.s
```

### Linker
```bash
$ ./aarch64-none-elf-ld -T linker.ld -o hello.elf startup.o vectors.o exceptions.o string.o uart_imx8.o gpt.o systime.o scheduler.o main.o
 ```

### Raw Binary
```bash
$ ./aarch64-none-elf-objcopy -O binary hello.elf hello.bin
```

## Executing our Serial Shell
```bash
$ ./qemu/build/qemu-system-aarch64 -M imx8mp-evk -m 512M -kernel hello.bin -no-reboot -display none -serial stdio -monitor telnet:127.0.0.1:1234,server,nowait
[i.MX8MP Board Emulated by QEMU]
Command> hello
Hello !
Command> time
systime = 3270 ms
Command> uname
[Bare-Metal i.MX8MP by Pierre GENARD (PGEN) #7]
  UART Driver with RX Interrupts
  UART Driver with TX Polling
  UART Circular Buffer
  GICv3 Init
  Basic Shell with Debug Commands
  Timer Driver with Interrupts
  Reboot with Watchdog Timer
  Uptime Command
  Cooperative Scheduler
Command> help
Commands:
  gpt <seconds>
  hello
  help
  md <address>
  reboot
  time
  uname
Command> time
systime = 11036 ms
Command> time
systime = 13631 ms
Command> reboot
Rebooting ...
[i.MX8MP Board Emulated by QEMU]
Command>
```

## Debugging
```bash
$ telnet localhost 1234
```

### Useful QEMU Traces:

```bash
-trace enable=imx_gpt_timeout
-trace enable=gicv3_dist_set_irq
-trace enable=gicv3_cpuif_set_irqs
```

## Author
**Pierre GENARD**
