/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: endian.h
    Description: Endian module for the VNiX Operating System
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

#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

static inline uint64_t be64toh(uint64_t be) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((be & 0xFF00000000000000) >> 56) |
           ((be & 0x00FF000000000000) >> 40) |
           ((be & 0x0000FF0000000000) >> 24) |
           ((be & 0x000000FF00000000) >> 8)  |
           ((be & 0x00000000FF000000) << 8)  |
           ((be & 0x0000000000FF0000) << 24) |
           ((be & 0x000000000000FF00) << 40) |
           ((be & 0x00000000000000FF) << 56);
#else
    return be;
#endif
}

static inline uint64_t le64toh(uint64_t le) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((le & 0xFF00000000000000) >> 56) |
           ((le & 0x00FF000000000000) >> 40) |
           ((le & 0x0000FF0000000000) >> 24) |
           ((le & 0x000000FF00000000) >> 8)  |
           ((le & 0x00000000FF000000) << 8)  |
           ((le & 0x0000000000FF0000) << 24) |
           ((le & 0x000000000000FF00) << 40) |
           ((le & 0x00000000000000FF) << 56);
#else
    return le;
#endif
}

static inline uint32_t be32toh(uint32_t be) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((be & 0xFF000000) >> 24) |
           ((be & 0x00FF0000) >> 8)  |
           ((be & 0x0000FF00) << 8)  |
           ((be & 0x000000FF) << 24);
#else
    return be;
#endif
}

static inline uint32_t le32toh(uint32_t le) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((le & 0xFF000000) >> 24) |
           ((le & 0x00FF0000) >> 8)  |
           ((le & 0x0000FF00) << 8)  |
           ((le & 0x000000FF) << 24);
#else
    return le;
#endif
}


static inline uint16_t be16toh(uint16_t be) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((be & 0xFF00) >> 8) |
           ((be & 0x00FF) << 8);
#else
    return be;
#endif
}

static inline uint16_t le16toh(uint16_t le) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((le & 0xFF00) >> 8) |
           ((le & 0x00FF) << 8);
#else
    return le;
#endif
}


static inline uint64_t bew64toh(uint64_t be) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((be & 0xFFFF000000000000) >> 48) |
           ((be & 0x0000FFFF00000000) >> 16) |
           ((be & 0x00000000FFFF0000) << 16) |
           ((be & 0x000000000000FFFF) << 48);
#else
    return be;
#endif
}

static inline uint64_t lew64toh(uint64_t le) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((le & 0xFFFF000000000000) >> 48) |
           ((le & 0x0000FFFF00000000) >> 16) |
           ((le & 0x00000000FFFF0000) << 16) |
           ((le & 0x000000000000FFFF) << 48);
#else
    return le;
#endif
}


static inline uint32_t bew32toh(uint32_t be) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((be & 0xFFFF0000) >> 16) |
           ((be & 0x0000FFFF) << 16);
#else
    return be;
#endif
}

static inline uint32_t lew32toh(uint32_t le) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ((le & 0xFFFF0000) >> 16) |
           ((le & 0x0000FFFF) << 16);
#else
    return le;
#endif
}

// The plot was lost right about here
static inline uint64_t lebbew64toh(uint64_t lbe) {
    return bew64toh(le64toh(lbe));
}


static inline uint64_t beblew64toh(uint64_t lbe) {
    return lew64toh(be64toh(lbe));
}


static inline uint32_t lebbew32toh(uint32_t lbe) {
    return bew32toh(le32toh(lbe));
}


static inline uint32_t beblew32toh(uint32_t lbe) {
    return lew32toh(be32toh(lbe));
}


#endif // ENDIAN_H