//Author: Jerry Jhird
//Source: https://codeberg.org/jerryjhird/CuoreOS
//License: MPLv2.0

#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>

#define nl_serial_write(str) serial_write(str, sizeof(str)-1) // no length serial write helper

void serial_init(void);
void serial_write(const char *msg, size_t len);
void serial_putc(char c);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_H