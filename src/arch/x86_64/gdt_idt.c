#include <stdint.h>
#include "../../includes/arch/x86_64/io.h"
#include "../../includes/arch/x86_64/gdt_idt.h"
#include "../../includes/util/serial.h"
static struct gdt_entry gdt[3];
static struct gdt_ptr   gp;

void halt(void) {
    for (;;) __asm__ volatile("hlt");
}

// helper to fill a descriptor
static void gdt_set(
    int i, uint32_t base, uint32_t limit,
    uint8_t access, uint8_t gran)
{
    gdt[i].limit_low   = limit & 0xFFFF;
    gdt[i].base_low    = base & 0xFFFF;
    gdt[i].base_mid    = (base >> 16) & 0xFF;
    gdt[i].access      = access;
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].base_high   = (uint8_t)((base >> 24) & 0xFF);
}

void gdt_init(void) {
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (uint64_t)&gdt;
    gdt_set(0, 0, 0, 0, 0);
    gdt_set(1, 0, 0, 0x9A, 0xA0); // kernel code
    gdt_set(2, 0, 0, 0x92, 0xA0); // kernel data

    // load gdt
    __asm__ volatile (
        "lgdt %0\n\t"
        :
        : "m"(gp)
        : "memory"
    );

    __asm__ volatile (
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"

        "pushq $0x08\n\t"
        "leaq 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        :
        :
        : "rax", "memory"
    );
}

// IDT

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idtp;

// helper to fill a idt entry
static void idt_set_gate(int n, uint64_t handler, uint16_t sel, uint8_t flags, uint8_t ist) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = sel;
    idt[n].ist         = ist & 0x7;
    idt[n].type_attr   = flags;
    idt[n].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[n].zero        = 0;
}

// helper for printing registers to serial
__attribute__((unused))
void regtserial(/*uint64_t *regs*/) {
    char buf[32];
    const char *names[] = {
        "RAX", "RBX", "RCX", "RDX",
        "RSI", "RDI", "RBP", "R8",
        "R9",  "R10", "R11", "R12",
        "R13", "R14", "R15"
    };

    for (int i = 0; i < 15; i++) {
        serial_write(names[i], 3);
        serial_write(": 0x", 4);
        serial_write(buf, sizeof(void*)*2);
        serial_write("\n", 1);
    }
}

// register dumping for c environment
void dumpreg(void) {
    uint64_t regs[15];
    __asm__ volatile(
        "mov %%rax, %0\n\t"
        "mov %%rbx, %1\n\t"
        "mov %%rcx, %2\n\t"
        "mov %%rdx, %3\n\t"
        "mov %%rsi, %4\n\t"
        "mov %%rdi, %5\n\t"
        "mov %%rbp, %6\n\t"
        "mov %%r8,  %7\n\t"
        "mov %%r9,  %8\n\t"
        "mov %%r10, %9\n\t"
        "mov %%r11, %10\n\t"
        "mov %%r12, %11\n\t"
        "mov %%r13, %12\n\t"
        "mov %%r14, %13\n\t"
        "mov %%r15, %14\n\t"
        : "=m"(regs[0]), "=m"(regs[1]), "=m"(regs[2]), "=m"(regs[3]),
          "=m"(regs[4]), "=m"(regs[5]), "=m"(regs[6]), "=m"(regs[7]),
          "=m"(regs[8]), "=m"(regs[9]), "=m"(regs[10]), "=m"(regs[11]),
          "=m"(regs[12]), "=m"(regs[13]), "=m"(regs[14])
        :
        : "memory"
    );
    regtserial(regs);
}

__attribute__((naked))
void generic_exception_handler(void) {
    __asm__ volatile(
        "push %rax\n\t"
        "push %rbx\n\t"
        "push %rcx\n\t"
        "push %rdx\n\t"
        "push %rsi\n\t"
        "push %rdi\n\t"
        "push %rbp\n\t"
        "push %r8\n\t"
        "push %r9\n\t"
        "push %r10\n\t"
        "push %r11\n\t"
        "push %r12\n\t"
        "push %r13\n\t"
        "push %r14\n\t"
        "push %r15\n\t"

        "lea panic_str(%rip), %rdi\n\t"   // pointer to string
        "mov $18, %rsi\n\t"               // length
        "call serial_write\n\t"

        // dump registers
        "mov %rsp, %rdi\n\t"
        "call regtserial\n\t"

        // Halt
        "jmp haltjmp\n\t"

        "haltjmp:\n\t"
        "hlt\n\t"
        "jmp haltjmp\n\t"

        "panic_str: .ascii \"CPU EXCEPTION!!!!\\n\""
    );
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint64_t)&idt;

    // use the generic_exception handler for all exceptions
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint64_t)generic_exception_handler, 0x08, 0x8E, 0);
    }

    __asm__ volatile("lidt %0" : : "m"(idtp));
}
