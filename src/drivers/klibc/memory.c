/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: memory.c
    Description: Memory module for the VNiX Operating System
    Author: Mejd Almohammedi (and some parts from Jerry Jhird)

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

// Some parts may be under the fullowing license:
// https://github.com/Prankiman/tetrisOS/blob/master/LICENSE

#include <stdint.h>
#include <stddef.h>
#include "stdbool.h"
#define ALIGN_UP(x, a) (((x) + (uintptr_t)((a)-1)) & ~((uintptr_t)((a)-1)))

uint8_t* heap_ptr = (uint8_t*)0x100000;  
uint8_t* heap_end = (uint8_t*)0x200000;


uint8_t *memset_pattern(void *ptr, const void *pattern, size_t pattern_size, size_t num) {
    uint8_t *p = (uint8_t *)ptr;
    const uint8_t *pat = (const uint8_t *)pattern;
    size_t pat_idx = 0;

    for (size_t i = 0; i < num; i++) {
        p[i] = pat[pat_idx];
        pat_idx = (pat_idx + 1) % pattern_size;
    }

    return p;
}

char *itoa(int32_t value, char *str, uint32_t base) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers for base 10
    if (value < 0 && base == 10) {
        isNegative = 1;
        value = -value;  // Make the value positive
    }

    // Process digits
    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';  // Convert to char
        value = value / base;
    }

    // Add negative sign if necessary
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';  // Null-terminate the string

    // Reverse the string to get the correct order
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

void *memset(void *ptr, int value, size_t num) {
    uint8_t *p = (uint8_t *)ptr;
    while (num--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int8_t memcmp(const void *ptr1, const void *ptr2, size_t n) {
    const uint8_t *s1 = (const uint8_t *)ptr1;
    const uint8_t *s2 = (const uint8_t *)ptr2;

    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (int8_t)(s1[i] - s2[i]);
        }
    }
    return 0;
}

void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    if (d < s) {
        return memcpy(dst, src, n);
    }

    for (size_t i = n; i > 0; i--) {
        d[i - 1] = s[i - 1];
    }

    return dst;
}

//Author: Jerry Jhird
//Source: https://codeberg.org/jerryjhird/CuoreOS/src/branch/master/src/libc/memory.c
//License: MPLv2.0
bool heap_can_alloc(size_t size) {
    uintptr_t ptr = (uintptr_t)heap_ptr;
    ptr = ALIGN_UP(ptr, 8);

    if (ptr + size > (uintptr_t)heap_end)
        return false;

    return true;
}


void* malloc(size_t size) {
    if (!heap_can_alloc(size))
        return NULL;

    uintptr_t ptr = (uintptr_t)heap_ptr;
    ptr = ALIGN_UP(ptr, 8); // 8-byte alignment
    if (ptr + size > (uintptr_t)heap_end) return NULL;

    heap_ptr = (uint8_t*)(ptr + size);
    return (void*)ptr;
}

void free(void* ptr, size_t size) {
    // free the last allocated block
    uintptr_t block = (uintptr_t)ptr;
    if (block + size == (uintptr_t)heap_ptr) {
        heap_ptr = (uint8_t*)block; // rewind heap pointer
    }
}

//End Code Attribution


int8_t memcmp_const(const void *ptr1, const uint8_t val, size_t n) {
    const uint8_t *s1 = (const uint8_t *)ptr1;

    for (size_t i = 0; i < n; i++) {
        if (s1[i] != val) {
            return (int8_t)(s1[i] - val);
        }
    }
    return 0;}

