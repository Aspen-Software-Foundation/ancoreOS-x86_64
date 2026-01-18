/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: memory.c
    Description: Memory module for the Ancore Operating System
    Author: Mejd Almohammedi (and some parts from Jerry Jhird)

    All components of the Ancore Operating System, except where otherwise noted, 
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
#include <stdbool.h>
#include <stddef.h>

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

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        isNegative = 1;
        value = -value;  
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';  // Convert to char
        value = value / base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0'; 

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


void exit(int status) {

    (void)status;
    while(1) halt();
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

void free(void* ptr) {
    uintptr_t block = (uintptr_t)ptr;
    
    if (block == (uintptr_t)heap_ptr) {
        heap_ptr = (uint8_t*)block;
    }
}
//End Code Attribution




void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    if (ptr) {
        char* p = ptr;
        for (size_t i = 0; i < total; i++) {
            p[i] = 0;
        }
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    // TODO: Implement with heap allocator
    (void)ptr;
    (void)size;
    return NULL;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

long atol(const char* str) {
    long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

long long atoll(const char* str) {
    long long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

long strtol(const char* str, char** endptr, int base) {
    long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    if (base == 0) {
        if (*str == '0') {
            if (str[1] == 'x' || str[1] == 'X') {
                base = 16;
                str += 2;
            } else {
                base = 8;
                str++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }
    
    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else {
            break;
        }
        
        if (digit >= base) break;
        
        result = result * base + digit;
        str++;
    }
    
    if (endptr) {
        *endptr = (char*)str;
    }
    
    return sign * result;
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    return (unsigned long)strtol(str, endptr, base);
}

long long strtoll(const char* str, char** endptr, int base) {
    return (long long)strtol(str, endptr, base);
}

unsigned long long strtoull(const char* str, char** endptr, int base) {
    return (unsigned long long)strtol(str, endptr, base);
}

static void swap_bytes(char* a, char* b, size_t size) {
    while (size--) {
        char temp = *a;
        *a++ = *b;
        *b++ = temp;
    }
}

void* bsearch(const void* key, const void* base, size_t num, size_t size,
              int (*compar)(const void*, const void*)) {
    const char* pivot;
    int result;
    
    while (num > 0) {
        pivot = (const char*)base + (num / 2) * size;
        result = compar(key, pivot);
        
        if (result == 0) {
            return (void*)pivot;
        }
        
        if (result > 0) {
            base = pivot + size;
            num = num - (num / 2) - 1;
        } else {
            num = num / 2;
        }
    }
    
    return NULL;
}

void qsort(void* base, size_t num, size_t size,
           int (*compar)(const void*, const void*)) {
    if (num < 2) return;
    
    char* arr = (char*)base;
    char* pivot = arr + (num / 2) * size;
    char* left = arr;
    char* right = arr + (num - 1) * size;

    while (left <= right) {
        while (compar(left, pivot) < 0) left += size;
        while (compar(right, pivot) > 0) right -= size;
        
        if (left <= right) {
            if (left != right) {
                swap_bytes(left, right, size);
            }
            left += size;
            right -= size;
        }
    }
    

    if (arr < right) {
        qsort(arr, (right - arr) / size + 1, size, compar);
    }
    if (left < arr + (num - 1) * size) {
        qsort(left, (arr + (num - 1) * size - left) / size + 1, size, compar);
    }
}