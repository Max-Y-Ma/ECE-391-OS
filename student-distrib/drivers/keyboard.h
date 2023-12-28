/* keyboard.h - Initialization and interrupt handling of the keyboard.
    The keyboard is set to scan code set 1 by default.
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "../types.h"

#define KEYBOARD_IRQ 1

/* See OS Dev I/O Ports */
#define KEYBOARD_PORT 0x60

/* 216 allows for 0x00 - 0xD8, which should encompass all keycodes. */
#define KEY_MAPPING_SIZE 216

/* Special Modifing Codes */
#define CONTROL         0x80
#define SHIFT           0x81
#define ALT             0x82
#define CAPSLOCK        0x83

/* Special Keys */
#define CURSOR_UP       0x84
#define CURSOR_DOWN     0x85
#define CURSOR_RIGHT    0x86
#define CURSOR_LEFT     0x87
#define HOME            0x88
#define END             0x89
#define DELETE          0x8A
#define INSERT          0x8B
#define PAGE_DOWN       0x8C
#define PAGE_UP         0x8D

/* Function Keys */
#define F1              0x90
#define F2              0x91
#define F3              0x92
#define F4              0x93
#define F5              0x94
#define F6              0x95
#define F7              0x96
#define F8              0x97
#define F9              0x98
#define F10             0x99

/* Key States */
#define KEY_PRESSED     0x01
#define KEY_RELEASED    0x00

#define KEY_RELEASED_MASK(scancode) (0xC0 & scancode)
#define REMOVE_RELEASED_MASK(scancode) (0x3F & scancode)

extern device_op_table_t keyboard_op_table;

/**
 * @brief Struct to hold information for user-level programs 
 *        about the current keyboard states. 
*/
typedef struct key_packet_t {
    uint8_t keycode_flags[KEY_MAPPING_SIZE];        /* Keycode -> Key State */
    uint8_t capslock_latch;                         /* Latch for capslock support */
} key_packet_t;
const key_packet_t* get_key_packet(void);
 
typedef void (*keycode_handler_cb)(const key_packet_t*);

/* Keyboard Driver API */

/**
 * @brief Initialize keyboard data members
 * 
 * @return 0
*/
int keyboard_open(const uint8_t* filename);

/**
 * @brief Blocks until a keyboard interrupt is triggered
 * 
 * @return 0
*/
int keyboard_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Unsupported 
 * 
 * @return 0
*/
int keyboard_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Cleanup keyboard data members
*/
int keyboard_close(int32_t fd);

/**
 * @brief Initializes the keyboard.
 */
void initialize_keyboard(void);

/**
 * @brief Register keycode handler callback function.
 * 
 * @details Could support multiple callback functions 
 *          for different programs in the future.
*/
void register_keyboard_cb(keycode_handler_cb cb);

/**
 * @brief Initialize the keycode mapping lookup table
*/
void init_keycode_mapping(void);

/**
 * @brief Handles keyboard interrupts.
 */
void keyboard_handler(void);

/**
 * @brief Setter for keyboard output mode
*/
void set_output_mode(uint32_t mode);

#endif
