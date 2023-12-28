#include "loader.h"
#include "lib.h"

static elf_header_info_t elf_header;

/* This will contain the memory address of the entry point. When loading this, it must be casted as a pointer. */
uint32_t entry_point;

/**
 * DESCRIPTION: This function reads the filename for an executable, then fills in a struct called the elf_header.
 * Then, it also gets the entry point, which is the page address of where the program image should be loaded in memory.
 * WARNING: This assumes that the file name is an executable for a system call
 * INPUTS: filename -- name of file (will be for a system call)
 * OUTPUTS: 0 on success, -1 on failure
 * 
*/
int read_header(const uint8_t *filename) {
    if (filename == NULL){
        return -1;
    }
    dentry_t dentry;
    int ret = read_dentry_by_name((uint8_t*)filename, &dentry);
    if (ret == -1) {
        return -1;
    }
    int inode_idx = dentry.inode_num;

    /* We can't use a PCB here... we need some way to read the first */
    ret = read_data(inode_idx, 0, (uint8_t *)&elf_header, ELF_HEADER_SIZE);
    if (ret == -1) {
        return -1;
    }
    if (elf_header.ei_magic[0] != FIRST_BYTE || elf_header.ei_magic[1] != SECOND_BYTE || elf_header.ei_magic[2] != THIRD_BYTE || elf_header.ei_magic[3] != FOURTH_BYTE) {
        return -1;
    } 
    if (dentry.filetype != FILE_TYPE_REG) {
        return -1;
    }
    entry_point = (elf_header.e_entry[3] << 24 ) | (elf_header.e_entry[2] << 16) | (elf_header.e_entry[1] << 8) | (elf_header.e_entry[0]); /* this bitshifting ensures that the entry point is the correct value
                                                                                                                                            (left shifting 24 moves into 3rd byte, 16 moves into 2nd byte, 8 into 1st byte, 0 into 0th byte)   */
    return 0;
}
/**
 * DESCRIPTION:
 * WARNING: This assumes that the page has already been set up
 *  INPUTS: filename -- name of file
 *          prog_mem -- page where we copy the ELF file's data to (thus loading the program image at)
 * OUTPUTS: -1 on failure, 0 on success
*/
int load_program(const uint8_t *filename,  uint8_t *prog_mem) {
    if (filename == NULL){ 
        return -1;
    }

    dentry_t dentry;
    int ret;
    if ( (ret = read_dentry_by_name((uint8_t*)filename, &dentry) )== -1) {
        return -1;
    }
    int inode_idx = dentry.inode_num;
    int length = file_system.inode_base[inode_idx].length;

    /* Adjustment for fish program */
    ret = read_data(inode_idx, 0, prog_mem, length); /* length is not the exact amount, but read_data should account for going past the file*/

    return 0;
}
