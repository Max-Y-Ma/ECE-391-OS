#include "lib.h"
#include "page.h"
#include "loader.h"
#include "syscalls.h"
#include "proc/PCB.h"
#include "drivers/terminal.h"
#include "alloc.h"

/* halt system call: Index 1 */
int32_t system_halt (uint32_t status)
{
    /* Disable interrupts */
    cli();

    if (-1 == pcb_close_all_open()) {
        sti(); return -1;
    }

    /* Close video memory from vidmap system call */
    mark_page_not_present((uint8_t*) VIDEO_MEM_START_USER, get_curr_pcb()->id);

    /* Restore parent data */
    pcb_t* parent_pcb = get_curr_pcb()->parent_pcb;
    
    /* If the parent pcb is NULL, we have halted all processes -> restart shell */
    if (!parent_pcb) {
        if (-1 == pcb_destroy(get_curr_pcb())) {
            sti(); return -1;
        }
        system_execute((uint8_t*)"shell");

        /* This should never be reached */
        sti(); return -1;
    }

    /* Inform the tcb that a child has been killed, go back to the parent */
    tcb_set_pcb(parent_pcb->tcb_idx, parent_pcb);

    /* Restore parent paging */
    load_page_directory((uint32_t*)get_proc_page(parent_pcb->id)->proc_pdirectory);
    
    /* Destroy the pcb */
    if (-1 == pcb_destroy(get_curr_pcb())) {
        sti(); return -1;
    }

    /* Restore the TSS */
    tss.esp0 = (uint32_t)parent_pcb + KSTACK_SIZE;

    /* Enable interrupts */
    // sti();

    /* Restore the stack frame for returning to the parent context */
    asm volatile(
        "system_halt_stack_restore:     \n\t"
            "xorl %%eax, %%eax          \n\t"
            "movl %[status], %%eax      \n\t"
            "movl %[old_ebp], %%ebp     \n\t"
            "leave                      \n\t"
            "ret                        \n\t"
        :
        : [old_ebp] "r" (parent_pcb->old_ebp), [status] "r" ((uint32_t) status)
        : "eax"
    );

    /* Should never get here, since we leave and ret in inline asm */
    return -1;
}

/* execute system call: Index 2 */
int32_t system_execute (const uint8_t* command)
{
    /* Disable interrupts */
    cli();

    /* Parse Args */
    if (command == NULL) {
        sti(); return -1;
    }

    unsigned int cmd_start = 0;
    /* Strip leading spaces */
    while( *(command+cmd_start) != NULL){
        if( *(command+cmd_start) != ' ' ){
            break;
        }
        cmd_start++;
    }

    /* Copy command word into buffer */
    int arg_flag = 1;
    uint8_t command_buf[FILENAME_LEN + 1];                  //Extra size for NULL termination
    unsigned int cmd_len = 0;
    while( cmd_len != FILENAME_LEN + 1){
        if( *(command + cmd_start + cmd_len) == ' '){
            break;
        }
        if( *(command + cmd_start + cmd_len) == NULL ){     //No arguments for this command
            arg_flag = 0;
            break;
        }
        command_buf[cmd_len] = *(command + cmd_start + cmd_len);
        cmd_len++;
    }

    if(cmd_len >= FILENAME_LEN + 1){      //Command too long
        sti(); return -1;
    }
    /* Null terminate our buffer */
    command_buf[cmd_len] = NULL;

    /* Grab executable data */
    if (read_header(command_buf) != 0) {
        sti(); return -1;
    }

    /* Create and initialize PCB */
    pcb_t* pcb = alloc_pcb();
    if (!pcb) {
        sti(); return -1;
    }
    /* Only set the parent pcb if there is a parent */
    if (pcb != get_curr_pcb()) {
        pcb->parent_pcb = get_curr_pcb();
        
        /* Children inherit their parent's terminal */
        pcb->tcb_idx = pcb->parent_pcb->tcb_idx;
    } else {
        /* This block will only occur if we tried to exit from a shell. */

        pcb->parent_pcb = 0;
        /* The id will correspond also with the terminal id */
        pcb->tcb_idx = pcb->id;
    }

    /* Inform the asssociated tcb that there is a new process */
    tcb_set_pcb(pcb->tcb_idx, pcb);

    /* Save command args */
    /* Assuming command has no more leading spaces and does have args*/
    if(arg_flag){
        const uint8_t* arg_start = command + cmd_start + cmd_len;
        unsigned int i = 0;
        for(i=0; i < MAX_ARGS; i++){
            if(*(arg_start + i) != ' '){
                break;
            }
        }
        strcpy((int8_t*)pcb->args, (int8_t*)(arg_start + i));
    }
    /* Command has no args, pass in empty string */
    else{
        uint8_t* empty = (uint8_t*)"";
        strcpy((int8_t*)pcb->args, (int8_t*)empty);
    }

    /* Setup paging for new process */
    load_page_directory((uint32_t*)get_proc_page(pcb->id)->proc_pdirectory);

    /* Load executable into memory */
    if (load_program(command_buf, (uint8_t*)PROGRAM_START_MEM) != 0) {
        sti(); return -1;
    }

    /* Load TSS with SS0 and ESP0 to allow 
    for privilege switches from user to kernel */
    /* Save the TSS, and save special registers */
    tss.esp0 = (uint32_t)pcb + KSTACK_SIZE;
    tss.ss0 = KERNEL_DS;

    /* Store the current context for safe halting */
    uint32_t old_ebp;
    asm volatile (
        "movl %%ebp, %[old_ebp]     \n\t"
        : [old_ebp] "=g" (old_ebp)
        : 
    );
    if (pcb->parent_pcb != 0) {
        pcb->parent_pcb->old_ebp = old_ebp;
    }

    /* Enable interrupts */
    // sti();

    uint32_t eflags;
    asm volatile (
        "pushl %%eax             \n\t"
        "pushf                   \n\t"
        "popl  %%eax             \n\t"
        "orl   $0x200, %%eax     \n\t" /* saving EFLAGS with IF FLAG forced set */
        "movl  %%eax, %[eflags]  \n\t"
        "popl  %%eax             \n\t"
        : [eflags] "=g" (eflags)
        :
    );

    /* Set up the iret into the new context */
    asm volatile (
        "execute_context_switch:     \n\t"
            "pushl %%eax             \n\t"
            "movw $0x002B, %%ax      \n\t" /* 0x002B is USER_DS */
            "movw %%ax, %%ds         \n\t"
            "popl %%eax              \n\t"
            "pushl $0x002B           \n\t" /* 0x002B is USER_DS */
            "pushl $0x83FFFFC        \n\t" /* Push User Stack (Virtual Address 132 MB) */
            "pushl %[eflags]         \n\t" /* pushing eflags with IF FLAG forced set */
            "pushl $0x0023           \n\t" /* 0x0023 is USER_CS */
            "pushl entry_point       \n\t"
            "iret                    \n\t"
        :
        : [eflags] "r" (eflags)
        : "eax"
    );

    /* Should never reach here, since we iret into a new context */
    return -1;
}

/* read system call: Index 3 */
int32_t system_read (int32_t fd, void* buf, int32_t nbytes)
{
    return pcb_read(fd, buf, nbytes);
}

/* write system call: Index 4 */
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes)
{
    return pcb_write(fd, buf, nbytes);
}

/* open system call: Index 5 */
int32_t system_open (const uint8_t* filename)
{
    return pcb_open(filename);
}

/* close system call: Index 6 */
int32_t system_close (int32_t fd)
{
    return pcb_close(fd);
}

/* getargs system call: Index 7 */
int32_t system_getargs (uint8_t* buf, int32_t nbytes)
{

    /* NULL Checking */
    if(buf == NULL || nbytes < 0)
        return -1;

    /* Grab args form PCB */
    pcb_t* curr_pcb = get_curr_pcb();
    memset(buf, 0,MAX_ARGS);
    int32_t len = 0;
    while( *(curr_pcb->args + len) != NULL){
        len++;
    }

    /* Compare lengths to buffer, need space for NULL termination */
    if( len >= nbytes )
        return -1;

    /* Copy buffer */
    strcpy((int8_t*)buf, (int8_t*)curr_pcb->args);

    return 0;
}

/* vidmap system call: Index 8 */
int32_t system_vidmap (uint8_t** screen_start)
{
    /* Null Check screen_start */
    if (screen_start == NULL) {
        return -1;
    }

    /* Check screen_start 128MB and 132MB */
    if (!(PROGRAM_START_MEM <= (uint32_t)screen_start && (uint32_t)screen_start < PROGRAM_END_MEM)) {
        return -1;
    }

    /* Map Video Memory Page to 132MB + VIDEO_MEM_START*/
    proc_user_vidmap(get_curr_pcb()->id);
    *screen_start = (uint8_t*)VIDEO_MEM_START_USER;

    return VIDEO_MEM_START_USER;
}

/* set_handler system call: Index 9 */
int32_t system_set_handler (int32_t signum, void* handler_address)
{
    return -1;
}

/* sigreturn system call: Index 10 */
int32_t system_sigreturn (void)
{
    return -1;
}

/* malloc system call: Index 11 */
int32_t system_malloc(uint32_t nbytes)
{
    return (int32_t)kmalloc(nbytes, KMEM_ATOMIC | KMEM_USER);
}

/* free system call: Index 12 */
int32_t system_free(void* ptr)
{
    kfree(ptr, KMEM_ATOMIC | KMEM_USER);
    return 0;
}

/* ioctl system call: Index 13 */
int32_t system_ioctl(int32_t fd, uint32_t command, uint32_t args)
{
    /* */
    if (fd < 0 ) {
        return file_ioctl(0, command, args);
    }
    return pcb_ioctl(fd, command, args);
}
