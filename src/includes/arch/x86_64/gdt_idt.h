//Author: Jerry Jhird
//Source: https://codeberg.org/jerryjhird/CuoreOS
//License: MPLv2.0

#include <stdint.h>

void halt(void);

// descriptor tables
// 1. global descriptor table

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init(void);

#ifdef __cplusplus

#endif

// 2. interupt descriptor table

#define IDT_ENTRIES 256
#define NUM_GPRS 15

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;       // bits 0-2 = IST, rest zero
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_init(void);