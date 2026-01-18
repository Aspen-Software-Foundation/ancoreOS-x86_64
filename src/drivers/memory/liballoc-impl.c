/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: liballoc-impl.c
    Description: Liballoc implementations for the Ancore Operating System
    Author: Yazin Tantawi

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

#include "includes/memory/pmm.h"
#include "arch/limine.h"
#include <stddef.h>
#include <stdio.h>

// hhdm_request is defined in pmm.c, we'll access it through extern
extern volatile struct limine_hhdm_request hhdm_request;

int liballoc_lock() {
    // TODO: implement spinlock when we have SMP/multithreading
    __asm__ volatile("cli");
    return 0;
}

int liballoc_unlock() {
    // TODO: release spinlock
    __asm__ volatile("sti");
    return 0;
}

// allocate 'pages' number of physical pages
// returns a virtual address in the higher half direct map (HHDM)
void* liballoc_alloc(int pages) {
    if (pages <= 0) return NULL;
    
    struct limine_hhdm_response *hhdm_response = hhdm_request.response;
    if (!hhdm_response) {
        printf("ERROR: HHDM response is NULL!\n");
        return NULL;
    }
    
    // allocate physical pages (not necessarily contiguous, but that's OK for liballoc)
    uint64_t first_page = palloc();
    if (first_page == 0) {
        printf("ERROR: palloc failed on first page!\n");
        return NULL;
    }
    
    // allocate remaining pages
    for (int i = 1; i < pages; i++) {
        uint64_t page = palloc();
        if (page == 0) {
            printf("ERROR: palloc failed at page %d of %d\n", i, pages);
            // if out of memory, free what we allocated
            for (int j = 0; j < i; j++) {
                pfree(first_page + (j * 0x1000));
            }
            return NULL;
        }
    }
    
    // convert to virtual address in HHDM and return
    void* virt_addr = (void*)(first_page + hhdm_response->offset);
    printf("liballoc_alloc: allocated %d pages, phys=%p, virt=%p\n", pages, (void*)first_page, virt_addr);
    return virt_addr;
}

// free 'pages' number of pages starting at ptr
int liballoc_free(void* ptr, int pages) {
    if (!ptr || pages <= 0) return -1;
    
    struct limine_hhdm_response *hhdm_response = hhdm_request.response;
    if (!hhdm_response) {
        printf("ERROR: HHDM response is NULL in liballoc_free!\n");
        return -1;
    }
    
    // convert virtual address back to physical
    uint64_t phys_addr = (uint64_t)ptr - hhdm_response->offset;
    
    printf("liballoc_free: freeing %d pages, virt=%p, phys=%p\n", pages, ptr, (void*)phys_addr);
    
    // Free each page
    for (int i = 0; i < pages; i++) {
        pfree(phys_addr + (i * 0x1000));
    }
    
    return 0;
}