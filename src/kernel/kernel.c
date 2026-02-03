/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: kernel.c
    Description: The UEFI kernel for the VNiX Operating System.
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

#include "../../limine/limine.h"
#include "drivers/terminal/src/flanterm.h"
#include "drivers/terminal/src/flanterm_backends/fb.h"
#include "drivers/memory/heapalloc/tlsf.h"
#include <stdio.h>
#include "drivers/terminal/src/flanterm.h"
#include <string.h>
#include <stdlib.h>
#include "util/includes/serial.h"
#include "includes/arch/x86_64/idt.h"
#include "includes/arch/x86_64/gdt.h"
#include "includes/arch/x86_64/isr.h"
#include "includes/memory/pmm.h"
#include "includes/memory/vmm.h"
#include "util/includes/log-info.h"
#include "includes/pic/apic/apic.h"
#include "includes/pic/apic/apic_irq.h"
#include "includes/shell/keyboard.h"
#include "includes/shell/shell.h"
#include "includes/storage/stinit.h"

// IMPORTANT: Define the global flanterm context that printf uses
extern struct flanterm_context *global_flanterm;

void __assert_fail(const char *expr, const char *file, int line, const char *func) {
    (void)expr;
    (void)file;
    (void)line;
    (void)func;
}

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

#define KERNEL_HEAP_SIZE 0x100000
static unsigned char kernel_heap[KERNEL_HEAP_SIZE] __attribute__((aligned(8)));
tlsf_t kernel_tlsf;


void init_heap() {
    kernel_tlsf = tlsf_create_with_pool(kernel_heap, KERNEL_HEAP_SIZE);
    printf("%sWelcome to the VNiX Operating System, made by%s %sAspen%s\n", 
       COLOR_CYAN, COLOR_RESET, COLOR_RED, COLOR_RESET);
    LOG(Debug, init_heap, "Heap initialized at %p\n", kernel_heap);
    SERIAL(Debug, init_heap, "Heap initialized at %p\n", kernel_heap);
}

void kernel_main(void) {
    struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
    
    // Initialize flanterm and assign to GLOBAL variable
    global_flanterm = flanterm_fb_init(
        NULL,                    // malloc function
        NULL,                    // free function
        fb->address,             // framebuffer address
        fb->width,               // width
        fb->height,              // height
        fb->pitch,               // pitch
        fb->red_mask_size,       // red_mask_size
        fb->red_mask_shift,      // red_mask_shift
        fb->green_mask_size,     // green_mask_size
        fb->green_mask_shift,    // green_mask_shift
        fb->blue_mask_size,      // blue_mask_size
        fb->blue_mask_shift,     // blue_mask_shift
        NULL,                    // canvas
        NULL,                    // ansi_colours
        NULL,                    // ansi_bright_colours
        NULL,                    // background
        NULL,                    // background_bright
        NULL,                    // font
        NULL,                    // font_width
        NULL,                    // font_height
        0,                       // font_spacing
        0,                       // font_scale_x
        1,                       // font_scale_y
        0,                       // margin
        0,                       // margin_gradient
        0,                       // default_bg
        0xFFFFFF                 // default_fg (white text)
    );
    
    init_heap();
    GDT_Initialize();
    IDT_Initialize();
    vmm_init();
    pmm_init();
    enable_interrupts();
    ISR_Initialize();
        APIC_IRQ_Initialize();
    keyboard_apic_init();
    storage_init();
        void vmm_test_mapping(void) {
    uint64_t virt = 0x1000000000;  // idfk virtual address
    uint64_t phys = 0x200000;      // idfk physical address


    map_page(virt, phys, PTE_PRESENT | PTE_WRITABLE);

    // verify that the mapping works by accessing the virtual address
    uint64_t *ptr = (uint64_t *)virt;
    *ptr = 0x1234567890ABCDEF;  

    uint64_t value = *ptr;
    if (value == 0x1234567890ABCDEF) {
        LOG(Ok, vmm_test_mapping, "VMM test passed: Virtual address maps correctly!\n");
        SERIAL(Ok, vmm_test_mapping, "VMM test passed: Virtual address maps correctly!\n");
    } else {
        LOG(Error, vmm_test_mapping, "VMM test failed: Virtual address mapping is incorrect\n");
        SERIAL(Error, vmm_test_mapping, "VMM test failed: Virtual address mapping is incorrect\n");
    }
}

void vmm_test_unmap(void) {
    uint64_t virt = 0x1000000000;  // a virtual address to map
    uint64_t phys = 0x200000;     

    map_page(virt, phys, PTE_PRESENT | PTE_WRITABLE);

    //write value
    uint64_t *ptr = (uint64_t *)virt;
    *ptr = 0x1234567890ABCDEF; 

    //verifies the value
    uint64_t value = *ptr; 
    if (value == 0x1234567890ABCDEF) {
        LOG(Ok, vmm_test_unmap, "VMM test passed: Virtual address mapped and accessed correctly\n");
        SERIAL(Ok, vmm_test_unmap, "Virtual address mapped and accessed correctly\n");
    } else {
        LOG(Error, vmm_test_unmap, "VMM test failed: Virtual address access failed before unmap\n");
        SERIAL(Error, vmm_test_unmap, "VMM test failed: Virtual address access failed before unmap\n");
    }

unmap_page(virt);

    //printf("Attempting to access unmapped page...\n");
    //ptr = (uint64_t *)virt;
    //*ptr = 0xDEADBEEF;  // this triggers a page fault. DO NOT UNCOMMENT THIS!!!!

}


vmm_test_mapping();
vmm_test_unmap();



    printf("\n=== testing malloc ===\n");
    serial_write("\n=== testing malloc ===\n", 24);
    
    // test 1: malloc
    char* test1 = malloc(100);
    if (test1 != NULL) {
        printf("malloc(100) succeeded: %p\n", test1);
        serial_printf("malloc(100) succeeded: %p\n", test1);
        strcpy(test1, "Hello from malloc!");
        printf("String in malloc'd memory: %s\n", test1);
        serial_printf("String in malloc'd memory: %s\n", test1);
        free(test1);
        printf("free() completed\n");
        serial_write("free() completed\n", 16);
    } else {
        printf("malloc(100) failed!\n");
        serial_write("malloc(100) failed!\n", 20);
    }
    
    // test 2: multiple allocations
    int* numbers = malloc(10 * sizeof(int));
    char* string = malloc(256);
    void* big = malloc(4096);
    
    if (numbers && string && big) {
        printf("Multiple allocations succeeded\n");
        serial_write("Multiple allocations succeeded\n", 33);
        
        for (int i = 0; i < 10; i++) {
            numbers[i] = i * 10;
        }
        printf(" Array: [%d, %d, %d, %d, %d]\n", 
                numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
        serial_printf(" Array: [%d, %d, %d, %d, %d]\n", 
                numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
        
        free(numbers);
        free(string);
        free(big);
        printf(" Freed all allocations\n");
        serial_write(" Freed all allocations\n", 24);
    } else {
        printf(" Multiple allocations failed\n");
        serial_write(" Multiple allocations failed\n", 30);
    }
    
    // test 3: calloc
    int* zeros = calloc(5, sizeof(int));
    if (zeros) {
        printf(" calloc succeeded\n");
        serial_write(" calloc succeeded\n", 18);
        printf(" Values: [%d, %d, %d, %d, %d]\n", 
                zeros[0], zeros[1], zeros[2], zeros[3], zeros[4]);
        serial_printf(" Values: [%d, %d, %d, %d, %d]\n",
                zeros[0], zeros[1], zeros[2], zeros[3], zeros[4]);
        free(zeros);
    }
    

    printf("=== malloc tests complete ===\n\n");
    serial_write("=== malloc tests complete ===\n\n", 33);
    shell_main();

    while (1);
}