#include "vga.h"

uint8_t* vmem_base_addr;
static uint8_t *mem_image;

/**
 * @brief Initialization helper function for video mode
*/
void init_screen(void)
{
	/* Map video memory to program memory space */
	set_video_mode();
}

/**
 * @brief Setup program video memory map, using
 *        a call to the vidmap system call
*/
int32_t set_video_mode(void)
{
	/* Call vidmap system call */
	if(ece391_vidmap(&mem_image) == -1) {
        return - 1;
    }

	/* For missile assembly code */
	vmem_base_addr = mem_image;

	/* Return success */
	return 0;
}

/**
 * @brief Clear the screen
*/
void clear_screen()
{
	uint32_t i = 0;
	for(i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++){
		((uint8_t*)mem_image)[++i] = BLANK;
	}
}

/**
 * @brief Draw character to the screen
 * 
 * @param c : Character to draw
 * @param x : X position of character
 * @param y : Y position of character
*/
void write_char(char c, int x, int y)
{
	((char*)mem_image)[(80 * y + x) << 1] = c;
}

/**
 * @brief Draw string to the screen
 * 
 * @param s : String to draw
 * @param x : X position of character
 * @param y : Y position of character
*/
void write_string(char *s, int x, int y)
{
	uint32_t i = (80 * y + x) << 1;
	while(*s) {
		((uint8_t*)mem_image)[i] = *s++;
		i += 2;
	}
}

/**
 * @brief Draw horizontally centered string 
*/
void draw_centered_string(char *s, int y)
{
	write_string(s, (80 - ece391_strlen((uint8_t*)s)) / 2, y);
}

/**
 * @brief Draw game start screen
*/
void draw_starting_screen(void)
{
	uint32_t line = 5;
	DCS("  ___|)________________________________________________________  ");
	DCS(" |___/____________________________|___________________________|| ");
	DCS(" |__/|_______/|____/|_____/|______|___________________________|| ");
	DCS(" |_/(|,\\____/_|___/_|____/_|______|___________________________|| ");
	DCS(" |_\\_|_/___|__|__|__|___|__|___|__|___________________________|| ");
	DCS(" |   |     | ()  | ()   | ()   |  |                           || ");
	DCS(" | (_|   -()-  -()-   -()-   -()- | -()-  -()-  -()-   -()-   || ");
	DCS(" |________________________________|__|__()_|__()_|__()__|_____|| ");
	DCS(" |__/___\\_._______________________|__|__|__|__|__|__|___|_____|| ");
	DCS(" |__\\___|.________________________|___\\_|___\\_|___\\_|___|_____|| ");
	DCS(" |_____/__________________________|____\\|____\\|____\\|_________|| ");
	DCS(" |____/___________________________|___________________________|| ");
	DCS("                                                                 ");
	DCS("                                                                 ");
}

/**
 * @brief Draw game end screen
*/
void draw_end_screen(void)
{
	uint32_t line = 5;
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
	DCS("                                                                 ");
}
