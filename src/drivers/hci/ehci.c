/*
 ehci.c
 EHCI + USB Mass Storage sector reader with proper async list management

 Author: the Aspen Software Foundation (And the file's corresponding developers) (with fixes)
*/

/*
            The AMPOS Operating System is
            copyright under the Aspen Software Foundation (And the file's corresponding developers)
            
            This project is licensed under the GNU Public License v2;
            For more information, visit "https://www.gnu.org/licenses/gpl-2.0.en.html"
            OR see to the "LICENSE" file.

*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "util/strops.h"
#include "memory/memops.h"
#include "memory/memory.h"
#include "memory/paging.h"
#include "lightshell/terminal.h"
#include "time/time.h"
#include "usb/ehci.h"


volatile struct ehci_regs *ehci = NULL;
/* Global persistent async list head */
static qh_t *async_list_head = NULL;
static uintptr_t async_list_head_phys = 0;
static uint32_t global_tag = 0x1000;

extern uint32_t pid_rn;

/* -------------------------------------------------------------------------
   Low-level EHCI control
   ------------------------------------------------------------------------- */
int ehci_stop_controller(void) {
    terminal_printf("EHCI: stopping controller\n");
    ehci->USBCMD &= ~EHCI_RUN_STOP;

    uint32_t start = get_time_ms();
    while (!(ehci->USBSTS & EHCI_USBSTS_HCHALTED)) {
        if (get_time_ms() - start > 500) {
            PRINT_EXTENDED2(FG_WARN, "", "[%f] ./usb/ehci.c: EHCI: stop timeout, STS=%08x CMD=%08x, EHCIBASE=%08x\n",
                 get_time_ms_fp()/1000, ehci->USBSTS, ehci->USBCMD, ehci);
            return -1;
        }
        udelay(1000);
    }

    PRINT_EXTENDED2(FG_OK, "","[%f] ./usb/ehci.c: EHCI: halted successfully, STS=%08x CMD=%08x\n",
         get_time_ms_fp()/1000, ehci->USBSTS, ehci->USBCMD);
    return 0;
}

int ehci_reset_controller(void) {
    terminal_printf("EHCI: resetting controller\n");

    // optional: ensure halted before reset
    if (!(ehci->USBSTS & EHCI_USBSTS_HCHALTED)) {
        terminal_printf("EHCI: controller not halted, stopping first\n");
        if (ehci_stop_controller() < 0) {
            terminal_printf("EHCI: failed to stop before reset\n");
            return -2;
        }
    }

    udelay(1000);

    ehci->USBCMD = EHCI_RESET;
    terminal_printf("EHCI: CMD after set reset=%08x\n", ehci->USBCMD);

    uint32_t start = get_time_ms();
    while (ehci->USBCMD & EHCI_RESET) {
        if (get_time_ms() - start > 1000) {
            terminal_printf("EHCI: reset timeout, CMD=%08x STS=%08x, EHCIBASE=%08x\n",
                  ehci->USBCMD, ehci->USBSTS, ehci);
            return -1;
        }
        udelay(1000);
    }

    terminal_printf("EHCI: reset complete, CMD=%08x STS=%08x\n",
          ehci->USBCMD, ehci->USBSTS);

    udelay(1000);
    return 0;
}


int ehci_init_async_list(void) {
    void *qh_page = phys_to_virt(pg_malloc(16, 4096));
    if (!qh_page) return -1;
    
    memset(qh_page, 0, 4096);
    async_list_head = (qh_t *)qh_page;
    async_list_head_phys = (uintptr_t)virt_to_phys(qh_page);
    
    /* Setup dummy QH with H-bit */
    async_list_head->horiz_link = (uint32_t)async_list_head_phys | 0x2;
    async_list_head->ep_char = 0x00000000;
    async_list_head->ep_cap = 0x40000000;  /* H-bit */
    async_list_head->curr_qtd = 0x1;
    async_list_head->overlay.next = 0x1;
    async_list_head->overlay.token = 0x40;
    
    ehci->ASYNCLISTADDR = (uint32_t)async_list_head_phys;
    phys_flush_cache(async_list_head, sizeof(qh_t));
    
    return 0;
}

int ehci_init_simple(void) {
    terminal_printf("EHCI init start\n");

    if (ehci_stop_controller() < 0) {
        terminal_printf("EHCI stop failed\n");
        return -1;
    }

    terminal_printf("EHCI stopped\n");

    if (ehci_reset_controller() < 0) {
        terminal_printf("EHCI reset failed\n");
        return -2;
    }

    terminal_printf("EHCI reset ok\n");

    if (ehci_init_async_list() < 0) {
        terminal_printf("EHCI async list init failed\n");
        return -3;
    }

    terminal_printf("Async list ok\n");

    ehci->PERIODICLISTBASE = 0;
    ehci->USBINTR = 0;
    ehci->USBCMD |= (EHCI_ASYNC_EN | EHCI_RUN_STOP);

    terminal_printf("EHCI controller run command issued\n");

    uint32_t start = get_time_ms();
    while (!(ehci->USBSTS & (1 << 15))) {
        if (get_time_ms() - start > 100) {
            terminal_printf("Timeout waiting for EHCI HCHalted bit clear\n");
            break;
        }
        udelay(100);
    }

    uint32_t cmd = ehci->USBCMD;
    uint32_t sts = ehci->USBSTS;
    terminal_printf("EHCI USBCMD=0x%x, USBSTS=0x%x\n", cmd, sts);

    terminal_printf("EHCI init done\n");
    return 0;
}


/* -------------------------------------------------------------------------
   Port management
   ------------------------------------------------------------------------- */

int ehci_port_connected(int port_num) {
    if (port_num < 0 || port_num >= 16) return 0;
    return (ehci->PORTSC[port_num] & PORTSC_CONNECTED) ? 1 : 0;
}

int ehci_reset_port(int port_num) {
    if (port_num < 0 || port_num >= 16) return -1;
    
    volatile uint32_t *portsc = &ehci->PORTSC[port_num];
    
    if (!(*portsc & PORTSC_CONNECTED)) return -2;
    
    if (*portsc & PORTSC_ENABLED) {
        *portsc &= ~PORTSC_ENABLED;
        udelay(1000);
    }
    
    *portsc |= PORTSC_RESET;
    udelay(50000);
    *portsc &= ~PORTSC_RESET;
    
    uint32_t start = get_time_ms();
    while (1) {
        if (*portsc & PORTSC_ENABLED) break;
        
        if (!(*portsc & PORTSC_RESET)) {
            *portsc |= PORTSC_OWNER;
            return -3;
        }
        
        if (get_time_ms() - start > 500) return -4;
        udelay(1000);
    }
    
    *portsc |= (PORTSC_CONNECT_CHANGE | PORTSC_ENABLE_CHANGE);
    udelay(10000);
    
    return 0;
}

void ehci_power_on_ports(int num_ports) {
    for (int i = 0; i < num_ports && i < 16; i++) {
        ehci->PORTSC[i] |= PORTSC_POWER;
    }
    udelay(20000);
}

int ehci_find_and_reset_device(int *port_num_out) {
    for (int i = 0; i < 16; i++) {
        if (ehci_port_connected(i)) {
            int rc = ehci_reset_port(i);
            if (rc == 0) {
                *port_num_out = i;
                return 0;
            }
        }
    }
    return -1;
}

/* -------------------------------------------------------------------------
   Transfer functions
   ------------------------------------------------------------------------- */

int ehci_submit_bulk_simple(uint8_t devaddr, uint8_t ep_addr, void *data_vaddr, 
                           size_t len, int dir_in, uint32_t timeout_ms) {
    if (!async_list_head) return -1;
    
    void *page_alloced = pg_malloc(16, 4096);
    if (!page_alloced) return -1;
    
    uintptr_t page_idx = 0;
    qh_t *qh = (qh_t *)(page_alloced + page_idx);
    page_idx += ALIGN_UP(sizeof(qh_t), 64);
    qtd_t *qtd = (qtd_t *)(page_alloced + page_idx);

    memset(qh, 0, sizeof(*qh));
    memset(qtd, 0, sizeof(*qtd));

    uintptr_t qh_phys = (uintptr_t)virt_to_phys(qh);
    uintptr_t qtd_phys = (uintptr_t)virt_to_phys(qtd);
    uintptr_t buf_phys = (uintptr_t)virt_to_phys(data_vaddr);
    (void)qtd_phys;

    qtd->next = 0x1;
    qtd->alt_next = 0x1;
    
    uint32_t pid = dir_in ? PID_IN : PID_OUT;
    qtd->token = (1u << 7) | (len << 16) | (3 << 10) | (pid << 8);
    qtd->buffer[0] = (uint32_t)buf_phys;
    
    for (size_t i = 1; i < 5 && (len > (i * 4096)); i++) {
        qtd->buffer[i] = (uint32_t)(buf_phys + (i * 4096)) & ~0xFFF;
    }

    uint8_t ep_num = ep_addr & 0x0F;
    uint16_t max_packet = 512;
    
    qh->ep_char = (devaddr << 0) | (ep_num << 8) | (max_packet << 16) | (2 << 12);
    qh->ep_cap = (1 << 30);
    qh->curr_qtd = 0;
    qh->overlay = *qtd;
    
    qh->horiz_link = async_list_head->horiz_link;
    phys_flush_cache(qh, sizeof(*qh));
    phys_flush_cache(qtd, sizeof(*qtd));
    phys_flush_cache(data_vaddr, len);
    
    async_list_head->horiz_link = (uint32_t)qh_phys | 0x2;
    phys_flush_cache(async_list_head, 64);
    
    uint32_t start = get_time_ms();
    while (1) {
        phys_invalidate_cache(qh, sizeof(*qh));
        uint32_t overlay_token = qh->overlay.token;
        
        if (!(overlay_token & (1u << 7))) break;
        
        if (overlay_token & 0x7E) {
            async_list_head->horiz_link = qh->horiz_link;
            phys_flush_cache(async_list_head, 64);
            pg_free(16, (void *)page_alloced);
            return -3;
        }
        
        if (get_time_ms() - start > timeout_ms) {
            async_list_head->horiz_link = qh->horiz_link;
            phys_flush_cache(async_list_head, 64);
            pg_free(16, (void *)page_alloced);
            return -2;
        }
        
        udelay(100);
    }
    
    async_list_head->horiz_link = qh->horiz_link;
    phys_flush_cache(async_list_head, 64);
    
    if (dir_in) {
        phys_invalidate_cache(data_vaddr, len);
    }
    
    pg_free(16, (void *)page_alloced);
    return 0;
}

// Assume EHCI MMIO regs are already mapped
#define EHCI_USBCMD    ((volatile uint32_t *)(ehci_base + 0x00))
#define EHCI_ASYNCLISTADDR ((volatile uint32_t *)(ehci_base + 0x18))
#define EHCI_USBSTS    ((volatile uint32_t *)(ehci_base + 0x04))

// Flush the async schedule by telling EHCI to stop and wait for idle
static int ehci_flush_async(void) {
    // Stop async schedule
    ehci->USBCMD &= ~(1 << 5); // Clear AsyncEnable bit
    udelay(50);

    uint32_t timeout_ctr = 10000;
    // Wait until Async Schedule Status bit is 0
    while (ehci->USBSTS & (1 << 15) && timeout_ctr--) // ASYNC_SCHEDULE_STATUS
        udelay(10);

    if (ehci->USBSTS & (1 << 15)) {
        return -1;
    }
    return 0;
}

// Start async schedule
static void ehci_run_async(void) {
    ehci->ASYNCLISTADDR = (uint32_t)async_list_head;
    udelay(20);
    ehci->USBCMD |= (1 << 5); // AsyncEnable
    udelay(20);
}

/* -------------------------------------------------------------------------
   USB Control Transfer Implementation
   ------------------------------------------------------------------------- */
int usb_control_transfer(uint8_t devaddr, usb_ctrl_setup_t *setup,
                         void *data_buf, uint16_t data_len)
{
    if (!setup) {
        terminal_printf("usb_control_transfer: setup ptr is NULL\n");
        return -1;
    }

    // Allocate one page (rounded up to 4096) for QH + two qTDs (setup + optional data + status)
    const size_t allocSize = 4096;
    void *mem = pg_malloc(78, allocSize);
    if (!mem) {
        terminal_printf("usb_control_transfer: pg_malloc failed\n");
        return -2;
    }
    memset(mem, 0, allocSize);

    // Physical address of mem region
    uintptr_t phys_mem = (uintptr_t)virt_to_phys(mem);  // you need to implement this conversion

    // Layout: mem at base, setup qTD at base+64, data qTD at base+128, status qTD at base+192
    qh_t   *qh         = (qh_t *)          mem;
    qtd_t  *qtd_setup  = (qtd_t *)((uint8_t *)mem + 64);
    qtd_t  *qtd_data   = (qtd_t *)((uint8_t *)mem + 128);
    qtd_t  *qtd_status = (qtd_t *)((uint8_t *)mem + 192);

    // Zero everything
    memset(qh,         0, sizeof(qh_t));
    memset(qtd_setup,  0, sizeof(qtd_t));
    memset(qtd_data,   0, sizeof(qtd_t));
    memset(qtd_status, 0, sizeof(qtd_t));

    // --- Setup qTD ---
    qtd_setup->next   = (uint32_t)(phys_mem + 128);  // physical pointer to data qTD
    qtd_setup->token  = (8 << 16) /* total 8 bytes */ 
                      /* your token format: SETUP stage flagged */;  
    memcpy(qtd_setup->buffer, setup, sizeof(*setup));

    // --- Data qTD (if any) ---
    if (data_buf && data_len) {
        qtd_data->next  = (uint32_t)(phys_mem + 192); // link to status qTD
        qtd_data->token = ((data_len & 0x7FFF) << 16)
                         /* + PID based on bmRequestType IN or OUT */
                         /* + active bit */;
        uintptr_t buf = (uintptr_t)data_buf;
        for (int i = 0; i < 5; i++) {
            qtd_data->buffer[i] = (uint32_t)buf;
            // each pointer typically page align
            buf += 4096;
        }
    } else {
        // no data stage: link setup qTD directly to status
        qtd_setup->next = (uint32_t)(phys_mem + 192);
    }

    // --- Status qTD (zero length) ---
    qtd_status->next  = 1; // terminate
    qtd_status->token = (0 << 16) /* total 0 bytes */
                       /* + PID opposite data direction (if data) or IN */
                       /* + active bit */;

    // --- QH setup ---
    qh->curr_qtd  = 0;
    qh->overlay   = *qtd_setup;  // copy setup qTD into overlay
    qh->horiz_link = 1;          // terminate list
    qh->ep_char   = ((uint32_t)devaddr << 8) | (64 << 16);  // assume max packet size 64
    qh->ep_cap    = 0;

    // Flush caches before handing to hardware
    phys_flush_cache(mem, allocSize);

    // Insert QH into async schedule
    qh_t *old_head = async_list_head;
    async_list_head = qh;

    // Start schedule
    if (ehci_flush_async() != 0) {
        terminal_printf("usb_control_transfer: ehci_flush_async failed\n");
        async_list_head = old_head;
        pg_free(78, mem);
        return -3;
    }
    ehci_run_async();

    // Wait for completion with timeout
    uint64_t start = get_time_ms();
    while ((qh->overlay.token & (1 << 7)) /* active bit */) {
        if ((get_time_ms() - start) > 5000) {  // 5 second timeout
            terminal_printf("usb_control_transfer: timeout\n");
            async_list_head = old_head;
            pg_free(78, mem);
            return -4;
        }
        udelay(100);
    }

    // Invalidate cache so CPU sees updated token & buffer
    phys_invalidate_cache(mem, allocSize);

    // Compute actual transferred length
    int actual = data_len;
    if (data_len && !(qtd_data->token & 15<<3)) {
        actual = data_len - ((qtd_data->token >> 16) & 0x7FFF);
    }

    // Cleanup
    async_list_head = old_head;
    pg_free(78, mem);

    return actual;
}







/* -------------------------------------------------------------------------
   Helper control transfer functions
   ------------------------------------------------------------------------- */
int usb_get_device_descriptor(uint8_t devaddr, void *desc_buf, uint16_t len) {
    usb_ctrl_setup_t setup = {
        .bmRequestType = 0x80,
        .bRequest = USB_REQ_GET_DESCRIPTOR,
        .wValue = (0x01 << 8),
        .wIndex = 0,
        .wLength = len
    };
    return usb_control_transfer(devaddr, &setup, desc_buf, len);
}

int usb_get_config_descriptor(uint8_t devaddr, void *desc_buf, uint16_t len) {
    usb_ctrl_setup_t setup = {
        .bmRequestType = 0x80,
        .bRequest = USB_REQ_GET_DESCRIPTOR,
        .wValue = (0x02 << 8),
        .wIndex = 0,
        .wLength = len
    };
    return usb_control_transfer(devaddr, &setup, desc_buf, len);
}


int usb_set_address(uint8_t new_addr) {
    usb_ctrl_setup_t setup = {
        .bmRequestType = 0x00,
        .bRequest = USB_REQ_SET_ADDRESS,
        .wValue = new_addr,
        .wIndex = 0,
        .wLength = 0
    };
    int rc = usb_control_transfer(0, &setup, NULL, 0);
    if (rc == 0) {
        udelay(2000);
    }
    terminal_printf("RCL: %u\n", rc);
    return rc;
}

int usb_set_configuration(uint8_t devaddr, uint8_t config_value) {
    usb_ctrl_setup_t setup = {
        .bmRequestType = 0x00,
        .bRequest = USB_REQ_SET_CONFIGURATION,
        .wValue = config_value,
        .wIndex = 0,
        .wLength = 0
    };
    return usb_control_transfer(devaddr, &setup, NULL, 0);
}

int usb_msd_reset(uint8_t devaddr, uint8_t iface_num) {
    usb_ctrl_setup_t setup = {
        .bmRequestType = 0x21,
        .bRequest = 0xFF,
        .wValue = 0,
        .wIndex = iface_num,
        .wLength = 0
    };
    return usb_control_transfer(devaddr, &setup, NULL, 0);
}

/* -------------------------------------------------------------------------
   Device enumeration with descriptor parsing
   ------------------------------------------------------------------------- */

int find_mass_storage_device(usb_device_t *out_dev) {
    void *desc_buf = pg_malloc(19, 4096);
    if (!desc_buf) return -1;
    terminal_printf("1 \n");
    pid_rn=18;
    memset(desc_buf, 0, 4096);
    
    /* Get device descriptor (address 0) */
    int res=usb_get_device_descriptor(0, desc_buf, 18);
    if (res != 0) {
        pg_free(19, (void *)desc_buf);
        return -100-res;
    }
    terminal_printf("0x%x-%x-%x-%x", desc_buf);
    terminal_printf("2 \n");
    pid_rn=19;
    
    /* Set address to 1 */
    if (usb_set_address(1) != 0) {
        pg_free(19, (void *)desc_buf);
        return -3;
    }
    terminal_printf("3 \n");
    pid_rn=20;
    
    out_dev->address = 1;
    
    /* Get configuration descriptor */
    if (usb_get_config_descriptor(1, desc_buf, 9) != 0) {
        pg_free(19, (void *)desc_buf);
        return -4;
    }
    terminal_printf("4 \n");
    pid_rn=21;
    
    usb_config_descriptor_t *cfg = (usb_config_descriptor_t *)desc_buf;
    uint16_t total_len = cfg->wTotalLength;
    
    if (total_len > 1024) total_len = 1024;
    
    /* Get full configuration */
    if (usb_get_config_descriptor(1, desc_buf, total_len) != 0) {
        pg_free(19, (void *)desc_buf);
        return -5;
    }
    terminal_printf("5 \n");
    pid_rn=22;
    
    out_dev->config_value = cfg->bConfigurationValue;
    
    /* Parse descriptors to find mass storage interface */
    uint8_t *ptr = (uint8_t *)desc_buf + 9;
    uint8_t *end = (uint8_t *)desc_buf + total_len;
    int found_msd = 0;

    terminal_printf("Start 0x%x, end 0x%x", ptr, end);
    
    while (ptr < end) {
        uint8_t len = ptr[0];
        uint8_t type = ptr[1];

        if (len < 2) {
            terminal_printf("Bad descriptor length %u at %p\n", len, ptr);
            break;
        }

        if (type == 0x04 && len >= sizeof(usb_interface_descriptor_t)) {
            usb_interface_descriptor_t *iface = (usb_interface_descriptor_t *)ptr;
            terminal_printf("Interface #%u: class=%02x subclass=%02x proto=%02x\n",
                            iface->bInterfaceNumber,
                            iface->bInterfaceClass,
                            iface->bInterfaceSubClass,
                            iface->bInterfaceProtocol);

            if (iface->bInterfaceClass == USB_CLASS_MASS_STORAGE &&
                iface->bInterfaceSubClass == USB_SUBCLASS_SCSI &&
                iface->bInterfaceProtocol == USB_PROTOCOL_BULK_ONLY) {
                out_dev->iface_number = iface->bInterfaceNumber;
                found_msd = 1;
                terminal_printf("  -> Found Mass Storage interface\n");
            }

        } else if (type == 0x05 && len >= sizeof(usb_endpoint_descriptor_t)) {
            usb_endpoint_descriptor_t *ep = (usb_endpoint_descriptor_t *)ptr;
            terminal_printf("  Endpoint addr=%02x attr=%02x maxpkt=%u\n",
                            ep->bEndpointAddress,
                            ep->bmAttributes,
                            ep->wMaxPacketSize);

            if (found_msd && (ep->bmAttributes & 0x03) == 0x02) { /* Bulk endpoint */
                if (ep->bEndpointAddress & 0x80) {
                    out_dev->bulk_in.addr = ep->bEndpointAddress;
                    out_dev->bulk_in.max_packet = ep->wMaxPacketSize;
                    out_dev->bulk_in.attrs = ep->bmAttributes;
                    terminal_printf("    -> Bulk IN endpoint registered\n");
                } else {
                    out_dev->bulk_out.addr = ep->bEndpointAddress;
                    out_dev->bulk_out.max_packet = ep->wMaxPacketSize;
                    out_dev->bulk_out.attrs = ep->bmAttributes;
                    terminal_printf("    -> Bulk OUT endpoint registered\n");
                }
            }

        } else {
            terminal_printf("Other descriptor type=%02x len=%u\n", type, len);
        }

        ptr += len;
    }

    pg_free(19, (void *)desc_buf);

    if (!found_msd) {
        terminal_printf("No Mass Storage interface found.\n");
        return -6;
    }

    
    /* Set configuration */
    if (usb_set_configuration(out_dev->address, out_dev->config_value) != 0) {
        return -7;
    }
    
    udelay(100000);
    
    /* Perform Bulk-Only Mass Storage Reset */
    usb_msd_reset(out_dev->address, out_dev->iface_number);
    udelay(100000);
    
    return 0;
}

/* -------------------------------------------------------------------------
   Mass Storage BOT functions
   ------------------------------------------------------------------------- */


int msd_send_cbw(usb_device_t *dev, void *cbw_buf) {
    return ehci_submit_bulk_simple(dev->address, dev->bulk_out.addr, 
                                   cbw_buf, 31, 0, 1000);
}

int msd_receive_csw(usb_device_t *dev, void *csw_buf) {
    return ehci_submit_bulk_simple(dev->address, dev->bulk_in.addr, 
                                   csw_buf, 13, 1, 1000);
}

int msd_read_sector(usb_device_t *dev, uint32_t lba, void *buf_vaddr) {
    void *page_alloced = pg_malloc(16, 4096);
    if (!page_alloced) return -1;
    
    uintptr_t page_sharing_index = 0;
    struct CBW *cbw = (struct CBW *)(page_alloced + page_sharing_index);
    page_sharing_index += ALIGN_UP(sizeof(struct CBW), 16);
    struct CSW *csw = (struct CSW *)(page_alloced + page_sharing_index);

    memset(cbw, 0, sizeof(*cbw));
    cbw->signature = CBW_SIGNATURE;
    cbw->tag = ++global_tag;
    cbw->data_transfer_length = 512;
    cbw->flags = 0x80;
    cbw->lun = 0;
    cbw->cb_length = 10;
    cbw->cb[0] = SCSI_READ10;
    cbw->cb[1] = 0;
    cbw->cb[2] = (lba >> 24) & 0xFF;
    cbw->cb[3] = (lba >> 16) & 0xFF;
    cbw->cb[4] = (lba >> 8) & 0xFF;
    cbw->cb[5] = (lba >> 0) & 0xFF;
    cbw->cb[7] = 0;
    cbw->cb[8] = 1;

    phys_flush_cache(cbw, sizeof(*cbw));
    if (msd_send_cbw(dev, cbw) != 0) {
        pg_free(16, (void *)page_alloced);
        return -2;
    }

    if (ehci_submit_bulk_simple(dev->address, dev->bulk_in.addr, 
                               buf_vaddr, 512, 1, 1000) != 0) {
        pg_free(16, (void *)page_alloced);
        return -3;
    }

    memset(csw, 0, sizeof(*csw));
    if (msd_receive_csw(dev, csw) != 0) {
        pg_free(16, (void *)page_alloced);
        return -4;
    }

    phys_invalidate_cache(csw, sizeof(*csw));
    if (csw->signature != CSW_SIGNATURE) {
        pg_free(16, (void *)page_alloced);
        return -5;
    }
    if (csw->tag != cbw->tag) {
        pg_free(16, (void *)page_alloced);
        return -6;
    }
    if (csw->status != 0) {
        pg_free(16, (void *)page_alloced);
        return -7;
    }

    pg_free(16, (void *)page_alloced);
    return 0;
}

/* -------------------------------------------------------------------------
   Example usage
   ------------------------------------------------------------------------- */

int read_sector0_example(void) {
    uint16_t ports;
    uintptr_t mmio_base;
    void *bar0_virt = NULL;
    
    void *mmio = pci_map_ehci_mmio(&ports, &mmio_base, bar0_virt);
    if (!mmio) return -1;
    ehci = (volatile struct ehci_regs *)mmio;

    if (ehci_init_simple() != 0) return -2;
    
    ehci_power_on_ports(ports);
    
    int port_num = -1;
    if (ehci_find_and_reset_device(&port_num) != 0) return -3;

    usb_device_t dev;
    if (find_mass_storage_device(&dev) != 0) return -4;

    void *sector_buf = pg_malloc(18, 512);
    if (!sector_buf) return -5;
    memset(sector_buf, 0, 512);

    int rc = msd_read_sector(&dev, 0, sector_buf);
    if (rc != 0) {
        pg_free(18, (void *)sector_buf);
        return rc - 5;
    }

    /* Process sector data here */

    pg_free(18, (void *)sector_buf);
    return 0;
}


void *pci_map_ehci_mmio(uint16_t *num_ports, uintptr_t *mmio_base_out, void *bar0_virt) {    
    if (!bar0_virt) return NULL;
    
    /* Read CAPLENGTH to find operational registers offset */
    volatile ehci_cap_regs_t *cap_regs = (volatile ehci_cap_regs_t *)bar0_virt;
    uint8_t caplength = cap_regs->CAPLENGTH;
    
    /* Read HCSPARAMS to get number of ports */
    uint32_t hcsparams = cap_regs->HCSPARAMS;
    uint16_t n_ports = HCSPARAMS_N_PORTS(hcsparams);
    
    /* Check if controller has Port Power Control */
    int has_ppc = HCSPARAMS_PPC(hcsparams);
    (void)has_ppc;  // You might want to use this later
    
    /* Return pointer to operational registers base */
    void *op_regs = (void *)((uintptr_t)bar0_virt + caplength);
    
    /* Set output parameters */
    if (num_ports) *num_ports = n_ports;
    if (mmio_base_out) *mmio_base_out = (uintptr_t)bar0_virt;

    ehci = bar0_virt;
    
    return op_regs;
}

/* -------------------------------------------------------------------------
   Alternative: If you want more control over the PCI scanning
   ------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
   Helper: Debug print EHCI capabilities
   ------------------------------------------------------------------------- */

void ehci_print_capabilities(void *bar0_virt) {
    volatile ehci_cap_regs_t *cap = (volatile ehci_cap_regs_t *)bar0_virt;
    
    uint8_t caplength = cap->CAPLENGTH;
    uint16_t hciversion = cap->HCIVERSION;
    uint32_t hcsparams = cap->HCSPARAMS;
    uint32_t hccparams = cap->HCCPARAMS;
    
    terminal_printf("EHCI Capability Registers:\n");
    terminal_printf("  CAPLENGTH:  0x%x (%d bytes)\n", caplength, caplength);
    terminal_printf("  HCIVERSION: 0x%x (USB %d.%d)\n", 
                   hciversion, (hciversion >> 8) & 0xFF, hciversion & 0xFF);
    terminal_printf("  HCSPARAMS:  0x%x\n", hcsparams);
    terminal_printf("    N_PORTS:  %d\n", HCSPARAMS_N_PORTS(hcsparams));
    terminal_printf("    PPC:      %s\n", HCSPARAMS_PPC(hcsparams) ? "Yes" : "No");
    terminal_printf("    N_CC:     %d companion controllers\n", HCSPARAMS_N_CC(hcsparams));
    terminal_printf("  HCCPARAMS:  0x%x\n", hccparams);
    terminal_printf("  Op Regs at: BAR0 + 0x%x\n", caplength);
    
}