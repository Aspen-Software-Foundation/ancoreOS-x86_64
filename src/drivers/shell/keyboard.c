/*
    Copyright (C) 2026 Aspen Software Foundation

    Module: keyboard.c
    Description: Keyboard module for the VNiX Operating System
    Author: Mejd Almohammedi

    All components of the VNiX Operating System, except where otherwise noted, 
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

// Some parts may be under the fullowing license:
// https://github.com/Prankiman/tetrisOS/blob/master/LICENSE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "includes/shell/keyboard.h"
#include "includes/arch/x86_64/idt.h"
#include "includes/arch/x86_64/isr.h"
#include "includes/pic/pic_irq.h"
#include "includes/pic/apic/apic_irq.h"
#include "includes/pic/pic.h"
#include "includes/pic/apic/apic.h"
#include "includes/arch/x86_64/io.h"
#include "util/includes/log-info.h"
#include "util/includes/util.h"

#define KEYBOARD_IRQ_VECTOR         1
#define KEYBOARD_INTERRUPT_VECTOR   0x21
// IOAPIC base address and redirection entry for IRQ1 (keyboard)

#define IOAPIC_IRQ1_ENTRY     0x12  // Entry for IRQ1 in the Redirection Table
#define IOAPIC_IRQ1_VECTOR    0x21  // IRQ1 uses vector 0x21 on x86 systems (keyboard)
#define IOAPIC_DELIVERY_MODE  0x00000100  // Fixed delivery mode (normal interrupt)
#define IOAPIC_DESTINATION    0x00000000  // Send to all CPUs (this is usually specific for SMP systems)

uint8_t keyboard_read_response_safe() {
    // Wait until output buffer is full
    while (!(inb(0x64) & 0x01)) {
        /* spin */ 
    }
    return inb(0x60);  // now safe to read
}


// The following Keyboard layout struct may be under the fullowing license:
// https://github.com/Prankiman/tetrisOS/blob/master/LICENSE
keyboard_t keyboard;
const uint8_t keyboard_layout_us[2][128] = {
    {
        KEY_NULL, KEY_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
        '-', '=', KEY_BACKSPACE, KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u',
        'i', 'o', 'p', '[', ']', KEY_ENTER, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
        'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/', 0, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5,
        KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }, {
        KEY_NULL, KEY_ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
        '_', '+', KEY_BACKSPACE, KEY_TAB, 'Q', 'W',   'E', 'R', 'T', 'Y', 'U',
        'I', 'O', 'P',   '{', '}', KEY_ENTER, 0, 'A', 'S', 'D', 'F', 'G', 'H',
        'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
        'M', '<', '>', '?', 0, 0, 0, ' ', 0, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
        KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, 0, 0, KEY_HOME, KEY_UP,
        KEY_PAGE_UP, '-', KEY_LEFT, '5', KEY_RIGHT, '+', KEY_END, KEY_DOWN,
        KEY_PAGE_DOWN, KEY_INSERT, KEY_DELETE, 0, 0, 0, KEY_F11, KEY_F12
    }
};


// Function to send command to PS/2 keyboard
void keyboard_send_command(uint8_t cmd) {
    outb(KEYBOARD_CMD_PORT, cmd);
}

// Function to send data to PS/2 keyboard
void keyboard_send_data(uint8_t data) {
    outb(KEYBOARD_DATA_PORT, data);
}

// Function to read response from PS/2 keyboard
uint8_t keyboard_read_response() {
    return inb(KEYBOARD_DATA_PORT);
}

// Function to set the typematic rate and delay
uint8_t keyboard_set_typematic(keyboard_repeat_rate_t repeat_rate, keyboard_delay_time_t delay_time) {
    uint8_t typematic_byte = 0;

    // Set repeat rate (0x00 to 0x0B for 30 Hz to 2 Hz)
    typematic_byte |= (repeat_rate & REPEAT_RATE_MASK) << REPEAT_RATE_SHIFT;
    typematic_byte |= (delay_time & DELAY_MASK) << DELAY_SHIFT;
    typematic_byte &= ~UNUSED_BIT_MASK;

    // Send command to keyboard
    keyboard_send_command(CMD_TYPEMATIC_BYTE);
    keyboard_send_data(typematic_byte);
    iowait();

    // Read the response
    uint8_t response = keyboard_read_response();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to set LED states (ScrollLock, NumLock, CapsLock)
uint8_t keyboard_set_leds(keyboard_LEDState_t led_state) {
    keyboard_send_command(CMD_LED_STATE);
    keyboard_send_data((uint8_t)led_state);
    uint8_t response = keyboard_read_response_safe();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to enable scanning (keyboard sends scan codes)
uint8_t keyboard_enable_scanning() {
    keyboard_send_command(CMD_ENABLE_SCANNING);
    uint8_t response = keyboard_read_response_safe();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to reset keyboard (self-test)
uint8_t keyboard_reset() {
    keyboard_send_command(CMD_RESET);
    uint8_t response = keyboard_read_response_safe();
    if (response == RES_ACK) {
        // Self-test passed
        response = keyboard_read_response_safe(); // Read self-test result
        if (response == RES_SELF_TEST_PASSED) {
            return 0;
        } else if (response == RES_SELF_TEST_FAILED) {
            return 3;
        }
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Function to identify keyboard
uint8_t keyboard_identify() {
    keyboard_send_command(CMD_IDENTIFY_KEYBOARD);
    uint8_t response = keyboard_read_response_safe();
    if (response == RES_ACK) {
        return 0;
    } else if (response == RES_RESEND) {
        return 1;
    }
    return 2;
}

// Keyboard interrupt handler
void keyboard_handler(Registers_t *regs) {
    static bool extended = false;
    iowait();
    uint8_t scancode = keyboard_read_response_safe();


    if (scancode == 0xE0) {
        extended = true;
        return; // wait for the next byte
    }




    bool is_release = scancode & 0x80;
    uint8_t code = scancode & 0x7F;

    if (extended) {
        switch (code) {
            case 0x4B: // Left arrow
                keyboard.keys[KEY_LEFT] = !is_release;
                break;
            case 0x4D: // Right arrow
                keyboard.keys[KEY_RIGHT] = !is_release;
                break;
            case 0x48: // Up arrow
                keyboard.keys[KEY_UP] = !is_release;
                break;
            case 0x50: // Down arrow
                keyboard.keys[KEY_DOWN] = !is_release;
                break;
            case 0x53: // Delete
                keyboard.keys[KEY_DELETE] = !is_release;
                break;
            default:
                break;
        }
        extended = false; // reset prefix
        
        // Handle modifier keys
        if (KEY_SCANCODE(scancode) == KEY_LALT || KEY_SCANCODE(scancode) == KEY_RALT) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_ALT), KEY_IS_PRESS(scancode));
        } else if (KEY_SCANCODE(scancode) == KEY_LCTRL || KEY_SCANCODE(scancode) == KEY_RCTRL) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_CTRL), KEY_IS_PRESS(scancode));
        } else if (KEY_SCANCODE(scancode) == KEY_LSHIFT || KEY_SCANCODE(scancode) == KEY_RSHIFT) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SHIFT), KEY_IS_PRESS(scancode));
        } else if (KEY_SCANCODE(scancode) == KEY_CAPS_LOCK) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_CAPS_LOCK), KEY_IS_PRESS(scancode));
        } else if (KEY_SCANCODE(scancode) == KEY_NUM_LOCK) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_NUM_LOCK), KEY_IS_PRESS(scancode));
        } else if (KEY_SCANCODE(scancode) == KEY_SCROLL_LOCK) {
            keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SCROLL_LOCK), KEY_IS_PRESS(scancode));
        }
        
        (void)regs;
        return;
    }

    // Handle modifier keys
    if (KEY_SCANCODE(scancode) == KEY_LALT || KEY_SCANCODE(scancode) == KEY_RALT) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_ALT), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_LCTRL || KEY_SCANCODE(scancode) == KEY_RCTRL) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_CTRL), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_LSHIFT || KEY_SCANCODE(scancode) == KEY_RSHIFT) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SHIFT), KEY_IS_PRESS(scancode));
    } else if (scancode == MAGIC_CAPSLOCK_TOGGLE) {
        keyboard.mods ^= KEY_MOD_CAPS_LOCK;
    } else if (KEY_SCANCODE(scancode) == KEY_NUM_LOCK) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_NUM_LOCK), KEY_IS_PRESS(scancode));
    } else if (KEY_SCANCODE(scancode) == KEY_SCROLL_LOCK) {
        keyboard.mods = BIT_SET(keyboard.mods, HIBIT(KEY_MOD_SCROLL_LOCK), KEY_IS_PRESS(scancode));
    }

    bool was_on = keyboard.chars[KEY_CHAR(scancode|keyboard.mods)];

    // Update key state
    keyboard.keys[(uint8_t)(scancode & 0x7F)] = KEY_IS_PRESS(scancode);

    char keyDOWN = KEY_CHAR(scancode|keyboard.mods);
    if (keyboard.mods & KEY_MOD_CAPS_LOCK && keyDOWN >= 'a' && keyDOWN <= 'z')
        keyDOWN += keyboard.mods & KEY_MOD_SHIFT ? 32 : -32;
    keyboard.chars[(uint8_t)keyDOWN] = KEY_IS_PRESS(scancode);


    if (keyboard.chars[KEY_CHAR(scancode)] && !was_on && \
    (KEY_CHAR(scancode) >= 32 && KEY_CHAR(scancode) <= 126)) {
        //terminal_printf("You pressed %c \n", KEY_CHAR(scancode|keyboard.mods));
    } else if (!keyboard.chars[KEY_CHAR(scancode)] && was_on && \
    (KEY_CHAR(scancode) >= 32 && KEY_CHAR(scancode) <= 126)) {
        //terminal_printf("You released %c \n", KEY_CHAR(scancode|keyboard.mods));
    }

    LAPIC_SendEOI();
    (volatile void)regs;
}

void keyboard_pic_init() {
    PIC_IRQ_RegisterHandler(1, (IRQHandler_t)keyboard_handler);
    PIC_Unmask(1);
    LOG(Ok, keyboard_pic_init, "PIC Keyboard IRQ Initialized successfully.\n");
    SERIAL(Ok, keyboard_pic_init, "PIC Keyboard IRQ Initialized successfully.\n");
}

void keyboard_apic_init() {
    APIC_IRQ_RegisterHandler(1, (IRQHandler_t)keyboard_handler);
    IOAPIC_ConfigureKeyboard();
    // terminal_printf("Keyboard Interrupt configuration complete!\n");
}

// Function to configure IOAPIC for keyboard interrupt (IRQ1)
void IOAPIC_ConfigureKeyboard() {
    uint32_t entry_low, entry_high;

    // Read current IRQ1 entry
    entry_low = APIC_ReadIO(IOAPIC_IRQ1_ENTRY);       // Lower 32 bits
    entry_high = APIC_ReadIO(IOAPIC_IRQ1_ENTRY + 1);  // Upper 32 bits

    // Configure lower 32-bit redirection entry
    entry_low &= ~0x000000FF;  // Reset APICINT
    entry_low = IOAPIC_IRQ1_VECTOR;  // Set interrupt vector (0x21)
    entry_low &= ~0x00000700;  // Reset trig flag
    entry_low |= APIC_DELIVERY_MODE_FIXED;  // Fixed delivery mode
    entry_low &= ~(1 << 15);  // Edge-triggered (bit 15 = 0 for edge)
    entry_low &= ~(1 << 13);  // Active high polarity (bit 13 = 0 for high)
    entry_low &= ~(1 << 16); // Enable interrupt (bit 16 = 0)

    // Configure upper 32-bit redirection entry (destination field)
    entry_high &= ~0xFF000000;  // Clear the destination field
    entry_high |= (0 << 24);   // Route interrupt to CPU 0 (bit 24 = 1)

    // Write the updated values back
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY, entry_low);       // Write lower 32 bits
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY + 1, entry_high);  // Write upper 32 bits

    ;
    LOG(Ok, IOAPIC_ConfigureKeyboard, "IOAPIC Keyboard IRQ Initialized successfully.\n");
    SERIAL(Ok, IOAPIC_ConfigureKeyboard, "IOAPIC Keyboard IRQ Initialized successfully.\n");
}

// Function to mask the keyboard interrupt (IRQ1)
void IOAPIC_MaskIRQ1() {
    uint32_t entry_low, entry_high;

    entry_low = APIC_ReadIO(IOAPIC_IRQ1_ENTRY);  // Lower 32 bits
    entry_high = APIC_ReadIO(IOAPIC_IRQ1_ENTRY + 1);  // Upper 32 bits

    entry_low |= 0x00010000; 

    APIC_WriteIO(IOAPIC_IRQ1_ENTRY, entry_low);  // Write lower 32 bits
    APIC_WriteIO(IOAPIC_IRQ1_ENTRY + 1, entry_high);  // Write upper 32 bits
    printf("Masked IRQ1 (Keyboard)");
}


char getc(void) {

    static bool key_was_pressed[128] = {false};
    
    while (1) {
    (void)(keyboard.keys[KEY_LSHIFT] || keyboard.keys[KEY_RSHIFT]);
    (void)(keyboard.mods & KEY_MOD_CAPS_LOCK);


        //scan through chars
        for (uint8_t key = 0; key < 128; key++) {
            //if key was pressed, mark the specific key as true and print it 
            if (keyboard.chars[key] && !key_was_pressed[key]) {
                key_was_pressed[key] = true;

                if ((key >= 32 && key <= 126) || 
                    key == KEY_ENTER || 
                    key == KEY_BACKSPACE || 
                    key == KEY_TAB) {
                    return key;
                }
            }
            //if not pressed, revert key to false
            else if (!keyboard.chars[key] && key_was_pressed[key]) {
                key_was_pressed[key] = false;
            }
        }
    }
}