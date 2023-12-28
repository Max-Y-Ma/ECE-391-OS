#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "../lib.h"
#include "../types.h"
#include "keyboard.h"
#include "../proc/PCB.h"

extern device_op_table_t terminal_op_table;

/* IOCTL Commands for Terminal */
#define TERMINAL_IOCTL_SET_OUTPUT_MODE      0
#define TERMINAL_IOCTL_PLAY_AUDIO           1
#define TERMINAL_IOCTL_LOAD_SINEWAVE        2
#define TERMINAL_IOCTL_STOP_AUDIO           3

/**
 * 1.) Terminal Display
 *      a.) Support all numbers, lower and upper case letters, special characters, the shift, 
 *          ctrl, alt, capslock, tab, enter and backspace keys should work as intended.
 *      b.) You need not care about mapping the arrow keys or the numpad.
 *      c.) If the user fills up one line by typing and hasn’t typed 128 characters yet, 
 *          you should roll over to the next line.
 *      d.) Backspace on the new line should go back to the previous line as well.
 * 
 * 2.) terminal_read only returns when the enter key is pressed and should always 
 *     add the newline character at the end of the buffer before returning. 
 *      a.) Remember that the 128 character limit includes the newline character.
 *      b.) handle buffer overflow (user types more than 127 characters).
 *      c.) handle buffer underflow (user types less than 128 characters).
 * 
 * 3.) terminal_write should write the number of characters passed in the argument. 
 *      a.) Do not stop writing at a NUL byte. Do not print NUL bytes.
 * 
 * 4.) Both ctrl + l and ctrl + L should clear the screen. 
 *      a.) You should not be resetting the read buffer if the user clears the screen before 
 *          pressing enter while typing in terminal read.
 * 
 * 5.) Screen should scroll when it reaches the bottom, which removes the topmost line and creates a new
 *     blank line for new text.
 */

/* 128 characters (including 1 newline) + 1 null termination */
#define MAX_CHAR_BUF_SIZE   128U

#define RUN_TERM_TESTS      0U

#define BUILD_TERMINAL      1U

#define MAX_NUM_TERMINAL    3U

#define TAB_SIZE            4U

#define CRT_ADDR_PORT       0x3D4
#define CRT_DATA_PORT       0x3D5

#define MAX_SCAN_LINE_REG   0x09
#define CURSOR_START_REG    0x0A
#define CURSOR_END_REG      0x0B
#define CURSOR_HIGH_REG     0x0E
#define CURSOR_LOW_REG      0x0F

/* Display Variables */
#define VIDEO           0xB8000
#define NUM_COLS        80
#define NUM_ROWS        25
#define TERM1_ATTRIB    0x0A        /* Light Green on Black */
#define TERM2_ATTRIB    0x0E        /* Yellow on Black */
#define TERM3_ATTRIB    0x0C        /* Light Red on Black */
#define BSOD_ATTRIB     0x1F        /* Blue on White */

#define MAX_CHAR_BUFFER_SIZE    129U

#if (BUILD_TERMINAL == 1)

/**
 * @brief Terminal information structure used to save and restore terminals
*/
typedef struct terminal_info_t {
    /* Cursor position and boundaries */
    int terminal_limit_x;
    int terminal_screen_x;
    int terminal_screen_y;

    /* Output Mode */
    uint32_t output_mode;

    /* Display attribute */
    uint8_t mem_attrib;

    /* Circular buffer indexes */
    int volatile viewing_history;
    int history_idx;

    /* Circular buffer for storing history data */
    uint16_t screen_buffer[NUM_ROWS][NUM_COLS] __attribute__((aligned (4096)));
    uint16_t history_buffer[NUM_ROWS][NUM_COLS] __attribute__((aligned (4096)));

    /* terminal_read line buffer, 128 characters (including 1 newline) + 1 null termination */
    uint8_t terminal_rd;
    uint8_t line_buffer[MAX_CHAR_BUFFER_SIZE];

    /* Index to the end position of line buffer */   
    volatile uint8_t buffer_idx;

    pcb_t* curr_pcb;

    /* RTC internal state variables for each terminal. */
    int rtc_freq_counter;
    int rtc_freq_rollover;
    volatile int rtc_interrupt_occurred;

} terminal_info_t;

/**
 * @brief Clears the line buffer
*/
void clear_line_buffer(void);

/**
 * @brief Clears the history circular buffer
*/
void clear_history_buffer(void);

/**
 * @brief Saves the current screen into a flat buffer 
*/
void save_screen_buffer(void);

/**
 * @brief Print a single character to the terminal
 * 
 * @param c Character to display
*/
void putc(uint8_t c);

/**
 * @brief Prints an entire string to the terminal
 * 
 * @param s String to display
*/
int32_t puts(int8_t *s);

/**
 * @brief Clears the terminal screen
*/
void clear(void);

/**
 * @brief Shifts all pixels in the terminal screen by 1 bit
*/
void test_interrupts(void);

/**
 * @brief Check screen scrolling
*/
void scroll_screen(void);

/**
 * @brief Shifts screen one line upward
*/
void shift_screen_up(void);

/**
 * @brief Scrolls the screen one line upward to view history
*/
void show_history(void);

/**
 * @brief Scrolls the screen one line downward to view history
*/
void show_main(void);

/**
 * @brief Moves cursor on screen
*/
void update_cursor(int x, int y);

/**
 * @brief Enables the cursor for the terminal screen
*/
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);

/**
 * @brief Get current position of the cursor
*/
uint16_t get_cursor_position(void);

/**
 * @brief Disables the cursor for the terminal screen
*/
void disable_cursor(void);

/**
 * @brief Blue screen of death
*/
void terminal_bsod(void);

/**
 * @brief Cycles to the next terminal screen
*/
void next_terminal(void);

/**
 * @brief Set current terminal index
*/
void set_tcb_idx(uint8_t idx);

/**
 * @brief Get current terminal index
*/
uint8_t get_tcb_idx(void);

/**
 * @brief Set active terminal index
*/
void set_active_tcb_idx(uint8_t idx);

/**
 * @brief Get active terminal index
*/
uint8_t get_active_tcb_idx(void);

/**
 * @brief Get tcb screen buffer
*/
uint16_t* get_tcb_screen_buffer(uint8_t tcb_index);

/**
 * @brief Initialize all terminals
*/
void setup_terminals(void);

#endif


/**
 * @brief Initialize the terminal variables and settings 
 * 
 * @param filename string that describes the filename of the device
 * 
 * @return 0
*/
int32_t terminal_open(const uint8_t* filename);

/**
 * @brief Blocks until the enter key is pressed from the keyboad.
 * 
 * @details The caller should expect to recieve a maximum of 127 characters. 
 *          The function adds a newline character at the end of the buffer.
 *          Thus, the caller should pass a buffer with size 128 bytes. 
 * 
 * @param fd        file descriptor that is associated with terminal driver
 * @param buffer    buffer for read characters, size of atleast 128 bytes.
 * @param nbytes    number of bytes to read from the keyboard
 * 
 * @returns The number of bytes read into the buffer, return -1 for invalid parameters
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Writes the number of bytes from the buffer to the screen.
 * 
 * @param fd        file descriptor that is associated with terminal driver
 * @param buffer character buffer for the message to display to the terminal 
 * @param nbytes number of bytes to display on the terminal 
 * 
 * @return Return number of bytes written, -1 for invalid parameters 
*/ 
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Clear terminal specific variables and data
 * 
 * @param filename string that describes the filename of the device
 * 
 * @return 0
*/
int32_t terminal_close(int32_t fd);

/**
 * @brief Handles all terminal ioctl commands
 * 
 * @param fd : File descriptor for device
 * @param command : Device ioctl command 
 * @param args : Ioctl arguments, multiple arguments should be stored in a buffer
 * 
 * @return 0 on sucessful operation, -1 on failed operation
*/
int32_t terminal_ioctl(int32_t fd, uint32_t command, uint32_t args);

/**
 * @brief Terminal callback function for keyboard driver 
*/
void terminal_keycode_handler_cb(const key_packet_t*);

pcb_t* tcb_get_pcb(uint8_t idx);

void tcb_set_pcb(uint8_t idx, pcb_t* pcb);

uint8_t tcb_get_curr_idx(void);

void tcb_set_curr_idx(uint8_t idx);



#endif /* _TERMINAL_H_ */
