#include "page.h"
#include "drivers/terminal.h" 

/**
 * @brief The following data structure is used to hold all the page directories/tables
 *        for all the running processes in the system (NUM_PROCESS). Each process is given
 *        a page directory and a page table. The page directory should contain a 4MB page 
 *        at virtual address 128 MB, which is mapped to physical address 8MB + (pid * 4MB).
 *        The first process should be given pid 0. Additionally, each process should
 *        have a 4MB kernel page  at virtual and physical address 4MB. The video memory will
 *        be allocated a 4KB page in the page table. 
*/
static proc_page_t proc_pd[NUM_PROCESS];

/**
 * @brief Entrypoint function to initialize all paging structures and 
 *        system settings. 
*/
void init_paging(void)
{
    /* Assembly linkage to load page directory to CR3 */
    load_page_directory((uint32_t*)proc_pd);

    /* Initialize page directories/tables for all processes */
    init_proc_paging();

    /* Assembly linkage to enable page flags in CR0 and CR4 */
    enable_paging();
}

/**
 * @brief Initializes the page array for all the given processes in the system.
*/
void init_proc_paging(void)
{
    /* Empty out page directories for all processes */
    {
    int i;
    for (i = 0; i < NUM_PROCESS; i++) {
        int j;
        for(j = 0; j < PAGING_ENTRY_NUM; j++) {
            /* Entry attributes: R/W, superuser, and not present */
            proc_pd[i].proc_pdirectory[j].raw_pde = DEFAULT_BLANK_PAGE;
        }
    }
    }

    /* Initialize kernel memory 0MB - 4MB as 4KB pages for each process */
    {
    int i;
    for (i = 0; i < NUM_PROCESS; i++) {
        int j;
        for(j = 0; j < PAGING_ENTRY_NUM; j++) {
            /* Initializes Video memory page  */
            if ((j * PAGE_4KB_SIZE_B) == VIDEO_MEM_START) {
                /* Entry attributes: R/W, superuser, and present */
                proc_pd[i].proc_ptable[j].raw_pte = (j * PAGE_4KB_SIZE_B) | DEFAULT_KERNEL_4KB_PAGE_ENTRY; 
            } 
            /* Initializes DMA page blocks */
            else if (DMA_BLOCK_START_ADDR <= (j * PAGE_4KB_SIZE_B) && (j * PAGE_4KB_SIZE_B) < DMA_BLOCK_END_ADDR) {
                /* Entry attributes: R/W, superuser, and present */
                proc_pd[i].proc_ptable[j].raw_pte = (j * PAGE_4KB_SIZE_B) | DEFAULT_KERNEL_4KB_PAGE_ENTRY; 
            } else {
                /* Entry attributes: R/W, superuser, and not present */
                proc_pd[i].proc_ptable[j].raw_pte = DEFAULT_BLANK_PAGE;
            }
        }
    }
    }

    /* Initialize kernel memory 32MB - 36MB as 4KB pages for slab cache memory */
    {
    int i;
    for (i = 0; i < NUM_PROCESS; i++) {
        int j;
        for(j = 0; j < PAGING_ENTRY_NUM; j++) {
            /* Entry attributes: R/W, superuser, and present */
            proc_pd[i].proc_ptable3[j].raw_pte = DEFAULT_BLANK_PAGE; 
        }
    }
    }

    /* Setup page directory entries for each process */
    {
    int i;
    for (i = 0; i < NUM_PROCESS; i++) {
        /* Set kernel page directory entry 32 [128MB - 132MB] */
        /* Initialize virtual address 128MB to physical address 8MB + (pid * 4MB) */
        /* Entry attributes: 4MB, R/W, User, Present */
        proc_pd[i].proc_pdirectory[PDE_128MB].raw_pde = (KERNEL_PAGE_END + (i * KERNEL_PAGE_START)) | DEFAULT_USER_4MB_PAGE_ENTRY;

        /* Set kernel page directory entry 8 [32MB - 36MB] */
        /* Entry attributes: 4KB, R/W, Super User, Present */
        proc_pd[i].proc_pdirectory[PDE_32MB].raw_pde = (uint32_t)proc_pd[i].proc_ptable3 | DEFAULT_KERNEL_4KB_PAGE_ENTRY;
        
        /* Set kernel page directory entry 1 [4MB - 8MB] */
        /* Entry attributes: 4MB, R/W, Super User, Present */
        proc_pd[i].proc_pdirectory[PDE_4MB].raw_pde = KERNEL_PAGE_START | DEFAULT_KERNEL_4MB_PAGE_ENTRY;

        /* Set kernel page directory entry 0 [0MB - 4MB] */
        /* Entry attributes: 4KB, R/W, Super User, Present */
        proc_pd[i].proc_pdirectory[PDE_0MB].raw_pde = (uint32_t)proc_pd[i].proc_ptable | DEFAULT_KERNEL_4KB_PAGE_ENTRY;
    }
    }
}

/**
 * @brief Getter function for pointers to page directories given a pid
 * 
 * @param pid Process ID, index to processes page directory
*/
const proc_page_t* get_proc_page(uint8_t pid)
{
    if (pid >= NUM_PROCESS) {
        return NULL;
    }

    return &proc_pd[pid];
}

/**
 * @brief Setup user program video memory mapped at 132MB
 * 
 * @param pid Process ID, index to processes page directory
*/
void proc_user_vidmap(uint8_t pid)
{
    /* Set page directory entry corresponding to 132MB to point to page table */
    proc_pd[pid].proc_pdirectory[PDE_132MB].raw_pde = (uint32_t)proc_pd[pid].proc_ptable2 | DEFAULT_USER_4KB_PAGE_ENTRY;

    /* Set page table entry to physical address of video memory */
    proc_pd[pid].proc_ptable2[VIDEO_MEM_START / PAGE_4KB_SIZE_B].raw_pte = VIDEO_MEM_START | DEFAULT_USER_4KB_PAGE_ENTRY;
}

/**
 * @brief Map virtual memory page to physical address
 * 
 * @param va : virtual memory address
 * @param pa : physical memory address
 * @param pid : process id number
 * @param flags : page flags
*/
void map_page(uint8_t* va, uint8_t* pa, uint8_t pid, map_page_flags_e flags)
{
    /* Check virtual and physical addresses */
    if (va == NULL || pa == NULL) {
        /* Could throw signal */
        return;
    }

    /* Calculate page directory offset */
    uint32_t pd_idx = ((uint32_t)va & PAGE_DIRECTORY_MASK) >> PAGE_DIRECTORY_BIT_OFFSET;

    /* Calculate page table offset */
    uint32_t pt_idx = ((uint32_t)va & PAGE_TABLE_MASK) >> PAGE_TABLE_BIT_OFFSET;

    /* Grab page directory entry */
    pde_t* pde = &(proc_pd[pid].proc_pdirectory[pd_idx]);

    if (flags & ALLOC_4MB) {
        /* Set page directory entry */
        uint32_t mpage_base_addr = (uint32_t)pa & PAGE_4MB_BASE_ADDR_MASK;
        if (flags & ALLOC_USER)
            pde->raw_pde = mpage_base_addr | DEFAULT_USER_4MB_PAGE_ENTRY;
        else if (flags & ALLOC_KERNEL)
            pde->raw_pde = mpage_base_addr | DEFAULT_KERNEL_4MB_PAGE_ENTRY;
        else
            /* Could throw signal */
            return;
    } 
    else if (flags & ALLOC_4KB) {
        if (flags & ALLOC_SLAB) {
            /* Set page directory entry */
            pde->raw_pde = (uint32_t)proc_pd[pid].proc_ptable3;
            uint32_t page_base_addr = (uint32_t)pa & PAGE_4KB_BASE_ADDR_MASK;
            if (flags & ALLOC_KERNEL) {
                pde->raw_pde |= DEFAULT_KERNEL_4KB_PAGE_ENTRY;
                proc_pd[pid].proc_ptable3[pt_idx].raw_pte = page_base_addr | DEFAULT_KERNEL_4KB_PAGE_ENTRY;
            }
            else if (flags & ALLOC_USER) {
                pde->raw_pde |= DEFAULT_USER_4KB_PAGE_ENTRY;
                proc_pd[pid].proc_ptable3[pt_idx].raw_pte = page_base_addr | DEFAULT_USER_4KB_PAGE_ENTRY;
            }
            else {
                /* Could throw signal */
                return;
            }
        }
        else if (flags & ALLOC_PAGE) {
            /* Could throw signal */
            return;
        }
        else {
            /* Could throw signal */
            return;
        }
    } 
    else {
        /* Could throw signal */
        return;
    }
}

/**
 * @brief Check if the virtual memory page is mapped, if so
 *        mark the page as not present.
 * 
 * @param va : virtual memory address
 * @param pid : process id number
*/
void mark_page_not_present(uint8_t* va, uint8_t pid)
{
    /* Check virtual address */
    if (va == NULL) {
        return;
    }

    /* Calculate page directory offset */
    uint32_t pd_idx = ((uint32_t)va & PAGE_DIRECTORY_MASK) >> PAGE_DIRECTORY_BIT_OFFSET;

    /* Calculate page table offset */
    uint32_t pt_idx = ((uint32_t)va & PAGE_TABLE_MASK) >> PAGE_TABLE_BIT_OFFSET;

    /* Grab page directory entry */
    pde_t* pde = &(proc_pd[pid].proc_pdirectory[pd_idx]);

    /* Return if page directory entry is not present */
    if (pde->kpresent == 0) { return; }

    /* Grab page table entry */
    pte_t* pte = &(((pte_t*)(pde->kpt_base_address << PAGE_TABLE_BIT_OFFSET))[pt_idx]);

    /* Check 4MB page */
    if (pde->mpage_size) {
        /* Set 4MB page not present */
        pde->mpresent = 0;
    } else {
        /* Set 4KB page not present */
        pte->kpresent = 0;
        pde->mpresent = 0;
    }
}

/**
 * @brief Allocate page for non-display memory for a given process
*/
void activate_proc_vidmem(uint8_t pid)
{
    /* Move vidmap page table entry to 0xB8000 */
    proc_pd[pid].proc_ptable2[VIDEO_MEM_START / PAGE_4KB_SIZE_B].raw_pte = VIDEO_MEM_START | DEFAULT_USER_4KB_PAGE_ENTRY;
}

/**
 * @brief Deallocate page for non-display memory for a given process
*/
void deactivate_proc_vidmem(uint8_t pid)
{
    /* Move vidmap page table entry to terminal buffer */
    uint8_t tcb_index = ((pcb_t*)(KERNEL_BOTTOM - KSTACK_SIZE * (pid + 1)))->tcb_idx;
    uint16_t* buffer_base_address = get_tcb_screen_buffer(tcb_index);
    proc_pd[pid].proc_ptable2[VIDEO_MEM_START / PAGE_4KB_SIZE_B].raw_pte = (uint32_t)buffer_base_address | DEFAULT_USER_4KB_PAGE_ENTRY;
}
