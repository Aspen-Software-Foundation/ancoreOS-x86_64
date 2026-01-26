/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: string.c
    Description: String module for the VNiX Operating System
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


#include <string.h>
#include <stdlib.h>


int strncmp(const char *str1, const char *str2, size_t n) {
    while (n-- > 0) {
        if (*str1 != *str2) {
            return (unsigned char)(*str1) - (unsigned char)(*str2);
        }
        if (*str1 == '\0') {
            return 0;  // Return 0 if both strings are identical up to the null terminator
        }
        str1++;
        str2++;
    }
    return 0;  
}

size_t strlen(const char *str) {
    size_t l = 0;
    while (*str++ != 0) {
        l++;
    }
    return l;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return (unsigned char)(*str1) - (unsigned char)(*str2);
}

// SEE: https://opensource.apple.com/source/Libc/Libc-1158.30.7/string/strlcat.c.auto.html
size_t strlcat(char *dst, const char *src, size_t size) {
    const size_t sl = strlen(src),
          dl = strlen(dst);

    if (dl == size) {
        return size + sl;
    }

    if (sl < (size - dl)) {
        memcpy(dst + dl, src, sl + 1);
    } else {
        memcpy(dst + dl, src, size - dl - 1);
        dst[size - 1] = '\0';
    }

    return sl + dl;
}

size_t strlcpy(char *dst, const char *src, size_t n) {
    // copy as many bytes as can fit
    char *d = dst;
    const char *s = src;
    size_t size = n;

    while (--n > 0) {
        if ((*d++ = *s++) == 0) {
            break;
        }
    }

    // if we ran out of space, null terminate
    if (n == 0) {
        if (size != 0) {
            *d = 0;
        }

        // traverse the rest of s
        while (*s++);
    }

    return s - src - 1;
}

const char *strchr(const char *str, char chr) {
    if (str == NULL)
        return NULL;

    while (*str)
    {
        if (*str == chr)
            return str;

        ++str;
    }

    return NULL;
}

void strswap(char *str, char char1, char char2) {
    if (str == NULL)
        return;

    while (*str) {
        if (*str == char1) {
            *str = char2;
        }
        ++str;
    }
}

uint32_t strcount(char *str, char char1) {
    if (str == NULL)
        return 0;

    uint32_t count = 0;

    while (*str) {
        if (*str == char1) {
            ++count;
        }
        ++str;
    }
    return count;
}


/* strcat: append src onto dest.  Assumes dest has enough space. */
char *strcat(char *dest, const char *src)
{
    char *d = dest;
    /* find end of dest */
    while (*d != '\0') d++;
    /* copy src (including terminating NUL) */
    while ((*d++ = *src++) != '\0') {}
    return dest;
}

/* strncat: append at most n chars from src onto dest.
   The result is always NUL-terminated. */
char *strncat(char *dest, const char *src, size_t n)
{
    char *d = dest;
    /* find end of dest */
    while (*d != '\0') d++;

    while (n-- != 0 && *src != '\0') {
        *d++ = *src++;
    }

    *d = '\0';
    return dest;
}

int memcmp_const(const void *ptr1, const uint8_t val, size_t n) {
    const uint8_t *s1 = (const uint8_t *)ptr1;

    for (size_t i = 0; i < n; i++) {
        if (s1[i] != val) {
            return (int8_t)(s1[i] - val);
        }
    }
    return 0;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
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

void *memset(void *ptr, int value, size_t num) {
    uint8_t *p = (uint8_t *)ptr;
    while (num--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}


//Source: https://github.com/jakogut/mlibc/blob/master/src/string.c
//License: MIT, relicensed to GPLv2 for VNiX
char *strstr(const char *haystack, const char *needle)
{
	char *hpos = (char *)haystack;

	if (!*needle) return (char *)haystack;
	while (*hpos && strlen(hpos) >= strlen(needle)) {
		char *a = hpos, *b = (char *)needle;
		while (*a && *b && *a == *b) {
			a++;
			b++;
		}

		if (*b == '\0')
			return hpos;

		hpos++;
	}

	return NULL;
}

char *strpbrk(const char *haystack, const char *needle)
{
	const char *c1, *c2;
	for (c1 = haystack; *c1 != '\0'; ++c1) {
		for (c2 = needle; *c2 != '\0'; ++c2) {
			if (*c1 == *c2) {
				return (char *)c1;
			}
		}
	}
	return NULL;
}

static char *tok = NULL;
char *strtok(char *str, const char *delim)
{
	char *start, *end;
	start = (str != NULL ? str : tok);
	if (start == NULL)
		return NULL;

	end = strpbrk(start, delim);
	if (end) {
		*end = '\0';
		tok = end + 1;
	} else {
		tok = NULL;
	}
	return start;
}

int ipow(int base, int exp)
{
	int res = 1;
	for (int i = 0; i < exp; i++)
		res *= exp;

	return res;
}

//End code attribution