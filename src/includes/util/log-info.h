/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: log-info.h
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

#ifndef LOG_INFO_H
#define LOG_INFO_H

#include "serial.h"
#include "drivers/terminal/src/cuoreterm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

extern struct terminal fb_term;

typedef enum result_t {
    Ok,        //green
    Warn,      //yellow
    Error,     //light red
    Fatal,     //dark red
    Debug, // borple
    ResultCount
} result_t;

extern const char* result_str[ResultCount];

#define COLOR_BLACK    "\x1b[#000000m"
#define COLOR_RED      "\x1b[#FF0000m"
#define COLOR_GREEN    "\x1b[#00FF00m"
#define COLOR_YELLOW   "\x1b[#FFFF00m"
#define COLOR_BLUE     "\x1b[#0000FFm"
#define COLOR_MAGENTA  "\x1b[#FF00FFm"
#define COLOR_CYAN     "\x1b[#00FFFFm"
#define COLOR_WHITE    "\x1b[#FFFFFFm"
#define COLOR_GRAY     "\x1b[#808080m"
#define COLOR_LIGHTRED    "\x1b[#FF6666m"
#define COLOR_LIGHTGREEN  "\x1b[#66FF66m"
#define COLOR_LIGHTYELLOW "\x1b[#FFFF66m"
#define COLOR_LIGHTBLUE   "\x1b[#6666FFm"
#define COLOR_LIGHTCYAN   "\x1b[#66FFFFm"
#define COLOR_LIGHTGRAY   "\x1b[#C0C0C0m"
#define COLOR_DARKRED     "\x1b[#CC0000m"  // for FATAL
#define COLOR_PURPLE      "\x1b[#7300FFm"
#define COLOR_RESET       "\x1b[0m"

static inline const char* get_status_color(result_t status) {
    switch (status) {
        case Ok:    return COLOR_GREEN;
        case Warn:  return COLOR_YELLOW;
        case Error: return COLOR_LIGHTRED;
        case Fatal: return COLOR_DARKRED;
        case Debug: return COLOR_PURPLE;
        default:    return COLOR_RESET;
    }
}

static inline void write_color(struct terminal *term, const char *color, const char *msg) {
    writestr(term, color, strlen(color));
    writestr(term, msg, strlen(msg));
    writestr(term, COLOR_RESET, strlen(COLOR_RESET));
}

static inline void color_print(const char *color, const char *fmt, ...) {
    char buffer[256];
    va_list args;
    printf("%s", color);
    
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    printf("%s", buffer);
    printf("%s", COLOR_RESET);
}

#define COLORED_MSG(term, color, msg) \
    do { \
        writestr(term, color, strlen(color)); \
        writestr(term, msg, strlen(msg)); \
        writestr(term, COLOR_RESET, strlen(COLOR_RESET)); \
    } while(0)

void log_to_terminal(result_t status, const char *from, const char *file, int line, const char *fmt, ...);

#define LOG(status, from, fmt, ...) \
    log_to_terminal(status, #from, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define SERIAL(status, from, fmt, ...) \
    serial_printf("[  %s  ] %s in %s at line %d: " fmt "", \
                  result_str[status], #from, __FILE__, __LINE__, ##__VA_ARGS__)

#endif