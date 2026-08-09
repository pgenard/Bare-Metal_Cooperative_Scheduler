#ifndef UART_IMX8_H
#define UART_IMX8_H

#include "irq.h"

typedef enum {
        UART_OK = 0,
        UART_INVALID_ARGUMENT_BAUDRATE,
        UART_INVALID_ARGUMENT_WORDSIZE,
        UART_INVALID_ARGUMENT_STOP_BITS,
        UART_RECEIVE_ERROR,
        UART_NO_DATA
} uart_error;

typedef struct {
    unsigned char     data_bits;
    unsigned char     stop_bits;
    unsigned short     parity;
    unsigned int     baudrate;
} uart_config;

extern volatile unsigned char rx_buffer[];
extern volatile unsigned int rx_head;
extern volatile unsigned int rx_tail;
extern volatile unsigned int irq_count_3;
extern volatile unsigned int rx_count;

void uart_init(void);
void uart_putchar(unsigned char);
void uart_write(const unsigned char*);
unsigned char uart_getchar(void);
void interrupt_enable_bits(void);
void uart_irq_handler(struct irq_frame*);
void uart_print_hex(unsigned int);
int uart_getline(char *, unsigned int);
void uart_print_dec(unsigned int);
int uart_rx_available(void);

#endif
