#include "keyboard.h"
#include "terminal.h"
#include "../i8259.h"
#include "../lib.h"

#include "RTC.h"

/* Keyboard op table for PCB jumps, see types.h */
device_op_table_t keyboard_op_table = {
    keyboard_read,
    keyboard_write,
    keyboard_open,
    keyboard_close
};

/**
 * @brief Scancode to keycode mapping for scan codes in set 1 (US QWERTY).
 *        Maps the keycodes to their ASCII or meaningful counterparts. Initial mapping,
 *        refer to init_keycode_mapping for further special character support. 
 * 
 * @details Scancode -> Keycode
*/
uint8_t keycode_mapping[KEY_MAPPING_SIZE] =
{
    '\0', '\e', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',
     'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o', 'p',  '[',  ']', '\n', CONTROL,  'a',  's',
     'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'', '`', SHIFT, '\\',  'z',  'x',  'c',  'v',
     'b',  'n',  'm',  ',',  '.',  '/', SHIFT, '\0', ALT, ' ', CAPSLOCK, '\0', '\0', /* just special chars from here... */
};

/**
 * @brief Scancode to shift + keycode mapping for scan codes in set 1 (US QWERTY)
 * 
 * @details Scancode -> Shift + Keycode
*/
uint8_t shift_keycode_mapping[KEY_MAPPING_SIZE] = 
{
    '\0', '\e', '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', '\b', '\t',
     'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O', 'P',  '{',  '}', '\n', CONTROL,  'A',  'S',
     'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"', '~', SHIFT, '|',  'Z',  'X',  'C',  'V',
     'B',  'N',  'M',  '<',  '>',  '?', SHIFT, '\0', ALT, ' ', CAPSLOCK, '\0', '\0', /* just special chars from here... */
};

static key_packet_t driver_key_packet;
const key_packet_t* get_key_packet(void)
{
    return &driver_key_packet;
}

/* Special Keycode Callback Function */
keycode_handler_cb kc_handler_cb = NULL;

/* Keyboard Driver API */

/**
 * @brief Initialize keyboard data members
 * 
 * @return 0
*/
int keyboard_open(const uint8_t* filename)
{
    return 0;
}

/**
 * @brief Blocks until a keyboard interrupt is triggered
 * 
 * @return 0
*/
int keyboard_read(int32_t fd, void* buf, int32_t nbytes)
{
    return 0;
}

/**
 * @brief Unsupported 
 * 
 * @return 0
*/
int keyboard_write(int32_t fd, const void* buf, int32_t nbytes)
{
    return 0;
}

/**
 * @brief Cleanup keyboard data members
*/
int keyboard_close(int32_t fd)
{
    return 0;
}

/* Initializes the keyboard. */
void initialize_keyboard(void)
{
    init_keycode_mapping();

    /* Initialize key_packet struct */
    memset(&driver_key_packet, 0, sizeof(driver_key_packet));

    enable_irq(KEYBOARD_IRQ);
}

/* Register keycode handler callback function */
void register_keyboard_cb(keycode_handler_cb cb)
{
    kc_handler_cb = cb;
}

/* Initialize the keycode mapping lookup table */
void init_keycode_mapping(void)
{
    keycode_mapping[0x3B] = F1;
    keycode_mapping[0x3C] = F2;
    keycode_mapping[0x3D] = F3;
    keycode_mapping[0x3E] = F4;
    keycode_mapping[0x3F] = F5;
    keycode_mapping[0x40] = F6;
    keycode_mapping[0x41] = F7;
    keycode_mapping[0x42] = F8;
    keycode_mapping[0x43] = F9;
    keycode_mapping[0x44] = F10;
    keycode_mapping[0x47] = HOME;
    keycode_mapping[0x48] = CURSOR_UP;
    keycode_mapping[0x49] = PAGE_UP;
    keycode_mapping[0x4B] = CURSOR_LEFT;
    keycode_mapping[0x4D] = CURSOR_RIGHT;
    keycode_mapping[0x4F] = END;
    keycode_mapping[0x50] = CURSOR_DOWN;
    keycode_mapping[0x51] = PAGE_DOWN;
    keycode_mapping[0x52] = INSERT;
    keycode_mapping[0x53] = DELETE;
}

/* Handles interrupts raised by the keyboard. */
void keyboard_handler(void)
{
    /* Grab keycode from PS2 Controller */
    uint32_t scancode = inb(KEYBOARD_PORT);

    /* Process valid scancode */
    if (0 <= scancode && scancode < KEY_MAPPING_SIZE) {
        /* Scancode to keycode translation */
        uint8_t keycode; 
        if (driver_key_packet.keycode_flags[SHIFT] == KEY_PRESSED) {
            keycode = shift_keycode_mapping[REMOVE_RELEASED_MASK(scancode)];
        } else {
            keycode = keycode_mapping[REMOVE_RELEASED_MASK(scancode)];
        }

        /* Keycode translation for capslock */
        if (driver_key_packet.capslock_latch && isalpha(keycode)) {
            /* Shift & Capslock duality principle */
            if (driver_key_packet.keycode_flags[SHIFT] == KEY_PRESSED) {
                keycode = keycode_mapping[REMOVE_RELEASED_MASK(scancode)];
            } else {
                keycode = shift_keycode_mapping[REMOVE_RELEASED_MASK(scancode)];
            }
        }

        /* Set tcb_idx to active_terminal */
        uint8_t previous_terminal_idx = get_tcb_idx();
        set_tcb_idx(get_active_tcb_idx());

        /* Update keycode flags based on if the key was pressed or released */
        /* Page Up Support */
        if (scancode == 0xC9) {
            driver_key_packet.keycode_flags[PAGE_UP] = KEY_RELEASED;
        }
        else if (scancode == 0x49) {
            driver_key_packet.keycode_flags[PAGE_UP] = KEY_PRESSED;
        }
        /* Page Down Support */
        else if (scancode == 0xD1) {
            driver_key_packet.keycode_flags[PAGE_DOWN] = KEY_RELEASED;
        }
        else if (scancode == 0x51) {
            driver_key_packet.keycode_flags[PAGE_DOWN] = KEY_PRESSED;
        }
        else if (KEY_RELEASED_MASK(scancode)) {
            driver_key_packet.keycode_flags[keycode] = KEY_RELEASED;

            /* Native keyboard driver support for capslock */
            if (keycode == CAPSLOCK)
                driver_key_packet.capslock_latch = !driver_key_packet.capslock_latch;

        } else {
            driver_key_packet.keycode_flags[keycode] = KEY_PRESSED;

            /* Send keycode to terminal */
            putc(keycode);
        }   

        /* Handle special keys */
        if (kc_handler_cb != NULL)
            kc_handler_cb(&driver_key_packet);

        /* Restore tcb_idx */
        set_tcb_idx(previous_terminal_idx);
    }

    send_eoi(KEYBOARD_IRQ);
}
