#include "terminal.h"
#include "audio.h"
#include "../tests.h"
#include "../switch.h"
#include "../i8259.h"
#include "../proc/PCB.h"
#include "../page.h"

#if BUILD_TERMINAL

/* Display Variables */
static char* video_mem = (char *)VIDEO;

/* Terminal Control Block: terminal information data structures */
/*static*/ terminal_info_t tcb[MAX_NUM_TERMINAL];
/*static*/ uint8_t tcb_idx = 0;

/**
 * @brief Index to the active terminal
 * 
 * @details The active terminal writes to video memory
 *          Terminals that are not active write to their respective screen_buffer
*/
static uint8_t active_tcb_idx = 0; 

/* Terminal op table for PCB jumps, see types.h */
device_op_table_t terminal_op_table = {
    terminal_read,
    terminal_write,
    terminal_open,
    terminal_close,
    terminal_ioctl 
};

void clear_line_buffer(void)
{
    /* Clear line buffer */
    tcb[tcb_idx].buffer_idx = 0;
    memset(tcb[tcb_idx].line_buffer, '\0', MAX_CHAR_BUFFER_SIZE);
}

void clear_history_buffer(void)
{
    /* Clear history buffer data */
    tcb[tcb_idx].history_idx = 0;
    tcb[tcb_idx].viewing_history = 0;
    memset(tcb[tcb_idx].history_buffer, '\0', MAX_CHAR_BUFFER_SIZE * NUM_COLS);
}

void save_screen_buffer(void)
{
    /* Save entire screen into screen buffer */
    int i;
    for (i = 0; i < NUM_ROWS; i++) {
        memcpy(tcb[tcb_idx].screen_buffer[i], (uint16_t*)video_mem + (i * NUM_COLS), 2 * NUM_COLS);
    }
}

void putc(uint8_t c)
{
    /* No printing to terminal while viewing history buffers */
    if (tcb[tcb_idx].viewing_history) { return; }

    /* Check for backspace character */
    if (c == '\b') {
        if (tcb[tcb_idx].output_mode) {
            /* Check terminal x limit */
            if (tcb[tcb_idx].terminal_screen_x == tcb[tcb_idx].terminal_limit_x) { return; }

            tcb[tcb_idx].terminal_screen_x--;

            /* Support wrap around */
            if (tcb[tcb_idx].terminal_screen_x < 0) {
                /* End of screen boundary*/
                if (tcb[tcb_idx].terminal_screen_y == 0) {
                    tcb[tcb_idx].terminal_screen_x = 0; return;
                }

                tcb[tcb_idx].terminal_screen_y--;
                tcb[tcb_idx].terminal_screen_x = NUM_COLS - 1;
            }

            /* Remove character */
            if (active_tcb_idx == tcb_idx) {
                *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = ' ';
                *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
            } else {
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = ' ';
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
            }
        }

        /* Update line buffer, 1 last index before beginning */
        if (1 <= tcb[tcb_idx].buffer_idx && (active_tcb_idx == tcb_idx))
            tcb[tcb_idx].line_buffer[--tcb[tcb_idx].buffer_idx] = '\0';
    }
    /* Check for newline characters */
    else if (c == '\n' || c == '\r') {
        if (tcb[tcb_idx].output_mode) {
            tcb[tcb_idx].terminal_screen_y++;
            tcb[tcb_idx].terminal_screen_x = 0;

            /* Support scrolling */
            scroll_screen();
        }

        /* Update line buffer, 127 last index for newline */
        if ((tcb[tcb_idx].buffer_idx < MAX_CHAR_BUFFER_SIZE - 2) && (active_tcb_idx == tcb_idx))
            tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx++] = c;
    } 
    /* Check for tab character */
    else if (c == '\t') {
        int i;
        for (i = 0; i < TAB_SIZE; i++) {
            if (tcb[tcb_idx].output_mode) {
                if (active_tcb_idx == tcb_idx) {
                    *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = ' ';
                    *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
                } else {
                    *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = ' ';
                    *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
                }

                tcb[tcb_idx].terminal_screen_x++;

                /* Support wrap around, checking y first */
                tcb[tcb_idx].terminal_screen_y = (tcb[tcb_idx].terminal_screen_y + (tcb[tcb_idx].terminal_screen_x / NUM_COLS));

                /* Support scrolling */
                scroll_screen();

                tcb[tcb_idx].terminal_screen_x %= NUM_COLS;
            }

            /* Update line buffer, 127 last index for newline */
            if ((tcb[tcb_idx].buffer_idx < MAX_CHAR_BUFFER_SIZE - 2) && (active_tcb_idx == tcb_idx))
                tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx++] = ' ';
        }
    } 
    else {
        /* Check printable characters */
        if (!isprint(c)) {return;}

        if (tcb[tcb_idx].output_mode) {
            if (active_tcb_idx == tcb_idx) {
                *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = c;
                *(uint8_t *)(video_mem + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
            } else {
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1)) = c;
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((NUM_COLS * tcb[tcb_idx].terminal_screen_y + tcb[tcb_idx].terminal_screen_x) << 1) + 1) = tcb[tcb_idx].mem_attrib;
            }

            tcb[tcb_idx].terminal_screen_x++;

            /* Support wrap around, checking y first */
            tcb[tcb_idx].terminal_screen_y = (tcb[tcb_idx].terminal_screen_y + (tcb[tcb_idx].terminal_screen_x / NUM_COLS));

            /* Support scrolling */
            scroll_screen();

            tcb[tcb_idx].terminal_screen_x %= NUM_COLS;
        }

        /* Update line buffer, 127 last index for newline */
        if ((tcb[tcb_idx].buffer_idx < MAX_CHAR_BUFFER_SIZE - 2) && (active_tcb_idx == tcb_idx))
            tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx++] = c;
    }

    /* Update Cursor */
    if (tcb[tcb_idx].output_mode) {
        if (active_tcb_idx == tcb_idx) {
            update_cursor(tcb[tcb_idx].terminal_screen_x, tcb[tcb_idx].terminal_screen_y);
        }
    }
}

void update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;
 
    /* Set cursor low register */
	outb(CURSOR_LOW_REG, CRT_ADDR_PORT);
	outb((uint8_t) (pos & 0xFF), CRT_DATA_PORT);

    /* Set cursor high register */
	outb(CURSOR_HIGH_REG, CRT_ADDR_PORT);
	outb((uint8_t) ((pos >> 8) & 0xFF), CRT_DATA_PORT);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    /* Set maximum scan line register to 15 */
    outb(MAX_SCAN_LINE_REG, CRT_ADDR_PORT);
    outb(0x0F, CRT_DATA_PORT);

    /* Set cursor end line to 15 */
	outb(CURSOR_START_REG, CRT_ADDR_PORT);
	outb((0x1F & cursor_start), CRT_DATA_PORT);
 
    /* Set cursor start line to 14 + cursor visibility */
	outb(CURSOR_END_REG, CRT_ADDR_PORT);
	outb((0x1F & cursor_end), CRT_DATA_PORT);

    /* Reset memory attributes */
    int i = 0;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        if (active_tcb_idx == tcb_idx) {
            *(uint8_t *)(video_mem + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
        } else {
            *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
        }
    }
}

uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(CURSOR_LOW_REG, CRT_ADDR_PORT);
    pos |= inb(CRT_DATA_PORT);
    outb(CURSOR_HIGH_REG, CRT_ADDR_PORT);
    pos |= ((uint16_t)inb(CRT_DATA_PORT)) << 8;
    return pos;
}

void disable_cursor()
{
    /* Clear enable cursor bit */
	outb(CURSOR_START_REG, CRT_ADDR_PORT);
	outb(0x20, CRT_DATA_PORT);
}

int32_t puts(int8_t *s)
{
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

void clear(void) 
{
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        if (active_tcb_idx == tcb_idx) {
            *(uint8_t *)(video_mem + (i << 1)) = ' ';
            *(uint8_t *)(video_mem + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
        } else {
            *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1)) = ' ';
            *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
        }
    }

    /* Clean terminal variables*/
    tcb[tcb_idx].terminal_limit_x = 0;
    tcb[tcb_idx].terminal_screen_x = 0;
    tcb[tcb_idx].terminal_screen_y = 0;

    /* Update cursor */
    if (tcb[tcb_idx].output_mode) {
        enable_cursor(14, 15);
        update_cursor(tcb[tcb_idx].terminal_screen_x, tcb[tcb_idx].terminal_screen_y);

        /* Restore line buffer*/
        if (tcb[tcb_idx].terminal_rd) {
            uint8_t line_buffer_copy[MAX_CHAR_BUFFER_SIZE];
            memcpy(line_buffer_copy, tcb[tcb_idx].line_buffer, MAX_CHAR_BUFFER_SIZE);
            int32_t index = 0, buffer_idx_copy = tcb[tcb_idx].buffer_idx;
            /* Clearing line buffer for putc */
            clear_line_buffer();
            while (line_buffer_copy[index] != '\0' && index < buffer_idx_copy - 1) {
                /* putc will draw to screen restoring the line buffer to previous state */
                putc(line_buffer_copy[index]);
                index++;
            }
        }
    }
}

void shift_screen_up(void)
{
    /* Save top of video memory to history if not viewing it */
    // if (!tcb[tcb_idx].viewing_history) {
    //     memcpy(tcb[tcb_idx].history_buffer[tcb[tcb_idx].history_idx], (uint16_t*)video_mem, 2 * NUM_COLS);

    //     /* Update history flags and indexes */
    //     tcb[tcb_idx].history_idx = (tcb[tcb_idx].history_idx + 1) % NUM_ROWS;
    // }

    /* Print rows up until last row*/
    if (tcb[tcb_idx].output_mode) {
        int32_t i;
        for (i = 0; i < (NUM_ROWS - 1) * NUM_COLS; i++) {
            if (active_tcb_idx == tcb_idx) {
                *(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i + (NUM_COLS)) << 1));
                *(uint8_t *)(video_mem + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
            } else {
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1)) = *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + ((i + (NUM_COLS)) << 1));
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
            }
        }

        /* Clear last row */
        for (i = ((NUM_ROWS - 1) * NUM_COLS); i < NUM_ROWS * NUM_COLS; i++) {
            if (active_tcb_idx == tcb_idx) {
                *(uint8_t *)(video_mem + (i << 1)) = ' ';
                *(uint8_t *)(video_mem + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
            } else {
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1)) = ' ';
                *(uint8_t *)((uint8_t*)tcb[tcb_idx].screen_buffer + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
            }
        }
    }
}

void show_history(void)
{
    /* Check for viewable history */
    if (tcb[tcb_idx].viewing_history) {return;}
    tcb[tcb_idx].viewing_history = 1; disable_cursor();
    save_screen_buffer();

    /* Print circular buffer starting one before start pointer and working backwards */
    int i; int count = (tcb[tcb_idx].history_idx + (NUM_ROWS - 1)) % NUM_ROWS;
    for (i = 0; i < NUM_ROWS; i++) {
        if (count < 0) {count = NUM_ROWS - 1;}
        memcpy((uint16_t*)video_mem + ((NUM_ROWS - 1 - i) * NUM_COLS), tcb[tcb_idx].history_buffer[count], 2 * NUM_COLS);
        count--;
    }
}

void show_main(void)
{
    /* Update history flags and indexes */
    if (!tcb[tcb_idx].viewing_history) {return;}
    
    tcb[tcb_idx].viewing_history = 0; enable_cursor(14, 15);

    /* Print normal of screen */
    int32_t i;
    for (i = 0; i < NUM_ROWS; i++) {
        memcpy((uint16_t*)video_mem + (i * NUM_COLS), tcb[tcb_idx].screen_buffer[i], 2 * NUM_COLS);
    }
}

void scroll_screen(void)
{
    if (tcb[tcb_idx].terminal_screen_y == NUM_ROWS) {
        /* Shift video memory */
        shift_screen_up();

        tcb[tcb_idx].terminal_screen_y = NUM_ROWS - 1;
    }
}

void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

void terminal_bsod(void) 
{
    /* Set memory attribute */
    tcb[tcb_idx].mem_attrib = BSOD_ATTRIB;

    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = tcb[tcb_idx].mem_attrib;
    }

    /* Set BSOD message */
    int8_t str_bsod[] = "Blue Screen of Death";
    tcb[tcb_idx].terminal_screen_x = ((NUM_COLS / 2) - 1) - (strlen(str_bsod) / 2);
    tcb[tcb_idx].terminal_screen_y = (NUM_ROWS / 2) - 1;
    puts(str_bsod);

    /* Clean terminal variables */
    tcb[tcb_idx].terminal_screen_x = 0;
    tcb[tcb_idx].terminal_screen_y = 0;

    /* Halt */
    while(1) { asm volatile ("hlt"); }
}
#endif

void next_terminal(void)
{
    /* Save previous terminal screen */
    memcpy(tcb[active_tcb_idx].screen_buffer, (uint16_t*)video_mem, 2 * NUM_ROWS * NUM_COLS);

    /* Deactive current vidmap page, writing to terminal screen buffer */
    deactivate_proc_vidmem(tcb[active_tcb_idx].curr_pcb->id);

    /* Move to next terminal */
    active_tcb_idx = (active_tcb_idx + 1) % MAX_NUM_TERMINAL;

    /* Activate next vidmap page, writing to video memory */
    activate_proc_vidmem(tcb[active_tcb_idx].curr_pcb->id);
    
    /* Flush TLB */
    load_page_directory((uint32_t*)get_proc_page(get_curr_pcb()->id)->proc_pdirectory);

    /* Redraw screen */
    memcpy((uint16_t*)video_mem, tcb[active_tcb_idx].screen_buffer, 2 * NUM_ROWS * NUM_COLS);

    /* Update cursor */
    if (tcb[active_tcb_idx].output_mode) {
        enable_cursor(14, 15);
        update_cursor(tcb[active_tcb_idx].terminal_screen_x, tcb[active_tcb_idx].terminal_screen_y);
    } else {
        disable_cursor();
    }
}

/**
 * @brief Set current terminal index
*/
void set_tcb_idx(uint8_t idx)
{
    tcb_idx = idx;
}

/**
 * @brief Get current terminal index
*/
uint8_t get_tcb_idx(void)
{
    return tcb_idx;
}

/**
 * @brief Set active terminal index
*/
void set_active_tcb_idx(uint8_t idx)
{
    active_tcb_idx = idx;
}

/**
 * @brief Get active terminal index
*/
uint8_t get_active_tcb_idx(void)
{
    return active_tcb_idx;
}

/**
 * @brief Get tcb screen buffer
*/
uint16_t* get_tcb_screen_buffer(uint8_t tcb_index)
{
    return (uint16_t*)tcb[tcb_index].screen_buffer;
}

/**
 * @brief Initialize all terminals
*/
void setup_terminals(void)
{
    terminal_open((uint8_t*)"terminal");
}

/**
 * @brief Initialize the terminal variables and settings 
 * 
 * @param filename string that describes the filename of the device
 * 
 * @return 0
*/
int terminal_open(const uint8_t* filename)
{
    /* Check filename */
    if (filename == NULL) {return -1;}

    /* Set keyboard callback function */
    register_keyboard_cb(terminal_keycode_handler_cb);

    /* Initialize tcb buffers and variables */
    int i;
    for (i = 0; i < MAX_NUM_TERMINAL; i++) {
        /* Set display attribute */
        tcb[i].mem_attrib = (i == 0) ?  TERM1_ATTRIB : 
                            (i == 1) ?  TERM2_ATTRIB : 
                                        TERM3_ATTRIB ;

        /* Terminal read state */
        tcb[i].terminal_rd = 0;
        
        /* RTC internal state */
        tcb[i].rtc_freq_counter = 0;
        tcb[i].rtc_freq_rollover = 1;
        tcb[i].rtc_interrupt_occurred = 0;

        /* Output mode ON */
        tcb[i].output_mode = 1;

        /* Clear line buffer */
        tcb[i].buffer_idx = 0;
        memset(tcb[i].line_buffer, '\0', MAX_CHAR_BUFFER_SIZE);

        tcb_idx = i;
        clear();
    }

    /* Restore tcb_idx */
    tcb_idx = active_tcb_idx;
    
    return 0;
}

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
int terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
    /* Check arguments */
    if (fd < 0 || nbytes < 0 || buf == NULL) {return -1;}

    tcb[tcb_idx].terminal_rd = 1;
    clear_line_buffer();

    /* Return terminal line buffer upon a newline or overflow */
    while (tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx - 1] != '\n' && tcb[tcb_idx].buffer_idx < nbytes - 1 && tcb[tcb_idx].buffer_idx != MAX_CHAR_BUFFER_SIZE - 2);

    /* Append a Newline and null Termination */
    if (tcb[tcb_idx].buffer_idx == MAX_CHAR_BUFFER_SIZE - 2 || tcb[tcb_idx].buffer_idx == nbytes - 2) {
        tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx++] = '\n';
        tcb[tcb_idx].line_buffer[tcb[tcb_idx].buffer_idx++] = '\0';
    }
    memset((uint8_t*)buf, '\0', MAX_CHAR_BUFFER_SIZE);
    memcpy((uint8_t*)buf, tcb[tcb_idx].line_buffer, tcb[tcb_idx].buffer_idx);

    tcb[tcb_idx].terminal_rd = 0;

    return tcb[tcb_idx].buffer_idx;
}

/**
 * @brief Writes the number of bytes from the buffer to the screen.
 * 
 * @param fd        file descriptor that is associated with terminal driver
 * @param buffer    character buffer for the message to display to the terminal 
 * @param nbytes    number of bytes to display on the terminal 
 * 
 * @return Return number of bytes written, -1 for invalid parameters 
*/ 
int terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
    /* Check arguments */
    if (fd < 0 || nbytes < 0 || buf == NULL) {return -1;}

    /* Print buffer to terminal and return bytes written */
    int32_t index = 0;
    while (index < nbytes) {
        if (((int8_t *) buf)[index] == '\0') {
            index ++;
            continue;
        }
        putc(((int8_t*)buf)[index]);
        index++;
    }

    /* Set terminal x limit */
    tcb[tcb_idx].terminal_limit_x = tcb[tcb_idx].terminal_screen_x;

    return index;
}

/**
 * @brief Clear terminal specific variables and data
 * 
 * @param filename string that describes the filename of the device
 * 
 * @return 0
*/
int terminal_close(int32_t fd)
{
    /* Check file descriptor */
    if (fd < 0) {return -1;}

    clear_line_buffer();
    clear();

    disable_cursor();

    return 0;
}

/**
 * @brief Handles all terminal ioctl commands
 * 
 * @param fd : File descriptor for device
 * @param command : Device ioctl command 
 * @param args : Ioctl arguments, multiple arguments should be stored in a buffer
 * 
 * @return 0 on sucessful operation, -1 on failed operation
*/
int32_t terminal_ioctl(int32_t fd, uint32_t command, uint32_t args)
{
    /* Check file descriptor */
    if (fd < 0) {return -1;}

    switch(command) {
        case TERMINAL_IOCTL_SET_OUTPUT_MODE: {
            tcb[tcb_idx].output_mode = args;
            (args == 1) ? enable_cursor(14, 15) : disable_cursor();;
            break;
        }
        case TERMINAL_IOCTL_PLAY_AUDIO: {
            play_audio();
            break;
        }
        case TERMINAL_IOCTL_LOAD_SINEWAVE: {
            load_sine_wave(args);
            break;
        }
        case TERMINAL_IOCTL_STOP_AUDIO: {
            stop_audio();
            break;
        }
        default:
            break;
    }

    return 0;
}

/* Terminal callback function for keyboard driver */
void terminal_keycode_handler_cb(const key_packet_t* key_pkt)
{
    /* Support clearing the terminal screen */
    if (key_pkt->keycode_flags[CONTROL] && key_pkt->keycode_flags['l']) {
        clear();
    }
    else if (key_pkt->keycode_flags[CONTROL] && key_pkt->keycode_flags['L']) {
        clear();
    }
    /* Support calling blue screen of death */
    else if (key_pkt->keycode_flags[CONTROL] && key_pkt->keycode_flags[ALT] && key_pkt->keycode_flags[SHIFT]) {
        terminal_bsod();
    }
    /* Support viewing history */
    else if (key_pkt->keycode_flags[PAGE_UP]) {
        show_history();
    }
    else if (key_pkt->keycode_flags[PAGE_DOWN]) {
        show_main();
    }
    /* Support multiple terminals */
    else if (key_pkt->keycode_flags[ALT] && key_pkt->keycode_flags[F2]) {
        next_terminal();
    }

#if RUN_TERM_TESTS
    /* Run Test Cases */
    else if (key_pkt->keycode_flags[CONTROL] && key_pkt->keycode_flags['I']) {
        int i = 0;
        for (i = 0; i < 32; i++) {
            test_interrupts();
        }
        TEST_OUTPUT("idt_test", idt_test());
    }
    else if (key_pkt->keycode_flags[CONTROL] && key_pkt->keycode_flags['P']) {
        TEST_OUTPUT("paging_tests", paging_test());
    }
#endif
}

pcb_t* tcb_get_pcb(uint8_t idx) {
    return tcb[idx].curr_pcb;
}

void tcb_set_pcb(uint8_t idx, pcb_t* pcb) {
    tcb[idx].curr_pcb = pcb;
}

uint8_t tcb_get_curr_idx(void) {
    return tcb_idx;
}

void tcb_set_curr_idx(uint8_t idx) {
    tcb_idx = idx;
}
