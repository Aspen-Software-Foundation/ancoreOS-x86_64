/*
            The AMPOS Operating System is
            copyright under the Aspen Software Foundation (And the file's corresponding developers)
            
            This project is licensed under the GNU Public License v2;
            For more information, visit "https://www.gnu.org/licenses/gpl-2.0.en.html"
            OR see to the "LICENSE" file.

*/

#include <stdint.h>
#include <pci/pci.h>
#include <arch/x86/io.h>
#include <storage/sata.h>
#include <memory/paging.h>
#include <lightshell/terminal.h>
#include <time/time.h>

extern uint32_t pid_rn;

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (1U << 31)
                     | ((uint32_t)bus << 16)
                     | ((uint32_t)device << 11)
                     | ((uint32_t)func << 8)
                     | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data) {
    uint32_t address = (1U << 31)
                     | ((uint32_t)bus << 16)
                     | ((uint32_t)device << 11)
                     | ((uint32_t)func << 8)
                     | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, data);
}


void scan_pci_device(uint8_t bus, uint8_t device) {
    uint32_t vendor_data = pci_read(bus, device, 0, 0x00);
    uint16_t vendor_id = vendor_data & 0xFFFF;
    if (vendor_id == 0xFFFF) return;

    uint32_t header_data = pci_read(bus, device, 0, 0x0C);
    uint8_t header_type = (header_data >> 16) & 0xFF;

    uint8_t func_limit = (header_type & 0x80) ? 8 : 1;

    for (uint8_t func = 0; func < func_limit; func++) {
        uint32_t vd = pci_read(bus, device, func, 0x00);
        uint16_t vid = vd & 0xFFFF;
        if (vid == 0xFFFF) continue;

        uint16_t did = (vd >> 16) & 0xFFFF;
        uint32_t class_data = pci_read(bus, device, func, 0x08);
        uint8_t class_code = (class_data >> 24) & 0xFF;
        uint8_t subclass = (class_data >> 16) & 0xFF;

        uint8_t prog_if    = (class_data >> 8) & 0xFF;

        if (class_code == 0x01) {
            const char* type = "Unknown Storage";

            if (subclass == 0x01) {
                type = "IDE";
            } else if (subclass == 0x04) {
                type = "RAID";
            } else if (subclass == 0x06) {
                if (prog_if == 0x01) {
                    type = "SATA AHCI";
                    uint32_t bar5 = pci_read(bus, device, func, 0x24);
                    uint32_t ahci_base = bar5 & 0xFFFFFFF0;

                    terminal_printf(" [%f] ./pci/pci.c: AHCI MMIO base address: 0x%x\n", get_time_ms_fp()/1000, ahci_base);
                    pid_rn = 7;
                    set_page_mapping(ahci_base, ahci_base, PAGE_MMIODEFAULT);
                    sata_search(ahci_base);
                } else {
                    type = "SATA (non-AHCI)";
                }
            }

            terminal_printf("[%f] ./pci/pci.c: %s Controller: bus=%u dev=%u func=%u vendor id=0x%x dev id=0x%x\n",
               get_time_ms_fp()/1000, type, bus, device, func, vid, did);
        } else if (class_code == 0x0C && subclass == 0x03) {
            terminal_printf("[%f] ./pci/pci.c: USB Controller: bus=%u dev=%u func=%u vendor=0x%x dev=0x%x\n", get_time_ms_fp()/1000, bus, device, func, vid, did);
        } else if (class_code == 0x06 && subclass == 0x04) {
            uint32_t bus_data = pci_read(bus, device, func, 0x18);
            uint8_t secondary_bus = (bus_data >> 8) & 0xFF;
            scan_pci_bus(secondary_bus); // recurse
        }
    }
}


void scan_pci_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        scan_pci_device(bus, device);
    }
}


void start_pci_enumeration() {
    uint32_t header_data = pci_read(0, 0, 0, 0x0C);
    uint8_t header_type = (header_data >> 16) & 0xFF;

    if ((header_type & 0x80) == 0) {
        scan_pci_bus(0);
    } else {
        for (uint8_t func = 0; func < 8; func++) {
            uint32_t vd = pci_read(0, 0, func, 0x00);
            uint16_t vid = vd & 0xFFFF;
            if (vid == 0xFFFF) continue;
            uint32_t bus_data = pci_read(0, 0, func, 0x18);
            uint8_t bus_num = (bus_data >> 8) & 0xFF;
            scan_pci_bus(bus_num);
        }
    }
}

uint32_t pci_get_bar_size(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t orig = pci_read(bus, device, func, offset);

    // write all 1's
    pci_write(bus, device, func, offset, 0xFFFFFFFF);

    // read back
    uint32_t size_mask = pci_read(bus, device, func, offset);

    // restore original
    pci_write(bus, device, func, offset, orig);

    // mask out low bits (type info)
    size_mask &= ~0xF;

    // size = ~(mask) + 1
    uint32_t size = (~size_mask) + 1;

    return size;
}
