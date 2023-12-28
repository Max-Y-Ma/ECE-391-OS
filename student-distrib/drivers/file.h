#include "../types.h"
#include "../multiboot.h"

#define FILENAME_LEN 32 /*How long do I make the file name length*/
#define BLOCK_SIZE 4096
#define FILE_TYPE_DIR 1
#define FILE_TYPE_REG 2
#define FILE_TYPE_RTC 0
#define MAX_FILES 63
#define MAX_DATA_BLOCKS 1023 /* number of datablocks in a single inode*/
#define CREATE_NEW_FILE 0
#define CLEAR_FILE 1
#define SET_FILE_POS_CUR_LENGTH 2
#define SDM 3
typedef struct {
    uint8_t filename[FILENAME_LEN];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[24]; /* Need 24 reserved bytes of a dentry*/

} dentry_t;

typedef struct  {
    uint32_t dir_count; /*directory count*/
    uint32_t inode_count; /*count of inode blocks*/
    uint32_t data_count; /*count of data blocks*/
    unsigned char reserved[52]; /* Need 52 Reserved bytes of the boot block*/
    dentry_t direntries[63]; /*We can store 63 Directory entries */
} boot_block_t;

typedef struct {
    uint32_t length; /* size of the file in bytes*/
    uint32_t data_block_num[MAX_DATA_BLOCKS]; /* number of 4kB (4096 bytes) data blocks */
} inode_t;

typedef struct {
    uint8_t bytes[BLOCK_SIZE];
} data_block_t;

typedef struct file_system_t {
    boot_block_t* boot_block; /* pointer to the boot block */
    inode_t* inode_base; /* pointer to the first inode */
    data_block_t* data_block_base; /* pointer to the first data block */
    int8_t inode_flags[MAX_FILES]; /* If an inode # (out of 63) is used by a certain file*/         
                                    /* 1 means occupied, 0 means not occupied */
    int8_t data_block_flags[MAX_FILES * MAX_DATA_BLOCKS]; /* If a datablock # (out of 63 * 1023) is used by an inode*/
                                                          /* 1 means occupied, 0 means not occupied*/

} file_system_t;

extern file_system_t file_system;

/* Read a dentry by the given file name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
/*  Read a dentry by the given index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
/* Read a data blocks for a given inode */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t write_dentry_by_name (const uint8_t * fname, dentry_t * dentry);
int32_t write_dentry_by_index(uint32_t index, dentry_t * dentry);
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

int32_t get_avail_data_block_num();

int32_t get_avail_inode_idx();

int32_t file_ioctl(int32_t fd, uint32_t command, uint32_t args );

int32_t create_new_file( uint8_t * fname);

int32_t clear_file(uint8_t * fname);

int32_t set_file_pos_cur_length(int fd);

int32_t sdm(int forget);

int32_t delete_data(uint32_t inode, uint32_t length); 

/**
 * 
 * 
 * @brief Parses the filesystem structure into struct architecture. Look at file.h
 * 
 * @param mod_start pointer to the start of the module
 * @return -1 on fail, 0 on success
*/
uint32_t parse_filesystem (uint32_t mod_start);

/**
 * @brief Reads data from a file into the buffer
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data to
 * @param nbytes number of bytes to write to the buffer
 * @return -1 on fail, number of bytes placed in buffer on success
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Does nothing, returns -1
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data from
 * @param nbytes number of bytes to write from the buffer
 * @return -1
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Initializes any necessary structures (none for now)
 * 
 * @param filename name of the file to open
 * @return -1 on fail, 0 on success
*/
int32_t file_open(const uint8_t* filename);

/**
 * @brief Closes any structures created by file_open (none for now)
 * 
 * @param fd file descriptor index
 * @return -1 on fail, 0 on success
*/
int32_t file_close(int32_t fd);

/**
 * @brief Reads data from a directory into a buffer
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data to
 * @param nbytes number of bytes to write to the buffer
 * @return -1 on fail, number of bytes placed in buffer on success
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Does nothing, returns -1
 * 
 * @param fd file descriptor index
 * @param buf buffer to write data from
 * @param nbytes number of bytes to write from the buffer
 * @return -1
*/
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Initializes any necessary structures (none for now)
 * 
 * @param filename name of the file to open
 * @return -1 on fail, 0 on success
*/
int32_t dir_open(const uint8_t* filename);

/**
 * @brief Closes any structures created by dir_open (none for now)
 * 
 * @param fd file descriptor index
 * @return -1 on fail, 0 on success
*/
int32_t dir_close(int32_t fd);

extern device_op_table_t file_op_table;
extern device_op_table_t dir_op_table;
