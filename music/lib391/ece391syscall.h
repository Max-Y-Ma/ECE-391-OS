#ifndef _ECE391SYSCALL_H_
#define _ECE391SYSCALL_H_

#include "types.h"

#define TERMINAL_IOCTL_SET_OUTPUT_MODE      0
#define TERMINAL_IOCTL_PLAY_AUDIO           1
#define TERMINAL_IOCTL_LOAD_SINEWAVE        2
#define TERMINAL_IOCTL_STOP_AUDIO           3

/* All calls return >= 0 on success or -1 on failure. */

/*  
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */ 
extern int32_t ece391_halt (uint8_t status);
extern int32_t ece391_execute (const uint8_t* command);
extern int32_t ece391_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t ece391_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t ece391_open (const uint8_t* filename);
extern int32_t ece391_close (int32_t fd);
extern int32_t ece391_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t ece391_vidmap (uint8_t** screen_start);
extern int32_t ece391_malloc (uint32_t nbytes);
extern int32_t ece391_free (void* ptr);
extern int32_t ece391_ioctl (int32_t fd, uint32_t command, uint32_t args);

#endif /* _ECE391SYSCALL_H_ */

