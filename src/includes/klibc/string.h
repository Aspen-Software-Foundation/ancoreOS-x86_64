/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: string.h
    Description: String module for the VNiX Operating System
    Author: Mejd Almohammedi

    All components of the VNiX Operating System, except where otherwise noted, 
    are copyright of the Aspen Software Foundation (and the corresponding author(s)) and licensed under GPLv2 or later.
    For more information on the GNU Public License Version 2, please refer to the LICENSE file
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


#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>


#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define __MIN_IMPL(_x, _y, _xn, _yn) __extension__({\
        __typeof__(_x) _xn = (_x);\
        __typeof__(_y) _yn = (_y);\
        (_xn < _yn ? _xn : _yn);\
        })
#define MIN(_x, _y) __MIN_IMPL(_x, _y, CONCAT(__x, __COUNTER__), CONCAT(__y, __COUNTER__))

#define __MAX_IMPL(_x, _y, _xn, _yn) __extension__({\
        __typeof__(_x) _xn = (_x);\
        __typeof__(_y) _yn = (_y);\
        (_xn > _yn ? _xn : _yn);\
        })
#define MAX(_x, _y) __MAX_IMPL(_x, _y, CONCAT(__x, __COUNTER__), CONCAT(__y, __COUNTER__))

#define CLAMP(_x, _mi, _ma) (MAX(_mi, MIN(_x, _ma)))

// returns the highest set bit of x
// i.e. if x == 0xF, HIBIT(x) == 3 (4th index)
// WARNING: currently only works for up to 32-bit types
#define HIBIT(_x) (31 - __builtin_clz((_x)))

// returns the lowest set bit of x
#define LOBIT(_x)\
    __extension__({ __typeof__(_x) __x = (_x); HIBIT(__x & -__x); })

// returns _v with _n-th bit = _x
#define BIT_SET(_v, _n, _x) __extension__({\
        __typeof__(_v) __v = (_v);\
        (__v ^ ((-(_x) ^ __v) & (1 << (_n))));\
        })

#define PACKED __attribute__((packed))

#define _STR(x) #x
#define STR(x) _STR(x)

#ifndef asm
#define asm __asm__ volatile
#endif

#define CLI() asm ("cli")
#define STI() asm ("sti")
#define HALT() asm ("hlt")
#define INTERRUPT(n) asm("int %0" : : "N"(n))

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

#define ALIGN_UP(addr, align) (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))

#define unlikely(x) __builtin_expect(!!(x), 0)

typedef struct {
    uint8_t x;
    uint8_t y;
} uint8_Vector2_t;

typedef struct {
    uint16_t x;
    uint16_t y;
} uint16_Vector2_t;
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} uint8_Vector3_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} uint16_Vector3_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;



static inline color_t make_svga_color(uint8_t r, uint8_t g, uint8_t b) { 
    color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    return color;
}

static inline uint8_Vector2_t make_uint8_vector2(uint8_t x, uint8_t y) { 
    uint8_Vector2_t vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

static inline uint16_Vector2_t make_uint16_vector2(uint16_t x, uint16_t y) { 
    uint16_Vector2_t vec;
    vec.x = x;
    vec.y = y;
    return vec;
}

static inline uint8_Vector3_t make_uint8_vector3(uint8_t x, uint8_t y, uint8_t z) { 
    uint8_Vector3_t vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return vec;
}

static inline uint16_Vector3_t make_uint16_vector3(uint16_t x, uint16_t y, uint8_t z) { 
    uint16_Vector3_t vec;
    vec.x = x;
    vec.y = y;
    vec.z = z;
    return vec;
}




int8_t strncmp(const char *str1, const char *str2, size_t n);
size_t strlen(const char *str);
size_t strlcat(char *dst, const char *src, size_t size);
size_t strlcpy(char *dst, const char *src, size_t n);
const char *strchr(const char *str, char chr);
void strswap(char *str, char char1, char char2);
uint32_t strcount(char *str, char char1);


/* Append SRC onto DEST.  */
char *strcat (char *dest, const char *src);
/* Append no more than N characters from SRC onto DEST.  */
char *strncat (char *dest, const char *src, size_t n);

uint16_Vector3_t rotate_point_x(uint16_Vector3_t point, float angle);
uint16_Vector3_t rotate_point_y(uint16_Vector3_t point, float angle);

#endif // UTIL_H