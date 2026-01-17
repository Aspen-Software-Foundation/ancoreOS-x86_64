/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: stdio.c
    Description: The stdio module for the Ancore Operating System.
    Author: Yazin Tantawi

    All components of the Ancore Operating System, except where otherwise noted, 
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
#include "drivers/terminal/src/cuoreterm.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

static struct terminal *global_term = NULL;
static uint32_t global_fg = 0xFFFFFF;

void terminal_set_instance(struct terminal *term, uint32_t fg) {
    global_term = term;
    global_fg = fg;
}

int kprintf(const char *format, ...) {
    if (!global_term) return -1;

    va_list args;
    va_start(args, format);

    uint32_t c;
    char buf[64];
    char *p = NULL, *p2 = NULL;
    int pad = 0;
    char pad_char = ' ';
    int chars_written = 0;

    while ((c = *format++) != 0) {
        pad = 0;
        pad_char = ' ';

        if (c != '%') {
            cuoreterm_draw_char(global_term, c, global_fg);
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

        c = *format++;

        switch (c) {
            case 'd':
            case 'u':
            case 'x':
            case 'o':
                itoa(va_arg(args, int), buf,
                     (c == 'x') ? 16 : (c == 'o') ? 8 : 10);
                p = buf;
                break;

            case 'f': {
                double fval = va_arg(args, double);
                int int_part = (int)fval;
                double frac = fval - int_part;
                itoa(int_part, buf, 10);
                p = buf;
                while (*p) {
                    cuoreterm_draw_char(global_term, *p++, global_fg);
                    chars_written++;
                }
                cuoreterm_draw_char(global_term, '.', global_fg);
                chars_written++;
                frac *= 1000000; // 6 decimal places
                itoa((int)frac, buf, 10);
                p = buf;
                while (*p) {
                    cuoreterm_draw_char(global_term, *p++, global_fg);
                    chars_written++;
                }
                continue;
            }

            case 's':
                p = va_arg(args, char *);
                if (!p) p = "(null)";
                break;

            case 'c':
                cuoreterm_draw_char(global_term, va_arg(args, int), global_fg);
                chars_written++;
                continue;

            case 'p':
                itoa((uintptr_t)va_arg(args, void *), buf, 16);
                cuoreterm_draw_char(global_term, '0', global_fg);
                cuoreterm_draw_char(global_term, 'x', global_fg);
                chars_written += 2;
                p = buf;
                break;

            case 'm': // memory
                p = va_arg(args, char *);
                if (!p) p = "(null)";
                for (size_t i = 0; i < (size_t)(pad ? pad : 16); i++) {
                    cuoreterm_draw_char(global_term, p[i], global_fg);
                    chars_written++;
                }
                continue;

            default:
                cuoreterm_draw_char(global_term, c, global_fg);
                chars_written++;
                continue;
        }

        // print with padding
        int len = 0;
        for (p2 = p; *p2; p2++) len++;
        for (int i = len; i < pad; i++) {
            cuoreterm_draw_char(global_term, pad_char, global_fg);
            chars_written++;
        }
        while (*p) {
            cuoreterm_draw_char(global_term, *p++, global_fg);
            chars_written++;
        }
    }

    va_end(args);
    return chars_written;
}