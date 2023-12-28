#include "switch.h"
#include "lib.h"
#include "syscalls.h"
#include "loader.h"
#include "proc/PCB.h"
#include "drivers/RTC.h"
#include "page.h"

void context_switch(void) {
    /* next tcb will rotate between 0, 1, and 2 */



    uint8_t next_tcb_idx = (tcb_get_curr_idx() + 1) % MAX_NUM_TERMINAL;

    /* Inform the tcb that we are at a new index */
    tcb_set_curr_idx(next_tcb_idx);

    pcb_t* curr_pcb = get_curr_pcb();

    /* Get the next pcb associated with the next terminal */
    pcb_t* next_pcb = tcb_get_pcb(next_tcb_idx);

    load_page_directory((uint32_t*)get_proc_page(next_pcb->id)->proc_pdirectory);

  

    /* Set tss appropriately */
    tss.esp0 = (uint32_t)next_pcb + KSTACK_SIZE;
    tss.ss0 = KERNEL_DS;

    /* Save the current ebp */
    asm volatile (
        "movl %%ebp, %[this_ebp]\n\t"
        : [this_ebp] "=g" (curr_pcb->switch_ebp)
        :
    );

    /* Restore the next ebp */
    asm volatile (
        "movl %[next_ebp], %%ebp\n\t"
        :
        : [next_ebp] "r" (next_pcb->switch_ebp)
    );
}


int32_t setup_shell(void) {
    /* Since we call this from kernel, just pretend the first shell is active. */
    /* This will allow terminal 0 to associate with shell 0, etc. */
    get_curr_pcb()->active = 1;

    /* Mimick execute: */
    /* Read the header. */
    uint8_t command_buf[FILENAME_LEN + 1] = "shell\0";
    if (read_header(command_buf) != 0) return -1;

    /* Allocate a pcb. */
    pcb_t* pcb = alloc_pcb();
    if (!pcb) return -1;

    /* Set the parent to 0, i.e. kernel. */
    pcb->parent_pcb = 0;
    
    /* Load pages and program. */
    load_page_directory((uint32_t*)get_proc_page(pcb->id)->proc_pdirectory);
    if (load_program(command_buf, (uint8_t*)PROGRAM_START_MEM) != 0) {
        return -1;
    }

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

    /* Setting up the fake iret context */
    asm volatile (
        "movl %%esp, %%eax       \n\t" /* Save esp */
        /* Move the esp to the new proc's esp, for the purpose of setting up
        the fake iret in the process' kernel stack */
        "movl %[esp0], %%esp     \n\t" 
        "pushl %%eax             \n\t"
        "movw $0x002B, %%ax      \n\t" /* 0x002B is USER_DS */
        "movw %%ax, %%ds         \n\t"
        "popl %%eax              \n\t"
        "pushl $0x002B           \n\t" /* 0x002B is USER_DS */
        "pushl $0x83FFFFC        \n\t" /* Push User Stack (Virtual Address 132 MB) */
        "pushl %[eflags]         \n\t"
        "pushl $0x0023           \n\t" /* 0x0023 is USER_CS */
        "pushl entry_point       \n\t"
        "pushl $fake_iret        \n\t" /* return addr goes to a label that just irets */
        "pushl %[esp0]           \n\t" /* old ebp points to the bottom of the proc's kernel stack */
        "movl %%esp, %[switch_ebp]\n\t" /* save the ebp that we will switch to on the first context switch */
        "movl %%eax, %%esp       \n\t" /* restore the esp, everything back to normal */
        : [switch_ebp] "=g" (pcb->switch_ebp)
        : [esp0] "r" ((uint32_t)pcb + KSTACK_SIZE), [eflags] "r" (eflags)
        : "eax"
    );

    /* Stop pretending like the first shell is active. */    
    get_curr_pcb()->active = 0;

    /* Success. */
    return 0;
}
