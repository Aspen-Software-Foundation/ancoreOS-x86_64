/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: stdio.c
    Description: The stdio module for the VNiX Operating System.
    Author: Yazin Tantawi

    All components of the VNiX Operating System, except where otherwise noted, 
    are copyright of the Aspen Software Foundation (and the corresponding author(s)) and licensed under GPLv2 or later.
    For more information on the Gnu Public License Version 2, please refer to the LICENSE file
    or to the link provided here: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

 * THIS OPERATING SYSTEM IS PROVIDED "AS IS" AND "AS AVAILABLE" UNDER 
 * THE GNU GENERAL PUBLIC LICENSE VERSION 2, WITHOUT
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NON-INFRINGEMENT.
 * 
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
 * THE AUTHORS, COPYRIGHT HOLDERS, OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE), ARISING IN ANY WAY OUT OF THE USE OF THIS OPERATING SYSTEM,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE OPERATING SYSTEM IS
 * WITH YOU. SHOULD THE OPERATING SYSTEM PROVE DEFECTIVE, YOU ASSUME THE COST OF
 * ALL NECESSARY SERVICING, REPAIR, OR CORRECTION.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF THE GNU GENERAL PUBLIC LICENSE
 * ALONG WITH THIS OPERATING SYSTEM; IF NOT, WRITE TO THE FREE SOFTWARE
 * FOUNDATION, INC., 51 FRANKLIN STREET, FIFTH FLOOR, BOSTON,
 * MA 02110-1301, USA.
*/

#include <stdlib.h>
#include "drivers/terminal/src/flanterm.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// Global flanterm context (should be set in kernel_main)
struct flanterm_context *global_flanterm = NULL;

int printf(const char *format, ...) {
    if (!global_flanterm) return -1;

    va_list args;
    va_start(args, format);

    char c;
    char buf[64];
    char *p = NULL;
    int pad = 0;
    char pad_char = ' ';
    int chars_written = 0;

    while ((c = *format++) != 0) {
        pad = 0;
        pad_char = ' ';

        if (c != '%') {
            flanterm_write(global_flanterm, &c, 1);
            chars_written++;
            continue;
        }

        // handle padding: %0width
        if (*format == '0') {
            pad_char = '0';
            format++;
        }

        while (*format >= '0' && *format <= '9') {
            pad = pad * 10 + (*format - '0');
            format++;
        }

        // handle length modifiers: l, ll, L
        int is_long = 0;
        int is_longlong = 0;
        if (*format == 'l') {
            format++;
            if (*format == 'l') {
                is_longlong = 1;
                format++;
            } else {
                is_long = 1;
            }
        } else if (*format == 'L') {
            is_longlong = 1;
            format++;
        }

        c = *format++;
        p = NULL;

        switch (c) {
            case 'd':
            case 'u':
            case 'x':
            case 'o': {
                if (is_longlong) {
                    uint64_t val = va_arg(args, uint64_t);
                    if (c == 'x') {
                        flanterm_write(global_flanterm, "0x", 2);
                        chars_written += 2;
                    }
                    itoa(val, buf, (c == 'x') ? 16 : (c == 'o') ? 8 : 10);
                    p = buf;
                } else if (is_long) {
                    long val = va_arg(args, long);
                    if (c == 'x') {
                        flanterm_write(global_flanterm, "0x", 2);
                        chars_written += 2;
                    }
                    itoa(val, buf, (c == 'x') ? 16 : (c == 'o') ? 8 : 10);
                    p = buf;
                } else {
                    int val = va_arg(args, int);
                    if (c == 'x') {
                        flanterm_write(global_flanterm, "0x", 2);
                        chars_written += 2;
                    }
                    itoa(val, buf, (c == 'x') ? 16 : (c == 'o') ? 8 : 10);
                    p = buf;
                }
                break;
            }

            case 'f': {
                double fval = va_arg(args, double);
                int int_part = (int)fval;
                double frac = fval - int_part;
                itoa(int_part, buf, 10);
                
                // Write integer part
                char *temp = buf;
                while (*temp) {
                    flanterm_write(global_flanterm, temp, 1);
                    temp++;
                    chars_written++;
                }
                
                // Write decimal point
                flanterm_write(global_flanterm, ".", 1);
                chars_written++;
                
                // Write fractional part
                frac *= 1000000;
                itoa((int)frac, buf, 10);
                temp = buf;
                while (*temp) {
                    flanterm_write(global_flanterm, temp, 1);
                    temp++;
                    chars_written++;
                }
                continue;
            }

            case 's':
                p = va_arg(args, char *);
                if (!p) p = "(null)";
                break;

            case 'c': {
                char ch = (char)va_arg(args, int);
                flanterm_write(global_flanterm, &ch, 1);
                chars_written++;
                continue;
            }

            case 'p':
                flanterm_write(global_flanterm, "0x", 2);
                chars_written += 2;
                itoa((uintptr_t)va_arg(args, void *), buf, 16);
                p = buf;
                break;

            case 'm': {
                char *mem = va_arg(args, char *);
                if (!mem) mem = "(null)";
                int len = pad ? pad : 16;
                for (int i = 0; i < len; i++) {
                    flanterm_write(global_flanterm, &mem[i], 1);
                    chars_written++;
                }
                continue;
            }

            default: {
                flanterm_write(global_flanterm, &c, 1);
                chars_written++;
                continue;
            }
        }

        // If we have a string to print (p is set)
        if (p) {
            // Calculate length
            int len = 0;
            char *p2 = p;
            while (*p2++) len++;
            
            // Print padding
            for (int i = len; i < pad; i++) {
                flanterm_write(global_flanterm, &pad_char, 1);
                chars_written++;
            }
            
            // Print the string
            while (*p) {
                flanterm_write(global_flanterm, p, 1);
                p++;
                chars_written++;
            }
        }
    }

    va_end(args);
    return chars_written;
}