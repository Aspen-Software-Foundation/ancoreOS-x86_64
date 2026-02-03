/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: math.c
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


#include "util/math.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t roundf(float number) {
    return (number >= 0.0f) ? (int)(number + 0.5f) : (int)(number - 0.5f);
}

double fabs(double x) {
    return x < 0.0 ? -x : x;
}

double fmod(double x, double y) {
    double result;
    asm (
        "1:\n\t"       // loop label
        "fprem1;"      // perform floating-point remainder operation
        "fnstsw %%ax;" // store the FPU status word in AX
        "sahf;"        // transfer status flags from AH to CPU flags
        "jp 1b;"       // if parity flag is set, jump back to label 1
        : "=t" (result)    // output: top of FPU stack goes into result
        : "0" (x), "u" (y) // inputs: 'x' is in the top of the FPU stack and 'y' in the other register
        : "ax", "cc"       // clobbered registers: AX and condition codes
    );
    return result;
}

float fmodf(float x, float y) {
    float result;
    asm (
        "1:\n\t"       // loop label
        "fprem1;"      // perform floating-point remainder operation
        "fnstsw %%ax;" // store the FPU status word in AX
        "sahf;"        // transfer status flags from AH to CPU flags
        "jp 1b;"       // if parity flag is set, jump back to label 1
        : "=t" (result)    // output: top of FPU stack goes into result
        : "0" (x), "u" (y) // inputs: 'x' is in the top of the FPU stack and 'y' in the other register
        : "ax", "cc"       // clobbered registers: AX and condition codes
    );
    return result;
}



double sin(double x) {
    double result;
    asm (
        "fldl %1;"    // Load x onto the FPU stack
        "fsin;"      // Compute sine of ST(0)
        "fstpl %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}


double cos(double x) {
    double result;
    asm (
        "fldl %1;"    // Load x onto the FPU stack
        "fcos;"      // Compute cosine of ST(0)
        "fstpl %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}

float sinf(float x) {
    float result;
    asm (
        "flds %1;"    // Load x onto the FPU stack
        "fsin;"      // Compute sine of ST(0)
        "fstps %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}


float cosf(float x) {
    float result;
    asm (
        "flds %1;"    // Load x onto the FPU stack
        "fcos;"      // Compute cosine of ST(0)
        "fstps %0;"   // Store the result into result and pop the FPU stack
        : "=m" (result)
        : "m" (x)
    );
    return result;
}

double tan(double x) {
    x = fmod(x + PI, 2 * PI);
    if (x < 0)
        x += 2 * PI;
    // Shift to range -PI to PI
    x = x - PI;
    double cos_x = cos(x);

    // Define a small threshold to check for values close to zero
    const double epsilon = 1e-10;

    if (fabs(cos_x) < epsilon) {
        // Handle the case where cosine is too close to zero
        if (sin(x) > 0)
            return 1000000;
        else
            return -1000000;
    }

    return sin(x) / cos_x;
}

float tanf(float x) {
    x = fmod(x + PI, 2 * PI);
    if (x < 0)
        x += 2 * PI;
    // Shift to range -PI to PI
    x = x - PI;
    float cos_x = cosf(x);

    // Define a small threshold to check for values close to zero
    const float epsilon = 1e-10;

    if (fabs(cos_x) < epsilon) {
        // Handle the case where cosine is too close to zero
        if (sinf(x) > 0)
            return 1000000;
        else
            return -1000000;
    }

    return sinf(x) / cos_x;
}


// black magic
double pow(double x, double y) {
    double out;
    asm(
            "fyl2x;"
            "fld %%st;"
            "frndint;"
            "fsub %%st,%%st(1);"
            "fxch;"
            "fchs;"
            "f2xm1;"
            "fld1;"
            "faddp;"
            "fxch;"
            "fld1;"
            "fscale;"
            "fstp %%st(1);"
            "fmulp;" : "=t"(out) : "0"(x),"u"(y) : "st(1)" );
    return out;
}



uint64_t __udivdi3(uint64_t n, uint32_t d) {
    if (d == 0) {INTERRUPT(0); return 0;} // avoid division by zero
    if (n < d) return 0;

    uint64_t q = 0;
    uint64_t r = n;
    int shift = 0;

    while ((d << shift) <= r && shift < 32) shift++;

    for (int i = shift - 1; i >= 0; i--) {
        if (r >= ((uint64_t)d << i)) {
            r -= ((uint64_t)d << i);
            q |= ((uint64_t)1 << i);
        }
    }

    return q;
}


uint64_t __umoddi3(uint64_t n, uint32_t d) {
    if (d == 0) {INTERRUPT(0); return 0;} // avoid division by zero
    if (n < d) return n;

    uint64_t r = n;
    int shift = 0;

    while ((d << shift) <= r && shift < 32) shift++;

    for (int i = shift - 1; i >= 0; i--) {
        if (r >= ((uint64_t)d << i)) {
            r -= ((uint64_t)d << i);
        }
    }

    return r;
}