#include "vga.h"
#include "lib391/ece391syscall.h"
#include "lib391/types.h"

#define DURATION 25

int main() {
    /* Initialize Screen and Input */
	init_screen();

    /* Initialize RTC */
    uint32_t rtc_fd = ece391_open((uint8_t*)"rtc");
    uint32_t rtc_freq = 32;
    rtc_freq = ece391_write(rtc_fd, &rtc_freq, 4);

    /* Query user frequency input */
    uint8_t buf[128];
    ece391_write(STDOUT, "Enter a sine wave frequency to play: ", ece391_strlen((uint8_t*)"Enter a sine wave frequency to play: "));
    ece391_read(STDIN, buf, 128);

    /* Start App */
    draw_starting_screen();

    /* Setup terminal */
    ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 0);

    /* Play Audio */
    uint32_t frequency = ece391_atoi((char*) buf);
	ece391_ioctl(STDIN, TERMINAL_IOCTL_PLAY_AUDIO, 0);
    ece391_ioctl(STDIN, TERMINAL_IOCTL_LOAD_SINEWAVE, frequency);

    /* Wait time duration */
    uint32_t i, garbage;
    for(i = 0; i < DURATION; i++) {
        ece391_read(rtc_fd, &garbage, 4);
    }


    /* End App */
    ece391_close(rtc_fd);
    draw_end_screen();

    /* STOP Audio */
	ece391_ioctl(STDIN, TERMINAL_IOCTL_STOP_AUDIO, 0);

    /* Restore terminal */
	ece391_ioctl(STDIN, TERMINAL_IOCTL_SET_OUTPUT_MODE, 1);

    return 0;
}
