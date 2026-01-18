/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: kernel.c
    Description: The UEFI kernel for the Ancore Operating System.
    Author: Yazin Tantawi

    All components of the Ancore Operating System, except where otherwise noted, 
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

#include "drivers/terminal/src/cuoreterm.h"
#include "drivers/terminal/src/kfont.h"
#include "arch/limine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "includes/util/serial.h"
#include "includes/arch/x86_64/idt.h"
#include "includes/arch/x86_64/gdt.h"
#include "includes/arch/x86_64/isr.h"
#include "includes/memory/pmm.h"

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

struct terminal fb_term;

void kernel_main(void) {
    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
    terminal_set_instance(&fb_term, 0xFFFFFF);

    cuoreterm_init(
         &fb_term,
         (void *)fb->address,
         (uint32_t)fb->width,
         (uint32_t)fb->height,
         (uint32_t)fb->pitch,
         (uint32_t)fb->bpp,
         fb->red_mask_shift,
         fb->green_mask_shift,
         fb->blue_mask_shift,
         fb->red_mask_size,
         fb->green_mask_size,
         fb->blue_mask_size,
         iso10_f14_psf,
         8,
         14
    );
    

    serial_init();
    cuoreterm_clear(&fb_term);
    writestr(&fb_term, "Welcome to the Ancore Operating System,\x1b[#FF0000m made by Aspen\x1b[0m\n", 68);
    serial_write("\nWelcome to the Ancore Operating System, made by Aspen\n", 57);

    writestr(&fb_term, "\x1b[#7300FFm  [ debug ]\x1b[0m Successfully initalized kernel\n", 58);
    serial_write("[ debug ] Successfully initalized kernel\n", 46);

    GDT_Initialize();
    IDT_Initialize();
    pmm_init();
    enable_interrupts();
    ISR_Initialize();


    printf("\n=== testing malloc ===\n");
    
    // test 1: malloc
    char* test1 = malloc(100);
    if (test1 != NULL) {
        printf("malloc(100) succeeded: %p\n", test1);
        strcpy(test1, "Hello from malloc!");
        printf("String in malloc'd memory: %s\n", test1);
        free(test1);
        printf("free() completed\n");
    } else {
        printf("malloc(100) failed!\n");
    }
    
    // test 2: multiple allocations
    int* numbers = malloc(10 * sizeof(int));
    char* string = malloc(256);
    void* big = malloc(4096);
    
    if (numbers && string && big) {
        printf("Multiple allocations succeeded\n");
        
        for (int i = 0; i < 10; i++) {
            numbers[i] = i * 10;
        }
        printf(" Array: [%d, %d, %d, %d, %d]\n", 
                numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
        
        free(numbers);
        free(string);
        free(big);
        printf(" Freed all allocations\n");
    } else {
        printf(" Multiple allocations failed\n");
    }
    
    // test 3: calloc
    int* zeros = calloc(5, sizeof(int));
    if (zeros) {
        printf(" calloc succeeded\n");
        printf(" Values: [%d, %d, %d, %d, %d]\n", 
                zeros[0], zeros[1], zeros[2], zeros[3], zeros[4]);
        free(zeros);
    }
    
    
    printf("=== malloc tests complete ===\n\n");


    while (1);
}
