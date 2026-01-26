/*
            The AMPOS Operating System is
            copyright under the Aspen Software Foundation (And the file's corresponding developers)
            
            This project is licensed under the GNU Public License v2;
            For more information, visit "https://www.gnu.org/licenses/gpl-2.0.en.html"
            OR see to the "LICENSE" file.

*/

#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Read from PCI config space
uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
void pci_write(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t data);
void scan_pci_bus(uint8_t bus);
uint32_t pci_get_bar_size(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

// Start PCI enumeration for USB and storage devices
void start_pci_enumeration(void);

#endif // PCI_H