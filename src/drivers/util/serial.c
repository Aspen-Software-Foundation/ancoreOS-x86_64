// Author: Jerry Jhird
// Source: https://codeberg.org/jerryjhird/CuoreOS
// License: MPLv2.0

/*
This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at 
https://mozilla.org/MPL/2.0/.
*/

#include "includes/arch/x86_64/io.h"
#include "includes/klibc/string.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define SERIAL_COM1  0x3F8          // COM1


void serial_init(void) {
    outb(SERIAL_COM1 + 1, 0x00);    // disable interrupts
    outb(SERIAL_COM1 + 3, 0x80);    // enable DLAB
    outb(SERIAL_COM1 + 0, 0x01);    // divisor low byte (115200 baud)
    outb(SERIAL_COM1 + 1, 0x00);    // divisor high byte
    outb(SERIAL_COM1 + 3, 0x03);    
    outb(SERIAL_COM1 + 2, 0xC7);    // enable FIFO
    outb(SERIAL_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

// wait for transmitter to be empty
static void serial_wait_tx(void) {
    while (!(inb(SERIAL_COM1 + 5) & 0x20)) {}
}

static void serial_putc(char c) {
    serial_wait_tx();
    outb(SERIAL_COM1, (uint8_t)c);
}

void serial_write(const char *msg) {
    for (size_t i = 0; i < strlen(msg); i++) {
        if (msg[i] == '\n') serial_putc('\r');
        serial_putc(msg[i]);
    }
}

// Made by Yazin Tantawi
static void serial_print_hex(uint64_t num, int width) {
    char hex[] = "0123456789abcdef";
    char buffer[17]; // max 16 hex digits + null
    int i = 0;
    
    if (num == 0) {
        serial_putc('0');
        return;
    }
    
    while (num > 0 && i < 16) {
        buffer[i++] = hex[num & 0xF];
        num >>= 4;
    }
    
    while (i < width) {
        buffer[i++] = '0';
    }
    
    while (i > 0) {
        serial_putc(buffer[--i]);
    }
}


static void serial_print_dec(int64_t num) {
    if (num < 0) {
        serial_putc('-');
        num = -num;
    }
    
    if (num == 0) {
        serial_putc('0');
        return;
    }
    
    char buffer[20];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    while (i > 0) {
        serial_putc(buffer[--i]);
    }
}


void serial_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    for (const char *p = fmt; *p; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd':  
                case 'i':
                    serial_print_dec(va_arg(args, int));
                    break;
                    
                case 'u':  
                    serial_print_dec(va_arg(args, unsigned int));
                    break;
                    
                case 'x': 
                    serial_print_hex(va_arg(args, unsigned int), 0);
                    break;
                    
                case 'X': 
                    serial_print_hex(va_arg(args, unsigned int), 0);
                    break;
                    
                case 'p':  
                    serial_write("0x");
                    serial_print_hex(va_arg(args, uint64_t), 16);
                    break;
                    
                case 's': 
                    serial_write(va_arg(args, const char*));
                    break;
                    
                case 'c': 
                    serial_putc((char)va_arg(args, int));
                    break;
                    
                case '%': 
                    serial_putc('%');
                    break;
                    
                default:
                    serial_putc('%');
                    serial_putc(*p);
                    break;
            }
        } else {
            if (*p == '\n') serial_putc('\r');
            serial_putc(*p);
        }
    }
    
    va_end(args);
}

//End code attribution