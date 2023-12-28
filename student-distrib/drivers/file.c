#include "file.h"
#include "../lib.h"
#include "../proc/PCB.h"

file_system_t file_system;
int8_t delete_mode = 0; /* 0 -- do not delete. 1 -- delete.*/

/* File op table for PCB jumps, see types.h */
device_op_table_t file_op_table = {
    file_read,
    file_write,
    file_open,
    file_close,
    file_ioctl
};

/* Dir op table for PCB jumps, see types.h */
device_op_table_t dir_op_table = {
    dir_read,
    dir_write,
    dir_open,
    dir_close
};

int cur_dir; /* If we dir_read rather than file_read, this keeps track of which file we're on.*/

/**
 * read_dentry_by_name
 * DESCRIPTION: Read a dentry by the given file name
 * 
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if (strlen((int8_t*)fname) > FILENAME_LEN) {
        return -1;
    }

    if (dentry == NULL) {
        return -1;
    }
    if (fname == NULL){ 
        return -1;
    }
    boot_block_t * bblock = file_system.boot_block;
    /* Loop through the boot block's dentries, and terminate if we find the filename matching or if we can't find the filename*/
    int dir_idx; /* directory index*/
    for(dir_idx = 0; dir_idx < bblock->dir_count; dir_idx ++) {
        dentry_t cur_dentry = bblock->direntries[dir_idx]; // get current dentry
        const uint8_t * dentry_fname = cur_dentry.filename;
        // strncpy(dentry_fname, cur_dentry.filename, strlen(cur_dentry.filename <= FILENAME_LEN ) ? strlen(cur_dentry.filename) : FILENAME_LEN); // get that dentry's filename
        
        // if (strlen((int8_t*)dentry_fname) != strlen((int8_t*)fname)) { // if the lengths of the dentry's filename doesn't match the length of the fname, continue;
        //     continue;
        // }



        int cmp_res = strncmp((int8_t*)fname, (int8_t*)dentry_fname, FILENAME_LEN); //compare the two strings
        if (cmp_res == 0) {
            strncpy((int8_t*)dentry->filename, (int8_t*)fname, FILENAME_LEN); // copy cur_dentry file name into dentry filename
            dentry->filetype = cur_dentry.filetype;
            dentry->inode_num  = cur_dentry.inode_num;
            return 0;
        }
    }
    return -1;

}

/**
 * read_dentry_by_index
 * DESCRIPTION: Read a dentry by the given index
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if (dentry == NULL) {
        return -1;
    }
    boot_block_t * bblock = file_system.boot_block;
    if (index < 0 || index >= bblock->dir_count) {
        return -1;
    }

        dentry_t cur_dentry = bblock->direntries[index]; /* get current dentry*/
        strcpy((int8_t*)dentry->filename, (int8_t*)cur_dentry.filename); // copy cur_dentry file name into dentry filename
        dentry->filetype = cur_dentry.filetype;
        dentry->inode_num  = cur_dentry.inode_num;
        return 0;
        
    
    
}

/**
 * read_data
 * DESCRIPTION: Read a data blocks for a given inode
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    boot_block_t * bblock = file_system.boot_block;
    if (buf == NULL) {
        return -1;
    }
    if (inode >= bblock->inode_count || inode < 0) {
        return -1;
    }
    inode_t* this_inode= &file_system.inode_base[inode]; /* Get this inode */

    /*Two levels of indirection for data blocks. */
    int num_bytes = this_inode->length; /*number of bytes for this inode*/
    int32_t dblock_indirect_idx; /* indirect index to access where in the data_block_num field of the inode*/
    if (offset < BLOCK_SIZE) {
        dblock_indirect_idx = 0;
    } else { 
        dblock_indirect_idx = offset / BLOCK_SIZE;
    }

    int32_t dblock_idx = this_inode->data_block_num[dblock_indirect_idx]; /*index to access which data block*/
    data_block_t* dblock= &file_system.data_block_base[dblock_idx] ; /*the current data block*/
    
    int cur_byte = offset; /* Current byte out of total bytes of file*/
    int cur_pos = (offset < BLOCK_SIZE) ? offset : offset % BLOCK_SIZE; /*current position within the 4096-byte block */
    uint8_t * start_pos_src = dblock->bytes + offset % BLOCK_SIZE; /* Find start_pos of the source where we copy data from*/
    uint8_t * start_pos_dst = buf;                   /* Find start_pos of the source where we copy data to*/

    uint32_t len_counter; /* len_counter for for loop, capped at length*/
    uint32_t num_bytes_read = 0; /* num_bytes read*/
    for (len_counter = 0; len_counter < length; len_counter ++){
        if (cur_byte >= num_bytes) {
            break;
        }
        memcpy(start_pos_dst, start_pos_src, 1); /* copy one byte at a time into the buffer*/
        num_bytes_read += 1;
        cur_pos += 1;
        cur_byte += 1;
        start_pos_dst += 1;
        start_pos_src += 1;
        if (cur_pos == BLOCK_SIZE) {
            dblock_indirect_idx += 1;
            dblock_idx = this_inode->data_block_num[dblock_indirect_idx];
            dblock = &file_system.data_block_base[dblock_idx];
            start_pos_src = dblock->bytes;
        }

    }

    return num_bytes_read;
}

int32_t get_avail_data_block_num() {
    int i;
    for (i = 0; i < MAX_FILES * MAX_DATA_BLOCKS; i++) {
        if (file_system.data_block_flags[i] == 0) {
            return i;
        }
    }
    return -1; /* no possible free data blocks*/

}
int32_t get_avail_inode_idx() {
    int i;
    for (i = 0; i < MAX_FILES; i ++) {
        if (file_system.inode_flags[i] == 0) {

            return i;
        }
    }
    return -1; /* no possible free inodes*/
}

int32_t write_dentry_by_name (const uint8_t * fname, dentry_t * dentry) {
    if (fname == NULL) {
        return -1;
    }
    if (dentry == NULL) {
        return -1;
    }

    boot_block_t * bblock = file_system.boot_block;
    uint32_t num_direntries = bblock->dir_count;
    int dir_idx;
    for ( dir_idx = 0; dir_idx < num_direntries; dir_idx ++) {
        dentry_t cur_dentry = bblock->direntries[dir_idx];
        const uint8_t * dentry_fname = cur_dentry.filename;
        
        int cmp_res = strncmp((int8_t*)fname, (int8_t*)dentry_fname, FILENAME_LEN); //compare the two strings
        if (cmp_res == 0) {
            strncpy((int8_t*)dentry->filename, (int8_t*)fname, FILENAME_LEN); // copy cur_dentry file name into dentry filename
            dentry->filetype = cur_dentry.filetype;
            dentry->inode_num  = cur_dentry.inode_num;
            return 0;
        }
    }
    /* We didn't find a matching file name. This means we are not writing to an existing file but rather creating a new file*/
    if (num_direntries >= MAX_FILES) {
        return -1; /* Reached max files*/
    } 

    int32_t new_inode_num = get_avail_inode_idx(); /* Get next available inode index*/
    file_system.inode_flags[new_inode_num] = 1; /* Mark as occupied now*/
    if (new_inode_num == -1)return -1;

    /* update the boot block's dentries */
    // bblock->direntries[bblock->dir_count];
    strncpy(bblock->direntries[bblock->dir_count].filename, (int8_t*)fname, FILENAME_LEN );
    bblock->direntries[bblock->dir_count].filetype = FILE_TYPE_REG; /* Assuming I'm not working with a directory or the RTC...  idk what to do for that */
    bblock->direntries[bblock->dir_count].inode_num = new_inode_num;

    /* Then copy to the passed in dentry */
    strncpy((int8_t*)dentry->filename, (int8_t*)fname, FILENAME_LEN); // copy cur_dentry file name into dentry filename
    dentry->filetype = bblock->direntries[bblock->dir_count].filetype;
    dentry->inode_num  = bblock->direntries[bblock->dir_count].inode_num;

    bblock->dir_count ++;

    return 0;

}
int32_t write_dentry_by_index(uint32_t index, dentry_t * dentry) {
    return 0;
}

/**
 * We need to return the number of bytes written... for now??
 * DESCRIPTION: If we find a data_block_num = 0, then find a new data block, and start writing to that.
 *              Otherwise, write to the current data block.
*/
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){
    if (delete_mode == 1) {

        int res = delete_data(inode, length);
        delete_mode = 0;
        return res;
    }


    boot_block_t * bblock = file_system.boot_block;
    if (inode >= bblock->inode_count) {
        return -1;
    }
    inode_t * inode_block = &file_system.inode_base[inode];
    uint32_t initial_length = inode_block->length; /* Initial length*/
   
   
    /* We should notice that 0 is never used for a data block number... this could be slightly problematic,
    as this deprives us of an additional 4096 bytes, but I will gloss over this for now.                    */

   
    int32_t dblock_indirect_idx; /* indirect index to access where in the data_block_num field of the inode*/
    if (offset < BLOCK_SIZE) {
        dblock_indirect_idx = 0;
    } else { 
        dblock_indirect_idx = offset / BLOCK_SIZE;
    }
    if (dblock_indirect_idx >= MAX_DATA_BLOCKS) { /* Used up all the data blocks. */
        return -1;
    }
    uint32_t num_bytes = inode_block->length; /* total number of bytes of the file*/
    int32_t dblock_idx = inode_block->data_block_num[dblock_indirect_idx]; /*index to access which data block*/
    if (dblock_idx == 0) { /* The data block number is 0. This means that we need to get a new data block.*/
        dblock_idx = get_avail_data_block_num();
        if (dblock_idx == -1) {
            return 0; /* No more data blocks. Return.*/
        }
        inode_block->data_block_num[dblock_indirect_idx] = dblock_idx;
        file_system.data_block_flags[dblock_idx] = 1; /* Mark this as occupied.*/
    }

    data_block_t* dblock= &file_system.data_block_base[dblock_idx] ; /*the current data block*/
    
    int cur_byte = offset; /* Current byte out of total bytes of file*/
    int cur_pos = (offset < BLOCK_SIZE) ? offset : offset % BLOCK_SIZE; /*current position within the 4096-byte block */
    uint8_t * start_pos_dst = dblock->bytes + offset % BLOCK_SIZE; /* Find start_pos of the destination where we copy data TO*/
    uint8_t * start_pos_src = buf;                   /* Find start_pos of the source where we copy data from*/

    uint32_t len_counter; /* len_counter for for loop, capped at length*/
    uint32_t num_bytes_written = 0; /* num_bytes written*/
    for (len_counter = 0; len_counter < length; len_counter ++){
        
         if (cur_pos == BLOCK_SIZE) { /* We've reached the end of a data block. Move on to the next data block.*/
            dblock_indirect_idx += 1;
            dblock_idx = inode_block->data_block_num[dblock_indirect_idx];
            if (dblock_idx == 0) { /* We've reached all of our currently used data blocks. Find a brand-new one. */
                dblock_idx = get_avail_data_block_num();
                if (dblock_idx == -1) {
                    return num_bytes_written; /* No more data blocks. Return.*/
                }
                inode_block->data_block_num[dblock_indirect_idx] = dblock_idx;
                file_system.data_block_flags[dblock_idx] = 1;/* Marked this as occupied. */
            }


            dblock = &file_system.data_block_base[dblock_idx];
            start_pos_dst = dblock->bytes;
        }
        memcpy(start_pos_dst, start_pos_src, 1); /* copy one byte at a time into the buffer*/
        num_bytes_written += 1;
        if (offset + num_bytes_written >= inode_block->length) { /* If we end up writing past the original length of the file*/
            inode_block->length = offset + num_bytes_written;
        }
        cur_pos += 1;
        cur_byte += 1;
        start_pos_dst += 1;
        start_pos_src += 1;
       

    }

    return num_bytes_written;
   

     
}

/* Helper for backspace -- offset is always assumed to be the length of the file*/
int32_t delete_data(uint32_t inode, uint32_t length) {
    
    boot_block_t * bblock = file_system.boot_block;
    if (inode >= bblock->inode_count) {
        return -1;
    }
    inode_t * inode_block = &file_system.inode_base[inode];
    uint32_t initial_length = inode_block->length; /* Initial length*/
   
   
    /* We should notice that 0 is never used for a data block number... this could be slightly problematic,
    as this deprives us of an additional 4096 bytes, but I will gloss over this for now.                    */

   
    int32_t dblock_indirect_idx; /* indirect index to access where in the data_block_num field of the inode*/
   
    dblock_indirect_idx = initial_length / BLOCK_SIZE;
    
    uint32_t num_bytes = inode_block->length; /* total number of bytes of the file*/
    int32_t dblock_idx = inode_block->data_block_num[dblock_indirect_idx]; /*index to access which data block*/
   

    data_block_t* dblock= &file_system.data_block_base[dblock_idx] ; /*the current data block*/
    
    int cur_byte = num_bytes; /* Current byte out of total bytes of file*/
    int cur_pos = (num_bytes < BLOCK_SIZE) ? num_bytes : num_bytes % BLOCK_SIZE; /*current position within the 4096-byte block */
    uint8_t * start_pos_dst = dblock->bytes + num_bytes % BLOCK_SIZE; /* Find start_pos of the destination where we copy data TO*/

    uint32_t len_counter; /* len_counter for for loop, capped at length*/
    uint32_t num_bytes_written = 0; /* num_bytes written*/
    for (len_counter = 0; len_counter < length; len_counter ++){
        
         if (cur_pos < 0) { /* We've reached the end of a data block. Move on to the next data block.*/
            if (dblock_indirect_idx == 0) { /* We've cleared the whole file! */
                return num_bytes_written;
            }
            file_system.data_block_flags[dblock_idx] = 0; /* Mark as unoccupied*/
            inode_block->data_block_num[dblock_indirect_idx] = 0;
            dblock_indirect_idx -= 1;
            dblock_idx = inode_block->data_block_num[dblock_indirect_idx];
            dblock = &file_system.data_block_base[dblock_idx];
            start_pos_dst = dblock->bytes[BLOCK_SIZE - 1];
        }
        *start_pos_dst = 0;
        num_bytes_written += 1;
        inode_block->length -= 1;
        cur_pos -= 1;
        cur_byte -= 1;
        start_pos_dst -= 1;
       

    }

    return num_bytes_written;


}

/* parses the file system */
uint32_t parse_filesystem (uint32_t mod_start) {
    /* null pointer checks */
    if (!mod_start) return -1;
    memset(file_system.inode_flags, 0, MAX_FILES);
    memset(file_system.data_block_flags, 0 ,MAX_FILES * MAX_DATA_BLOCKS);
    file_system.data_block_flags[0] = 1; /* 0 is never used for a data block number*/ 


    /* Setting the pointers to the appropriate blocks */
    file_system.boot_block = (boot_block_t*)mod_start;
    /* The first inode is one after the boot block --- Should we just set these to the file_system member variables or do it like this for byte offsets?*/
    file_system.inode_base = (inode_t*)file_system.boot_block + 1;
    file_system.data_block_base = (data_block_t*)(file_system.inode_base + file_system.boot_block->inode_count);
    
    
    int dir_idx;
    for (dir_idx = 0; dir_idx < file_system.boot_block->dir_count; dir_idx ++) {
        int inode_num = file_system.boot_block->direntries[dir_idx].inode_num;
        file_system.inode_flags[inode_num] = 1; /* in use*/
        inode_t cur_inode = file_system.inode_base[inode_num];
        int dblock_indirect_idx = 0;
        int dblock_idx = cur_inode.data_block_num[dblock_indirect_idx];
        while (dblock_idx != 0) {
            file_system.data_block_flags[dblock_idx] = 1;
            dblock_indirect_idx += 1;
            dblock_idx = cur_inode.data_block_num[dblock_indirect_idx];

        }
        
    }

    
    /* Success. */
    return 0;
}


int32_t file_ioctl(int32_t fd, uint32_t command, uint32_t arg1)
{
    /* Check file descriptor */
    if (fd < 0) {return -1;}

    switch(command) {
        case CREATE_NEW_FILE: {
          return create_new_file( (uint8_t *) arg1);

        }
        case CLEAR_FILE: {
            return clear_file( (uint8_t * )arg1);
        }
        case SET_FILE_POS_CUR_LENGTH:{
            return set_file_pos_cur_length(arg1);
        }
        case SDM:
        {
            return sdm(arg1);
        }
        default:
            break;
    }

    return 0;
}

int32_t create_new_file(uint8_t * fname) {
    if (fname == NULL) {
        return -1;
    }
    

    boot_block_t * bblock = file_system.boot_block;
    uint32_t num_direntries = bblock->dir_count;
    /* We didn't find a matching file name. This means we are not writing to an existing file but rather creating a new file*/
    if (num_direntries >= MAX_FILES) {
        return -1; /* Reached max files*/
    } 

    int32_t new_inode_num = get_avail_inode_idx(); /* Get next available inode index*/
    file_system.inode_flags[new_inode_num] = 1; /* Mark as occupied now*/
    if (new_inode_num == -1)return -1;

    /* update the boot block's dentries */
    // bblock->direntries[bblock->dir_count];
    strncpy(bblock->direntries[bblock->dir_count].filename, (int8_t*)fname, FILENAME_LEN );
    bblock->direntries[bblock->dir_count].filetype = FILE_TYPE_REG; /* Assuming I'm not working with a directory or the RTC...  idk what to do for that */
    bblock->direntries[bblock->dir_count].inode_num = new_inode_num;
    bblock->dir_count ++;

    return 0;
}


int32_t clear_file(uint8_t * fname)  {
    if (fname == NULL) {
        return -1;
    }

    dentry_t temp_dentry;
    write_dentry_by_name(fname, &temp_dentry) ; /* Fill in the dentry, which we use to get the inode number*/
    int inode_num = temp_dentry.inode_num; /* Get the inode number*/
    inode_t * inode = &file_system.inode_base[inode_num]; /* Get the inode */
    int num_data_blocks = inode->length / BLOCK_SIZE; /* Find number of data blocks to clear*/
    if (inode->length % BLOCK_SIZE > 0) {
        num_data_blocks ++;
    }

    int d;
    for (d = 0 ; d < num_data_blocks ;d ++ ){
        int dblock_idx = inode->data_block_num[d];
        data_block_t * cur_block = &file_system.data_block_base[dblock_idx];
        memset( cur_block->bytes, 0 , BLOCK_SIZE); /* Clear the data blocks*/
        inode->data_block_num[d] = 0;
    }

    inode->length = 0;
 
    
    return 0;    

}

int32_t set_file_pos_cur_length(int fd) {
    int inode_num = pcb_get_inode(fd);
    if (inode_num == -1) return -1;
    int length = file_system.inode_base[inode_num].length;
    pcb_set_file_pos(fd, length);
    return 0;
}

/* Set delete mode */
int32_t sdm(int fd) {
    delete_mode = 1;
    return 0;
}
/* Reads n number of bytes from a file into the buffer */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    int32_t file_pos = pcb_get_file_pos(fd);
    int32_t inode = pcb_get_inode(fd);
    if (-1 == file_pos) return -1;
    if (-1 == inode) return -1;
    /* For files, return 0 if we are at or beyond the EOF */
    ///TODO: this should go into read_data
    if (file_pos >= (file_system.inode_base+inode)->length) return 0;
    int num_bread = read_data(inode, file_pos, buf,nbytes);
    file_pos += num_bread;
    pcb_set_file_pos(fd,file_pos);
    return num_bread ;
}

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t file_pos = pcb_get_file_pos(fd);
    int32_t inode = pcb_get_inode(fd);
    if (-1 == file_pos) return -1;
    if (-1 == inode) return -1;
    int num_bread = write_data(inode, file_pos, buf,nbytes);
    if (! delete_mode) {
    file_pos += num_bread;
    } else {
        file_pos -= num_bread;
        if (file_pos < 0 ) file_pos = 0;
    }
    pcb_set_file_pos(fd,file_pos);
    return num_bread ;
}

int32_t file_open(const uint8_t* filename) {
    delete_mode = 0;
    return 0;
    /* Otherwise open the file */
    //return PCB_open(0, dentry.inode_num);
}

int32_t file_close(int32_t fd) {
    delete_mode = 0;
    return 0;
    //return PCB_close(fd);
}

/* Read number of bytes from a directory */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    if (cur_dir < 0 || cur_dir >= file_system.boot_block->dir_count) {
        return 0;
    }
    dentry_t dentry;
    int ret = read_dentry_by_index(cur_dir, &dentry);
    if (ret == -1) {
        return 0;
    }
    uint8_t* name_of_file = dentry.filename;
    strncpy((int8_t*)buf, (int8_t*)name_of_file, FILENAME_LEN + 1);
    cur_dir += 1;
    if (strlen(buf) < FILENAME_LEN) {
        return strlen(buf);
    } else {
        return FILENAME_LEN;
    }
}

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


int32_t dir_open(const uint8_t* filename) {
    cur_dir = 0;
    return 0;
}

int32_t dir_close(int32_t fd) {
    cur_dir = 0;
    return 0;
}
