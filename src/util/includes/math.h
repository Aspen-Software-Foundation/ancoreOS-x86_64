/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: math.h
    Description: Math module for the VNiX Operating System.
    Author: Mejd Almohammedi

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


#ifndef MATH_H
#define MATH_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "includes/util/util.h"

#define E 2.71828
#define PI 3.14159265358979323846264338327950


static inline uint32_t abs32(int32_t x) {
    return x < 0 ? -x : x;
}

static inline uint16_t abs16(int16_t x) {
    return x < 0 ? -x : x;
}

static inline uint8_t abs8(int8_t x) {
    return x < 0 ? -x : x;
}

// Helper functions for min and max.
static inline uint32_t min32(int a, int b) {
    return (a < b) ? a : b;
}
static inline uint32_t max32(int a, int b) {
    return (a > b) ? a : b;
}

static inline uint32_t log2u(uint32_t number) {
    uint32_t num=number, counter=0;
    for (; num!=1; counter++) {
        num >>= 1;
    }
    return counter;
}

uint32_t roundf(float number);
double fabs(double x);
double fmod(double x, double y);
float fmodf(float x, float y);
double sin(double x);
double cos(double x);
float sinf(float x);
float cosf(float x);
double tan(double x);
float tanf(float x);
double pow(double x, double y);

// Compute the x coordinate where the horizontal line at y intersects the edge (a, b).
static inline int compute_intersection_x(uint16_Vector2_t a, uint16_Vector2_t b, int y) {
    if (a.y == b.y)
        return a.x; // Horizontal edge - return one endpoint.
    float t = (float)(y - a.y) / (float)(b.y - a.y);
    return a.x + (int)((b.x - a.x) * t);
}

uint64_t __udivdi3(uint64_t n, uint32_t d);
uint64_t __umoddi3(uint64_t n, uint32_t d);

#endif // MATH_H