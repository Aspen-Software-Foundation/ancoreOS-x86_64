// Author: Jerry Jhird
// Source: https://codeberg.org/jerryjhird/CuoreOS
// License: MPLv2.0

/*
This Source Code Form is subject to the terms of the Mozilla Public License, version 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at 
https://mozilla.org/MPL/2.0/.
*/

#ifndef SERIAL_H
#define SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#define SERIAL_COM1  0x3F8 

void serial_init(void);
void serial_write(const char *format, ...);
void putc(char c);
void serial_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_H