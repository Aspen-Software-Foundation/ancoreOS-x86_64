//Author: Jerry Jhird
//Source: https://codeberg.org/jerryjhird/CuoreOS
//License: MPLv2.0

#include "../../includes/arch/x86_64/io.h"
#include <stdint.h>
#include <stddef.h>

#define SERIAL_COM1  0x3f8          // COM1
void serial_init(void) {
   outb(SERIAL_COM1  + 1, 0x00);    // Disable all interrupts
   outb(SERIAL_COM1  + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(SERIAL_COM1  + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(SERIAL_COM1  + 1, 0x00);    //                  (hi byte)
   outb(SERIAL_COM1  + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(SERIAL_COM1  + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(SERIAL_COM1  + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(SERIAL_COM1  + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(SERIAL_COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)


    outb(SERIAL_COM1  + 4, 0x1E); 
    outb(SERIAL_COM1  + 0, 0xAE);
}

// wait for transmitter to be empty
static void serial_wait_tx(void) {
    while (!(inb(SERIAL_COM1  + 5) & 0x20)) {}
}

static void serial_putc(char c) {
    serial_wait_tx();
    outb(SERIAL_COM1 , (uint8_t)c);
}

void serial_write(const char *msg, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (msg[i] == '\n') serial_putc('\r');
        serial_putc(msg[i]);
    }
}