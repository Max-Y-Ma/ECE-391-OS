/* PCB.h - 
 * vim:ts=4 noexpandtab
 */

#ifndef _PCB_H
#define _PCB_H

#include "../types.h"
#include "../x86_desc.h"

#define MAX_TASKS 6
#define KSTACK_SIZE 8192
#define KERNEL_BOTTOM 0x800000

/* We can have up to 8 open files */
#define FILE_ARRAY_SIZE 8

/* Maximum length of args for execute is 1024 */
#define MAX_ARGS 1024

#define FLAG_IN_USE 0x1

typedef struct file_descriptor_t {
    device_op_table_t* op_table;
    int32_t inode;
    int32_t file_pos;
    int32_t flags;
} fd_t;

typedef struct __attribute__((packed)) pcb_t {
    uint8_t active;
    uint32_t id;
    fd_t file_array[FILE_ARRAY_SIZE]; /* */
    struct pcb_t* parent_pcb;
    uint32_t old_ebp;
    uint32_t switch_ebp;
    uint8_t args[MAX_ARGS];
    int32_t tcb_idx;
    uint32_t rtc_freq;
    uint32_t rtc_val;
    uint32_t rtc_int_occ;
} pcb_t;

extern pcb_t* get_curr_pcb(void);

/**
 * @brief Entry point for an open syscall
 * 
 * @param filename name of the file/driver
 * @return -1 on fail, fd index on success
*/
int32_t pcb_open(const uint8_t* filename);

/**
 * @brief Entry point for a close syscall
 * 
 * @param fd file descriptor index
 * @return -1 on fail, 0 on success
*/
int32_t pcb_close(int32_t fd);

/**
 * @brief Close all open fds
 * 
 * @return -1 on fail, 0 on success
*/
int32_t pcb_close(int32_t fd);

/**
 * @brief Entry point for a read syscall
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data to
 * @param nbytes number of bytes to write to the buffer
 * @return -1 on fail, number of bytes placed in buffer on success
*/
int32_t pcb_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Entry point for a write syscall
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data from
 * @param nbytes number of bytes to write from the buffer
 * @return -1 on fail, number of bytes written on success
*/
int32_t pcb_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Entry point for a ioctl syscall
 * 
 * @param fd : File descriptor for device
 * @param command : Device ioctl command 
 * @param args : Ioctl arguments, multiple arguments should be stored in a buffer
 * @return 0 on sucessful operation, -1 on failed operation 
*/
int32_t pcb_ioctl(int32_t fd, uint32_t command, uint32_t args);

/**
 * @brief Grabs the file position from a given fd index.
 * @warning Assumes you have selected the correct pcb beforehand
 * 
 * @param fd file descriptor index
 * @return -1 on fail, file position on success
 */
int32_t pcb_get_file_pos(int32_t fd);

/**
 * @brief Sets the file position from a given fd index.
 * @warning Assumes you have selected the correct pcb beforehand
 * 
 * @param fd file descriptor index
 * @param pos position to set 
 * @return nothing
 */
void pcb_set_file_pos(int32_t fd, int32_t pos) ;

/**
 * @brief Grabs the inode from a given fd index.
 * @warning Assumes you have selected the correct pcb beforehand
 * 
 * @param fd file descriptor index
 * @return -1 on fail, inode on sucess
 */


int32_t pcb_get_inode(int32_t fd);

/**
 * @brief Sets the inode from a given fd index.
 * @warning Assumes you have selected the correct pcb beforehand
 * 
 * @param fd file descriptor index
 * @param inode_num inode_num to set
 * @return 
 */

void pcb_set_inode(int32_t fd, int32_t inode_num);

/**
 * @brief Checks if the fd is in bounds and if that fd index is in use
 * 
 * @param fd file descriptor index
 * @return -1 on fail, 0 on success
 */
int32_t pcb_check_valid_fd(int32_t fd);

/**
 * @brief Empties the pcb and activates it
 * 
 * @param pcb pcb to initialize
 * @return -1 on fail, 0 on success
 */
int32_t pcb_init(pcb_t* pcb);

/**
 * @brief Empties the pcb
 * 
 * @param pcb pcb to destroy
 * @return -1 on fail, 0 on success
 * @warning Make sure to close all open files before you destroy!!
 */
int32_t pcb_destroy(pcb_t* pcb);

/**
 * @brief Opens stdin and stdout
 * 
 * @param pcb pcb to initialize
 * @return -1 on fail, 0 on success
 */
int32_t pcb_std(pcb_t* pcb);

/**
 * @brief Allocates space for a new pcb and initializes it
 * 
 * @return pointer to the alloced pcb, NULL if failed
 */
pcb_t* alloc_pcb(void);

/**
 * @brief Finds the current pcb of the process
 * 
 * @return pointer to the pcb
 */
extern pcb_t* get_curr_pcb(void);

/**
 * @brief Empties all memory where pcbs are supposed to be.
 * 
 */
void initialize_all_pcbs(void);

/**
 * @brief Close all open file descriptors
*/
int32_t pcb_close_all_open(void);

#endif /* _PCB_H */
