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
#include "drivers/terminal/src/flanterm.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum result_t {
    Ok,        // green
    Warn,      // yellow
    Error,     // light red
    Fatal,     // dark red
    Debug,     // purple
    ResultCount
} result_t;

extern const char* result_str[ResultCount];

// Standard ANSI color codes that Flanterm supports
#define COLOR_BLACK         "\033[30m"
#define COLOR_RED           "\033[31m"
#define COLOR_GREEN         "\033[32m"
#define COLOR_YELLOW        "\033[33m"
#define COLOR_BLUE          "\033[34m"
#define COLOR_MAGENTA       "\033[35m"
#define COLOR_CYAN          "\033[36m"
#define COLOR_WHITE         "\033[37m"
#define COLOR_GRAY          "\033[90m"
#define COLOR_LIGHTRED      "\033[91m"
#define COLOR_LIGHTGREEN    "\033[92m"
#define COLOR_LIGHTYELLOW   "\033[93m"
#define COLOR_LIGHTBLUE     "\033[94m"
#define COLOR_LIGHTMAGENTA  "\033[95m"
#define COLOR_LIGHTCYAN     "\033[96m"
#define COLOR_LIGHTGRAY     "\033[97m"

// Background colors (if needed)
#define COLOR_BG_BLACK      "\033[40m"
#define COLOR_BG_RED        "\033[41m"
#define COLOR_BG_GREEN      "\033[42m"
#define COLOR_BG_YELLOW     "\033[43m"

// Text styles
#define COLOR_BOLD          "\033[1m"
#define COLOR_DIM           "\033[2m"
#define COLOR_UNDERLINE     "\033[4m"
#define COLOR_BLINK         "\033[5m"
#define COLOR_REVERSE       "\033[7m"

#define COLOR_RESET         "\033[0m"

static inline const char* get_status_color(result_t status) {
    switch (status) {
        case Ok:    return COLOR_GREEN;
        case Warn:  return COLOR_YELLOW;
        case Error: return COLOR_LIGHTRED;
        case Fatal: return COLOR_BOLD COLOR_RED;  // Bold red for fatal
        case Debug: return COLOR_MAGENTA;
        default:    return COLOR_RESET;
    }
}

void log_to_terminal(result_t status, const char *from, const char *file, int line, const char *fmt, ...);

#define LOG(status, from, fmt, ...) \
    log_to_terminal(status, #from, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define SERIAL(status, from, fmt, ...) \
    serial_printf("[  %s  ] %s in %s at line %d: " fmt "", \
                  result_str[status], #from, __FILE__, __LINE__, ##__VA_ARGS__)

#endif