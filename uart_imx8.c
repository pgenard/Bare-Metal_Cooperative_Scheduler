#include "uart_imx8.h"
#include "irq.h"

#define UART_BASE 0x30860000UL

#define URXD  0x00 /* UART Receiver Register */
#define UTXD  0x40 /* UART Transmitter Register */
#define UCR1  0x80 /* UART Control Register 1 */
#define UCR2  0x84 /* UART Control Register 2 */
#define UCR3  0x88 /* UART Control Register 3 */
#define UCR4  0x8c /* UART Control Register 4 */
#define UFCR  0x90 /* UART FIFO Control Register */
#define USR1  0x94 /* UART Status Register 1 */
#define USR2  0x98 /* UART Status Register 2 */
#define UESC  0x9c /* UART Escape Character Register */
#define UTIM  0xa0 /* UART Escape Timer Register */
#define UBIR  0xa4 /* UART BRM Incremental Register */
#define UBMR  0xa8 /* UART BRM Modulator Register */
#define UBRC  0xac /* UART Baud Rate Counter Register */
#define ONEMS 0xb0 /* UART One Millisecond Register */
#define UTS   0xb4 /* UART Test Register */

#define UART_SR2_TXFIFO_EMPTY (1u << 14)
#define UART_SR2_RXFIFO_RDR   (1u << 0)
#define UART_SR1_TRDY         (1u << 13)
#define UART_SR1_RRDY         (1u << 9)

#define UART_REG(offset) (*(volatile unsigned int*) (UART_BASE + (offset)))
#define GICD_ISPENDR(n) (*(volatile unsigned int *)(0x38800000UL + 0x200 + ((n / 32) * 4)))

#define RX_BUFFER_SIZE 128

volatile unsigned char rx_buffer[RX_BUFFER_SIZE];
volatile unsigned int rx_head = 0;
volatile unsigned int rx_tail = 0;

volatile unsigned int irq_count_3 = 0;
volatile unsigned int rx_count = 0;

int uart_getline(char *buf, unsigned int size) {
    unsigned int pos = 0;

    while (1)
    {
        unsigned char c = uart_getchar();

        uart_putchar(c);

        if (c == '\r')
        {
	    uart_putchar('\n');
            buf[pos] = '\0';
            return pos;
        }

        if (pos < size - 1)
        {
            buf[pos] = c;
            pos++;
        }
    }
}

void uart_putchar(unsigned char c) {
    while (!(UART_REG(USR2) & UART_SR2_TXFIFO_EMPTY));
    UART_REG(UTXD) = c;
}

unsigned char uart_getchar(void) {
    while (rx_head == rx_tail)
    {
        ;
    }

    asm volatile("" ::: "memory");
    
    unsigned char c = rx_buffer[rx_tail];
    
    rx_tail++;

    if (rx_tail >= RX_BUFFER_SIZE)
        rx_tail = 0;
	
    return c;
}

void uart_init(void)
{
    /* Disable UART */
    UART_REG(UCR1) = 0;

    /* Configure clock divider */
    UART_REG(UFCR) = 0x0001;
    
    /* Configure baud */
    UART_REG(UBIR) = 0x08FF;
    UART_REG(UBMR) = 0x0C34;

    /* 8N1 + enable TX/RX */
    UART_REG(UCR2) = 0x2127;

    /* Enable RX interrupt */
    UART_REG(UCR4) |= (1 << 0);

    /* Enable UART + RRDY interrupt */
    // UART_REG(UCR1) = 0x2201;
    UART_REG(UCR1) = 0x0201;
}

void uart_write(const unsigned char* data) {
    while (*data) {
        uart_putchar(*data++);
    }
}

void uart_irq_handler(struct irq_frame *frame) {
    (void) frame;

    irq_count_3++;
    
    while (UART_REG(USR2) & UART_SR2_RXFIFO_RDR)
    {
        unsigned char c = UART_REG(URXD);	
        unsigned int next = rx_head + 1;

        if (next >= RX_BUFFER_SIZE)
            next = 0;
	
        if (next != rx_tail)
        {
            rx_buffer[rx_head] = c;
	    
            asm volatile("" ::: "memory");
 
            rx_head = next;
            rx_count++;
	  
        }     
    }
}

void uart_print_hex(unsigned int value)
{
    const char *hex = "0123456789ABCDEF";
    
    char buf[9];

    for (int i = 0; i < 8; i++){
      buf[i] = hex[(value >> (28 - 4 * i)) & 0xF];
    }

    buf[8] = '\0';
    
    uart_write((const unsigned char*) buf);
}

void uart_print_dec(unsigned int value) {
    char buffer[11];
    int i = 0;

    if (value == 0) {
        uart_write("0");
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        uart_putchar(buffer[--i]);
    }
}

int uart_rx_available(void) {
    return rx_head != rx_tail;
}
