/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: isr.c
    Description: Interrupt Service Routine module for the Ancore Operating System
    Author: Mejd Almohammedi

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

// Despite the lack of legal requirements, this file is licensed by Nanobyte under the unlicense:
// https://github.com/nanobyte-dev/nanobyte_os/blob/master/LICENSE

#include "includes/arch/x86_64/isr.h"
#include "includes/arch/x86_64/idt.h"
#include "includes/arch/x86_64/gdt.h"
#include "includes/arch/x86_64/io.h"
#include <stdio.h>
#include "includes/util/serial.h"
#include <stddef.h>

ISRHandler_t g_ISRHandlers[256];

void ISR_InitializeGates();

void ISR_Initialize() {
    ISR_InitializeGates();
    for (int i = 0; i < 256; i++) {
        IDT_EnableGate(i);
    }

    kprintf("  [  OK  ] arch/x86_64/isr.c: ISR handlers initialized successfully.\n");
    serial_printf("[  OK  ] arch/x86_64/isr.c: ISR handlers initialized successfully.\n", 70);
}

void __attribute__((cdecl)) ISR_Handler(Registers_t *regs) {
    if (g_ISRHandlers[regs->interrupt] != NULL) {
        g_ISRHandlers[regs->interrupt](regs);

    } else if (regs->interrupt >= 32) {
        kprintf("Unhandled interrupt %d!\n", regs->interrupt);
        serial_printf("Unhandled interrupt %d!\n", regs->interrupt);
        
        kprintf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        serial_printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        kprintf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        serial_printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        kprintf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);
        serial_printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);
 
    } else {
        kprintf("Unhandled exception %d: %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        serial_printf("Unhandled exception %d: %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        kprintf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
        serial_printf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
                regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        kprintf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);
        serial_printf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
                regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        kprintf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);
        serial_printf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);

        kprintf("KERNEL PANIC!\n");
        serial_write("KERNEL PANIC!\n", 14);
        halt();
    }
}

void ISR_RegisterHandler(int interrupt, ISRHandler_t handler) {
    g_ISRHandlers[interrupt] = handler;
    IDT_EnableGate(interrupt);

}

