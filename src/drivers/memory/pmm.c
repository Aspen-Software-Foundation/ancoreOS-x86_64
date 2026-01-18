#include <stddef.h>
#include "includes/memory/pmm.h"
#include "arch/limine.h"
#include <stdio.h>
#include "includes/util/serial.h"

struct PhysicalMemoryRegion *free_mem_head = NULL;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 3};

__attribute__((used, section(".limine_requests"))) 
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 3};

void pmm_init(void)
{

struct limine_memmap_response *response = memmap_request.response;
struct limine_hhdm_response *hhdm_response = hhdm_request.response; 
struct limine_memmap_entry **entries = response->entries;

    uint64_t entry_count = response->entry_count;

    for (uint64_t i = 0; i < entry_count; i++)
    {
        if (entries[i]->type == LIMINE_MEMMAP_USABLE)
        {
            uint64_t length = entries[i]->length;
            
            uint64_t base = ALIGN_UP(entries[i]->base, PAGE_SIZE);
            uint64_t end = ALIGN_DOWN((entries[i]->base + length), PAGE_SIZE);

            for (uint64_t current = base; current < end; current += PAGE_SIZE)
            {
                struct PhysicalMemoryRegion *region = (struct PhysicalMemoryRegion*)(current + hhdm_response->offset);
                region->base = current;
                region->next = free_mem_head;
                free_mem_head = region;
            }
        }
    }

    printf("  [  OK  ] drivers/memory/pmm.c: PMM initialized successfully\n");
    serial_write("[  OK  ] drivers/memory/pmm.c: PMM initialized successfully\n", 61);
}

uint64_t palloc(void)
{
    if (!free_mem_head) return (uint64_t)NULL;
    
    struct PhysicalMemoryRegion *region = free_mem_head;
    free_mem_head = free_mem_head->next;
    
    return region->base;
}

void pfree(uint64_t physc_addr)
{

    struct limine_hhdm_response *hhdm_response = hhdm_request.response; 
    struct PhysicalMemoryRegion *region = (struct PhysicalMemoryRegion*)(physc_addr + hhdm_response->offset);
    region->next = free_mem_head;
    region->base = physc_addr;
    free_mem_head = region;
}