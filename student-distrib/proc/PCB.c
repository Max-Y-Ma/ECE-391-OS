#include "PCB.h"
#include "../drivers/file.h"
#include "../drivers/keyboard.h"
#include "../drivers/terminal.h"
#include "../drivers/RTC.h"
#include "../lib.h"

static const pcb_t empty_pcb = {0};

int32_t pcb_open(const uint8_t* filename) {
    pcb_t* pcb = get_curr_pcb();
    if (!pcb || !filename) {
        KDEBUG("ERROR: Failed to call pcb_open!\n");
        return -1;
    }
    dentry_t dentry;
    int i;

    /* If the file doesn't exist, return -1 */
    if (!strncmp((int8_t*)filename, (int8_t*)"rtc", 3)) {
        dentry.filetype = 0;
        dentry.inode_num = 0;
    } else if (-1 == read_dentry_by_name(filename, &dentry)) return -1;
    /* Find a free spot in the array */
    for (i=0;i<FILE_ARRAY_SIZE;i++) {
        if (!(pcb->file_array[i].flags & FLAG_IN_USE)) {
            /* Indicate that the spot is in use */
            pcb->file_array[i].flags |= FLAG_IN_USE;

            /* RTC or dir or file */
            switch (dentry.filetype) {
                case 0: pcb->file_array[i].op_table = &RTC_op_table; break;
                case 1: pcb->file_array[i].op_table = &dir_op_table; break;
                case 2: pcb->file_array[i].op_table = &file_op_table; break;
            }

            pcb->file_array[i].inode = dentry.inode_num; 

            pcb->file_array[i].file_pos = 0;

            /* Perform the type specific open */
            pcb->file_array[i].op_table->open(filename);

            /* Return the fd index */
            return i;
        }
    }
    /* The array is full */
    KDEBUG("ERROR: PCB array was full!\n");
    return -1;
}

int32_t pcb_close(int32_t fd) {
    if (fd==0 || fd==1) {
        KDEBUG("WARNING: Tried to close stdin or stdout.\n");
        return -1;
    }
    if (-1 == pcb_check_valid_fd(fd)) return -1;

    pcb_t* pcb = get_curr_pcb();
    pcb->file_array[fd].op_table->close(fd);

    pcb->file_array[fd].flags = 0;
    pcb->file_array[fd].inode = 0;
    pcb->file_array[fd].file_pos = 0;
    pcb->file_array[fd].op_table = 0;

    return 0;
}

int32_t pcb_close_all_open(void) {
    int i;
    /* Close all open fds */
    /* (Except stdin or stdout) */
    for (i = 2; i < FILE_ARRAY_SIZE; i++) {
        /* If it's not open, just continue */
        if (-1 == pcb_check_valid_fd(i)) continue;
        if (-1 == pcb_close(i)) return -1;
    }
    
    /* Success. */
    return 0;
}

int32_t pcb_read(int32_t fd, void* buf, int32_t nbytes) {
    if (-1 == pcb_check_valid_fd(fd)) return -1;
    
    /* Attempting to read from stdout */
    if (fd == 1) return -1;

    return get_curr_pcb()->file_array[fd].op_table->read(fd, buf, nbytes);
}

int32_t pcb_write(int32_t fd, const void* buf, int32_t nbytes) {
    if (-1 == pcb_check_valid_fd(fd)) return -1;

    /* Attempting to write to stdin */
    if (fd == 0) return -1;

    return get_curr_pcb()->file_array[fd].op_table->write(fd, buf, nbytes);
}

int32_t pcb_ioctl(int32_t fd, uint32_t command, uint32_t args)
{
    return get_curr_pcb()->file_array[fd].op_table->ioctl(fd, command, args);
}

int32_t pcb_get_file_pos(int32_t fd) {
    if (-1 == pcb_check_valid_fd(fd)) return -1;

    return get_curr_pcb()->file_array[fd].file_pos;
}

void pcb_set_file_pos(int32_t fd, int32_t pos) {
    if (-1 == pcb_check_valid_fd(fd)) return;

    get_curr_pcb()->file_array[fd].file_pos = pos;
}

void pcb_set_inode(int32_t fd, int32_t inode_num) {
    if (-1 == pcb_check_valid_fd(fd)) return;

    get_curr_pcb()->file_array[fd].inode = inode_num;
}

int32_t pcb_get_inode(int32_t fd) {
    if (-1 == pcb_check_valid_fd(fd)) return -1;

    return get_curr_pcb()->file_array[fd].inode;
}

int32_t pcb_check_valid_fd(int32_t fd) {
    pcb_t* pcb = get_curr_pcb();
    if (!pcb) {
        KDEBUG("ERROR: Invalid PCB!\n");
        return -1;
    }
    /* Bounds checking */
    if (0 > fd || fd >= FILE_ARRAY_SIZE) {
        KDEBUG("ERROR: Invalid fd!\n");
        return -1;
    }
    /* Trying to handle a non-opened fd fails */
    if (!(pcb->file_array[fd].flags & FLAG_IN_USE)) {
        return -1;
    }

    return 0;
}

int32_t pcb_init(pcb_t* pcb) {
    if (!pcb) {
        KDEBUG("ERROR: Failed to init pcb!\n");
        return -1;
    }
    /* Clear the pcb */
    *pcb = empty_pcb;

    if (-1 == pcb_std(pcb)) return -1;

    /* Activate the pcb */
    pcb->active = 1;

    /* Success. */
    return 0;
}

int32_t pcb_destroy(pcb_t* pcb) {
    if (!pcb) {
        KDEBUG("ERROR: Failed to destroy pcb!\n");
        return -1;
    }
    /* Clear the pcb */
    *pcb = empty_pcb;

    return 0;
}

int32_t pcb_std(pcb_t* pcb) {
    if (!pcb) {
        KDEBUG("ERROR: Failed to set std!\n");
        return -1;
    }
    /* Set the first two elements as in use, they are always stdin and stdout */
    pcb->file_array[0].flags |= FLAG_IN_USE;
    pcb->file_array[1].flags |= FLAG_IN_USE;

    /* Both stdin and stdout are terminal drivers */
    pcb->file_array[0].file_pos = 0;
    pcb->file_array[0].inode = 0;
    pcb->file_array[0].op_table = &terminal_op_table;
    
    pcb->file_array[1].file_pos = 0;
    pcb->file_array[1].inode = 0;
    pcb->file_array[1].op_table = &terminal_op_table;
    /* WARNING: This is susceptible to closing stdin and stdout... */

    /* Success. */
    return 0;
}

pcb_t* alloc_pcb(void) {
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        /* +1 here because pcb0 points to 8MB - 8kB */
        pcb_t* pcb = (pcb_t*) (KERNEL_BOTTOM - KSTACK_SIZE * (i+1));
        if (pcb->active) continue;

        /* Free space found, initialize */
        if (-1 == pcb_init(pcb)) {
            return NULL;
        }
        pcb->id = i;
        return pcb;
    }

    /* Max number of processes reached... */
    KDEBUG("ERROR: Could not allocate PCB!\n");
    return NULL;
}

void initialize_all_pcbs(void) {
    int i;
    /* Ensure all pcbs in memory are empty */
    for (i = 0; i < MAX_TASKS; i++) {
        pcb_t* pcb = (pcb_t*) (KERNEL_BOTTOM - KSTACK_SIZE * (i+1));
        *pcb = empty_pcb;
    }
}
