#include "uart_imx8.h"
#include "gpt.h"
#include "string.h"
#include "irq.h"
#include "systime.h"
#include "scheduler.h"

#define GICD_BASE 0x38800000UL
#define GICR_BASE 0x38880000UL

#define UART1_GIC_ID 58
#define GPT1_GIC_ID 87

#define GICD_ISENABLER(n) (*(volatile unsigned int *)(GICD_BASE + 0x100 + ((n / 32) * 4)))
#define GICD_IPRIORITYR(n) (*(volatile unsigned char *)(GICD_BASE + 0x400 + (n)))
#define GICD_IROUTER(n) (*(volatile unsigned long long *)(GICD_BASE + 0x6000 + ((n) * 8)))
#define GICD_ISPENDR(n) (*(volatile unsigned int *)(0x38800000UL + 0x200 + ((n / 32) * 4)))
#define GICD_IGROUPR(n) (*(volatile unsigned int *)(GICD_BASE + 0x080 + ((n / 32) * 4)))
#define GICD_IGROUPR_ADDR(n) (GICD_BASE + 0x080 + (((n) / 32) * 4))

#define GICD_TYPER (*(volatile unsigned int *)(GICD_BASE + 0x004))
#define GICD_IIDR  (*(volatile unsigned int *)(GICD_BASE + 0x0FC))
#define GICD_ISPENDR0 (*(volatile unsigned int *)(0x38800000UL + 0x200))
#define GICR_WAKER (*(volatile unsigned int *)(GICR_BASE + 0x0014))
#define GICD_CTLR (*(volatile unsigned int *)(GICD_BASE + 0x0000))
#define GICD_RWP (*(volatile unsigned int *)(GICD_BASE + 0x0A0))
#define GICD_IGROUPR0 (*(volatile unsigned int *)(GICD_BASE + 0x080))
#define GICD_STATUSR (*(volatile unsigned int *)(GICD_BASE + 0x00C))
#define GICR_ISENABLER0 (*(volatile unsigned int *)(GICR_SGI_BASE + 0x100))
#define GICR_IGROUPR0 (*(volatile unsigned int *)(GICR_SGI_BASE + 0x080))

#define GICR_SGI_BASE (GICR_BASE + 0x10000)

#define WDOG1_BASE 0x30280000UL
#define WDOG_WCR (*(volatile unsigned short *)(WDOG1_BASE + 0x00))

#define WDOG_REBOOT_VALUE 0x0014

char command[64] = {0};

volatile unsigned long irq_count = 0;
static unsigned int gpt_start_count = 0;

static inline void enable_el2_irq(void)
{
    unsigned long hcr;

    asm volatile(
        "mrs %0, HCR_EL2"
        : "=r"(hcr));

    hcr |= (1 << 4);  // IMO
    hcr |= (1 << 3);  // FMO

    asm volatile(
        "msr HCR_EL2,%0\n"
        "isb"
        :
        : "r"(hcr));
}

static inline void route_irq_to_el2(void)
{
    unsigned long hcr;

    asm volatile(
        "mrs %0, HCR_EL2"
        : "=r"(hcr));

    hcr |= (1 << 4);   /* IMO */

    asm volatile(
        "msr HCR_EL2, %0\n"
        "isb"
        :
        : "r"(hcr));
}

static void gic_enable_group1(void)
{
    GICD_CTLR |= (1 << 1);

    uart_write("GICD_CTLR=");
    uart_print_hex(GICD_CTLR);
    uart_write("\n");
}

static void gic_wait_rwp(void)
{
    while (GICD_RWP & 1)
    {
        ;
    }
}

void gic_set_uart1_group1(void)
{
    GICD_IGROUPR(UART1_GIC_ID) =
        (1u << (UART1_GIC_ID % 32));

}

void gic_disable_distributor(void)
{
    unsigned int ctlr;

    ctlr = GICD_CTLR;
    GICD_CTLR = 0;

    asm volatile("dsb sy");
    asm volatile("isb");

    gic_wait_rwp();
}

void gic_test_enable_write(void)
{
    volatile unsigned int *reg =
        (volatile unsigned int *)(GICD_BASE + 0x100);

    *reg = 0x04000000;
}

void gic_redistributor_wake(void)
{
    unsigned int waker;

    /* Clear ProcessorSleep (bit 1) */
    waker = GICR_WAKER;
    waker &= ~(1u << 1);
    GICR_WAKER = waker;

    /* Wait until ChildrenAsleep == 0 */
    while (GICR_WAKER & (1u << 2));
}

void gic_distributor_enable(void)
{
    unsigned int ctlr;

    /*
     * Enable Affinity Routing for Non-secure Group 1 interrupts
     * and enable Group 1 interrupts.
     */
    ctlr = GICD_CTLR;

    ctlr |= (1u << 4); // 5   /* ARE_NS */
    ctlr |= (1u << 1);   /* EnableGrp1NS */

    GICD_CTLR = ctlr;

    /* Ensure write completes */
    asm volatile("dsb sy");
    asm volatile("isb");
}

void gic_enable_uart1(void)
{
    GICD_ISENABLER(UART1_GIC_ID) =
        (1u << (UART1_GIC_ID % 32));
}

void gic_route_uart1(void)
{
    GICD_IROUTER(UART1_GIC_ID) = 0;
}

static inline void gic_cpu_enable(void)
{
    unsigned long x;

    asm volatile("mrs %0, ICC_SRE_EL2" : "=r"(x));
    
    /* Enable system register interface */
    asm volatile(
        "mrs %0, ICC_SRE_EL2\n"
        "orr %0, %0, #1\n"
        "msr ICC_SRE_EL2, %0\n"
        "isb"
        : "=r"(x));
    
    /* Accept all priorities */
    x = 0xff;
    asm volatile(
        "msr ICC_PMR_EL1, %0\n"
        "isb"
        :
        : "r"(x));
    
    /* Enable Group 1 interrupts */
    x = 1;
    asm volatile(
        "msr ICC_IGRPEN1_EL1, %0\n"
        "isb"
        :
        : "r"(x));
}

static void command_md(unsigned long address)
{
    volatile unsigned int *ptr = (volatile unsigned int *) address;

    uart_print_hex((unsigned int) address);
    uart_write(": ");
    uart_print_hex(*ptr);
    uart_write("\n");
}

static void command_help(void) {
  uart_write("Commands:\n");
  uart_write("  gpt <seconds>\n");
  uart_write("  hello\n");
  uart_write("  help\n");
  uart_write("  md <address>\n");
  uart_write("  reboot\n");
  uart_write("  time\n");
  uart_write("  uname\n");
}

static void command_uname(void) {
  uart_write("[Bare-Metal i.MX8MP by Pierre GENARD (PGEN) #7]\n");
  uart_write("  UART Driver with RX Interrupts\n");
  uart_write("  UART Driver with TX Polling\n");
  uart_write("  UART Circular Buffer\n");
  uart_write("  GICv3 Init\n");	
  uart_write("  Basic Shell with Debug Commands\n");
  uart_write("  Timer Driver with Interrupts\n");
  uart_write("  Reboot with Watchdog Timer\n");
  uart_write("  Uptime Command\n");
  uart_write("  Cooperative Scheduler\n");
}

static void command_hello(void) {
  uart_write("Hello !\n");
}

static void command_unknown(void) {
  uart_write("Unknown command\n");
}

static void command_gpt(unsigned int seconds) {
    if (seconds == 0) {
        uart_write("Waiting 0s ...\n");
        uart_write("On Time !\n");
        return;
    }

    uart_write("Waiting ");
    uart_print_dec(seconds);
    uart_write("s ...\n");

    gpt_delay_seconds(seconds);

    uart_write("On Time !\n");
}

static void command_gpt_status(void) {
    uart_write("GPT_PR=");
    uart_print_hex(*(volatile unsigned int *) (0x302D0000UL + 0x04));
    uart_write("\n");
    
    uart_write("GPT_IR=");
    uart_print_hex(*(volatile unsigned int *) (0x302D0000UL + 0x0C));
    uart_write("\n");
 
    uart_write("GPT_CR=");
    uart_print_hex((*(volatile unsigned int *) (0x302D0000UL + 0x00)));
    uart_write("\n");
 
    uart_write("GPT_SR=");
    uart_print_hex((*(volatile unsigned int *) (0x302D0000UL + 0x08)));
    uart_write("\n");

    uart_write("GPT_CNT=");
    uart_print_hex((*(volatile unsigned int *) (0x302D0000UL + 0x24)));
    uart_write("\n");

    uart_write("GPT_OCR1=");
    uart_print_hex((*(volatile unsigned int *) (0x302D0000UL + 0x10)));
    uart_write("\n");
}

static void command_gpt_gic(void) {
    unsigned int n = GPT1_GIC_ID;

    uart_write("GIC ISENABLER=");
    uart_print_hex(GICD_ISENABLER(n));
    uart_write("\n");

    uart_write("GIC IGROUPR=");
    uart_print_hex(GICD_IGROUPR(n));
    uart_write("\n");

    uart_write("GIC IROUTER=");
    uart_print_hex((unsigned int) GICD_IROUTER(n));
    uart_write("\n");

    uart_write("GICD_CTLR=");
    uart_print_hex(GICD_CTLR);
    uart_write("\n");
}

static void command_gpt_pending(void) {
    unsigned int reg;
    unsigned int bit;

    reg = *(volatile unsigned int *)(GICD_BASE + 0x200 + ((GPT1_GIC_ID / 32) * 4));
    bit = (1u << (GPT1_GIC_ID % 32));

    uart_write("ISPENDR=");
    uart_print_hex(reg);
    uart_write(" bit=");
    uart_print_hex(bit);
    uart_write("\n");
}

static void system_reboot(void) {
    uart_write("Rebooting ...\r\n");

    WDOG_WCR = WDOG_REBOOT_VALUE;

    while (1) {
      asm volatile("wfi");
    }
}

static void parse_command(const char *cmd) {
    if (!strcmp(cmd, "help")) {
      command_help();
    }
    else if (!strcmp(cmd, "gicgpt")) {
      command_gpt_gic();
    }
    else if (!strncmp(cmd, "gpt ", 4)) {
      unsigned int seconds;

      seconds = strtoul_dec(cmd + 4);
      command_gpt(seconds);
    }
    else if (!strcmp(cmd, "gptp")) {
      command_gpt_pending();
    }
    else if (!strcmp(cmd, "gpts")) {
      command_gpt_status();
    }
    else if (!strcmp(cmd, "hello")) {
      command_hello();
    }
    else if (!strncmp(cmd, "md ", 3)) {
      unsigned long addr;

      addr = strtoul_hex(cmd + 3);
      command_md(addr);
    }
    else if (!strcmp(cmd, "reboot")) {
      system_reboot();
    }
    else if (!strcmp(cmd, "time")) {
      uart_write("systime = ");
      uart_print_dec(systime_get());
      uart_write(" ms\n");
    }
    else if (!strcmp(cmd, "uname")) {
      command_uname();
    }    
    else { // no match
      command_unknown();
    }
}

static void gic_set_group1(unsigned int irq) {
  GICD_IGROUPR(irq) |= (1u << (irq % 32));
}

static void gic_enable_irq(unsigned int irq) {
  GICD_ISENABLER(irq) = (1u << (irq % 32));
}

static void gic_route_irq(unsigned int irq) {
  GICD_IROUTER(irq) = 0;
}

static void gic_set_priority(unsigned int irq) {
  GICD_IPRIORITYR(irq) = 0x80;
}

static void task_shell(void) {
    static char buffer[128];
    static unsigned int pos = 0;

    if (!uart_rx_available())
        return;

    unsigned char c = uart_getchar();

    if (c == '\r') {
        uart_putchar('\n');

        buffer[pos] = '\0';

        if (pos > 0)
            parse_command(buffer);

        pos = 0;

        uart_write("Command> ");

        return;
    }

    if (c == '\b') {
        if (pos > 0) {
            pos--;
            uart_write("\b \b");
        }
        return;
    }

    if (pos < sizeof(buffer) - 1) {
        buffer[pos++] = c;
        uart_putchar(c);
    }
}

int main() {
        uart_init();
 
        gic_redistributor_wake();
	gic_distributor_enable();
	
	gic_set_group1(UART1_GIC_ID);
        gic_route_irq(UART1_GIC_ID);
        gic_set_priority(UART1_GIC_ID);
        gic_enable_irq(UART1_GIC_ID);

	gpt_init();

	gic_set_group1(GPT1_GIC_ID);
        gic_route_irq(GPT1_GIC_ID);
        gic_set_priority(GPT1_GIC_ID);
	gic_enable_irq(GPT1_GIC_ID);
	
        gpt_enable_irq();
 
	route_irq_to_el2();
        gic_cpu_enable();

	irq_register(UART1_GIC_ID, uart_irq_handler);
        irq_register(GPT1_GIC_ID, gpt_irq_handler);

        asm volatile(
          "msr daifclr, #2\n"
          "isb"
        );

	uart_write("[i.MX8MP Board Emulated by QEMU]\n");

	scheduler_init();

	scheduler_add_task(task_shell, 1);
	uart_write("Command> ");
	
	scheduler_run();

        /* while (1) {
          uart_write("Command> ");

          unsigned int ret = uart_getline(command, sizeof(command));
 
          if (ret >= 0) {
            parse_command(command);
	  } 
	  } */

       return 0;
}
