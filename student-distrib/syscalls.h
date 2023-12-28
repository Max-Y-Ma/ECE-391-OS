#ifndef __SYSCALLS_H_
#define __SYSCALLS_H_

#include "types.h"

/**
 * @brief Terminates a process, returning the specified value to its
 *        parent process. The system call handler itself is responsible
 *        for expanding the 8-bit argument from BL into the 32-bit
 *        return value to the parent program's execute system call.
 * 
 * @details This system call should never return to the caller.
 * 
 * @param status Return value to parent process
 * 
 * @return Never returns to the parent process
*/
int32_t system_halt (uint32_t status);

int32_t system_halt_helper (void);

/**
 * @brief Attempts to load and execute a new program, handing off the
 *        processor to the new program until it terminates.
 * 
 * @param command A space-separated sequence of words. The first word is the file name
 *                of the program to be executed. The rest of the command -- stripped
 *                of leading spaces -- should be provided to the new program on request
 *                via the getargs system call. 
 * 
 * @return -1 if the command cannot be executed,
 *         256 if the program dies by an exception 
 *         0-255 if the program executes a halt system call (value from halt)
*/
int32_t system_execute (const uint8_t* command);

/**
 * @brief The read system call reads data from the keyboard, a file, 
 *        device (RTC), or directory. You should use a jump table referenced
 *        by the task’s file array to call from a generic handler for this call 
 *        into a file-type-specific function. This jump table should be inserted 
 *        into the file array on the open system call (see below).
 * 
 * @param fd file descriptor, index into the file array in PCB
 * @param buf recieve buffer
 * @param nbytes number of bytes to be read from the file/device
 * 
 * @return 0 if the initial file position is at or beyond the end of the file. 
*/
int32_t system_read (int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief The write system call writes data to the terminal or to a device (RTC). 
 * 
 * @param fd file descriptor, index into the file array in PCB
 * @param buf write buffer
 * @param nbytes number of bytes to be written into the file/device
 * 
 * @return Writes to regular files should always return -1 to indicate failure 
 *         since the file system is read-only. The call returns the number of bytes
 *         written, or -1 on failure.
*/
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief The open system call provides access to the file system. 
 *        The call should find the directory entry corresponding to the
 *        named file, allocate an unused file descriptor, and set up any data 
 *        necessary to handle the given type of file (directory, RTC device, or regular file).
 * 
 * @param filename Name of the file to be opened
 * 
 * @return If the named file does not exist or no descriptors are free, the call returns -1.
*/
int32_t system_open (const uint8_t* filename);

/**
 * @brief The close system call closes the specified file descriptor and makes 
 *        it available for return from later calls to open. You should not allow 
 *        the user to close the default descriptors (0 for input and 1 for output).
 * 
 * @param fd file descriptor
 * 
 * @return Trying to close an invalid descriptor should result in a return value of -1; 
 *         successful closes should return 0.
*/
int32_t system_close (int32_t fd);

/**
 * @brief The getargs call reads the program’s command line arguments into a user-level buffer. 
 *        Obviously, these arguments must be stored as part of the task data when a new 
 *        program is loaded. Here they are merely copied into user space. 
 * 
 * @details The shell does not request arguments, but you should probably still initialize 
 *           he shell task’s argument data to the empty string.
 * 
 * @param buf User-space buffer for arguments
 * @param nbytes Size of the buffer
 * 
 * @return If there are no arguments, or if the arguments and a terminal NULL (0-byte) do not fit in 
 *        the buffer, simply return -1. 
*/
int32_t system_getargs (uint8_t* buf, int32_t nbytes);

/**
 * @brief The vidmap call maps the text-mode video memory into user space at a pre-set 
 *        virtual address. Although the address returned is always the same, it should 
 *        be written into the memory location provided by the caller 
 *        (which must be checked for validity). 
 * 
 * @details To avoid adding kernel-side exception handling for this sort of check, 
 *          you can simply check whether the address falls within the address range 
 *          covered by the single user-level page. Note that the video memory will 
 *          require you to add another page mapping for the program, 
 *          in this case a 4 kB page. It is not ok to simply change the permissions 
 *          of the video page located < 4MB and pass that address.
 * 
 * @param screen_start User-space virtual address for the video memory
 * 
 * @return If the location is invalid, the call should return -1.
*/
int32_t system_vidmap (uint8_t** screen_start);

int32_t system_set_handler (int32_t signum, void* handler_address);
int32_t system_sigreturn (void);

/**
 * @brief The malloc system call allows users to dynamically allocated memory.
 * 
 * @param nbytes Number of bytes requested
 * 
 * @return Address of newly allocated memory, otherwise NULL
*/
int32_t system_malloc(uint32_t nbytes);

/**
 * @brief The free system call allows users to dynamically deallocated memory.
 * 
 * @param ptr Pointer to previously allocated memory
 * 
 * @return If the pointer is invalid, the call should return -1.
*/
int32_t system_free(void* ptr);

/**
 * @brief Ioctl system call allows users to call driver specific operations
 * 
 * @param fd : File descriptor for device
 * @param command : Device ioctl command 
 * @param args : Ioctl arguments, multiple arguments should be stored in a buffer
 * 
 * @return 0 on sucessful operation, -1 on failed operation
*/
int32_t system_ioctl(int32_t fd, uint32_t command, uint32_t args);

/* IRET context switch to user program */
extern void execute_context_switch(void);

#endif  /* __SYSCALLS_H_ */
