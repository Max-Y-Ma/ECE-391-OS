#ifndef _VGA_H_
#define _VGA_H_

#include "lib391/types.h"
#include "lib391/ece391syscall.h"
#include "lib391/ece391support.h"

#define DCS(str) draw_centered_string( str , line++);

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

#define BLANK       0x20

/**
 * @brief Initialization helper function for video mode
*/
void init_screen(void);

/**
 * @brief Setup program video memory map, using
 *        a call to the vidmap system call
*/
int32_t set_video_mode(void);

/**
 * @brief Clear the screen
*/
void clear_screen();

/**
 * @brief Draw character to the screen
 * 
 * @param c : Character to draw
 * @param x : X position of character
 * @param y : Y position of character
*/
void write_char(char c, int x, int y);

/**
 * @brief Draw string to the screen
 * 
 * @param s : String to draw
 * @param x : X position of character
 * @param y : Y position of character
*/
void write_string(char *s, int x, int y);

/**
 * @brief Draw horizontally centered string 
*/
void draw_centered_string(char *s, int y);

/**
 * @brief Draw game status bar
*/
void draw_status_bar(void);

/**
 * @brief Draw game start screen
*/
void draw_starting_screen(void);

/**
 * @brief Draw game end screen
*/
void draw_end_screen(void);

#endif /* _VGA_H_ */
