/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: log-info.c
    Description: The log module for the VNiX Operating System.
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

#include "includes/log-info.h"
#include <stdarg.h>

extern struct flanterm_context *global_flanterm;

const char* result_str[ResultCount] = {
    [Ok]    = "OK",
    [Warn]  = "WARN",
    [Error] = "ERROR",
    [Fatal] = "FATAL",
    [Debug] = "DEBUG",
};

void log_to_terminal(result_t status, const char *from, const char *file, int line, const char *fmt, ...) {
    if (!global_flanterm) return;
    
    // build msg
    char message[512];
    char *ptr = message;
    
    // color
    const char *color = get_status_color(status);
    while (*color) *ptr++ = *color++;

    *ptr++ = '[';
    *ptr++ = ' ';
    *ptr++ = ' ';
    
    const char *status_str = result_str[status];
    while (*status_str) *ptr++ = *status_str++;
    
    *ptr++ = ' ';
    *ptr++ = ' ';
    *ptr++ = ']';
    
    // reset col
    const char *reset = COLOR_RESET;
    while (*reset) *ptr++ = *reset++;
    *ptr++ = ' ';
    
    // fn and location
    while (*from) *ptr++ = *from++;
    
    *ptr++ = ' ';
    *ptr++ = 'i';
    *ptr++ = 'n';
    *ptr++ = ' ';
    
    // file name
    const char *file_ptr = file;
    while (*file_ptr) *ptr++ = *file_ptr++;

    *ptr++ = ' ';
    *ptr++ = 'a';
    *ptr++ = 't';
    *ptr++ = ' ';
    *ptr++ = 'l';
    *ptr++ = 'i';
    *ptr++ = 'n';
    *ptr++ = 'e';
    *ptr++ = ' ';
    
    // ln num
    char line_str[16];
    itoa(line, line_str, 10);
    const char *line_ptr = line_str;
    while (*line_ptr) *ptr++ = *line_ptr++;
    
    *ptr++ = ':';
    *ptr++ = ' ';
    
    // process formatted message
    va_list args;
    va_start(args, fmt);
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd' || *fmt == 'i') {
                int val = va_arg(args, int);
                char num[16];
                itoa(val, num, 10);
                const char *num_ptr = num;
                while (*num_ptr) *ptr++ = *num_ptr++;
            } else if (*fmt == 'u') {
                unsigned int val = va_arg(args, unsigned int);
                char num[16];
                itoa((int)val, num, 10);
                const char *num_ptr = num;
                while (*num_ptr) *ptr++ = *num_ptr++;
            } else if (*fmt == 's') {
                char *str = va_arg(args, char*);
                if (str) {
                    while (*str) *ptr++ = *str++;
                } else {
                    const char *null_str = "(null)";
                    while (*null_str) *ptr++ = *null_str++;
                }
            } else if (*fmt == 'p') {
                void *p = va_arg(args, void*);
                *ptr++ = '0';
                *ptr++ = 'x';
                char hex[20];
                itoa((int)(uintptr_t)p, hex, 16);
                const char *hex_ptr = hex;
                while (*hex_ptr) *ptr++ = *hex_ptr++;
            } else if (*fmt == 'x' || *fmt == 'X') {
                unsigned int val = va_arg(args, unsigned int);
                char hex[16];
                itoa((int)val, hex, 16);
                const char *hex_ptr = hex;
                while (*hex_ptr) *ptr++ = *hex_ptr++;
            } else if (*fmt == '%') {
                *ptr++ = '%';
            } else {
                *ptr++ = '%';
                if (*fmt) *ptr++ = *fmt;
            }
        } else {
            *ptr++ = *fmt;
        }
        if (*fmt) fmt++;
    }
    va_end(args);

    *ptr = '\0';
    
    // write to flanterm
    flanterm_write(global_flanterm, message, ptr - message);
}