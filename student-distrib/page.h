/**
 * page.h: defines structures and operations used to implement paging
 *         and other virtual memory constructs
*/

#ifndef _PAGE_H
#define _PAGE_H

#include "lib.h"
#include "types.h"
#include "alloc.h"

#define PAGING_ENTRY_NUM            1024
#define NUM_PROCESS                 6
#define PDE_0MB                     0
#define PDE_4MB                     1
#define PDE_32MB                    8              
#define PDE_128MB                   32
#define PDE_132MB                   33
#define PDE_136MB                   34

#define PAGE_4KB_SIZE_B             0x1000
#define PAGE_4MB_SIZE_B             0x400000

#define MULTIBOOT_INFO_START        0x0002C000
#define VIDEO_MEM_START             0x000B8000  
#define VIDEO_MEM_END               0x000B9000
#define VIDEO_MEM_START_USER        0x084B8000
#define VIDEO_MEM_END_USER          0x084B9000

#define KERNEL_PAGE_START           0x00400000
#define KERNEL_PAGE_END             0x00800000

#define DEFAULT_BLANK_PAGE                  0x00000002
#define DEFAULT_KERNEL_4KB_PAGE_ENTRY       0x00000003
#define DEFAULT_USER_4KB_PAGE_ENTRY         0x00000007
#define DEFAULT_KERNEL_4MB_PAGE_ENTRY       0x00000183
#define DEFAULT_USER_4MB_PAGE_ENTRY         0x00000187

#define PAGE_DIRECTORY_MASK                 0xFFC00000
#define PAGE_4MB_BASE_ADDR_MASK             0xFFC00000
#define PAGE_DIRECTORY_BIT_OFFSET           22
#define PAGE_TABLE_MASK                     0x003FF000
#define PAGE_TABLE_BIT_OFFSET               12
#define PAGE_4KB_BASE_ADDR_MASK             0xFFFFF000

/**
 * @brief There are 8 DMA channels, each is given a 128KB block 
 *        of physical memory. This is enough to support
 *        64KB double buffer DMA transfers.
 * 
 * @details The starting addresses are physical memory addresses.
 *          These address span from 1MB (start) to 2MB (end).
 * 
 * @details The DMA has 24-bit addressibilty; thus, it can only 
 *          access the 1st 16MB of physical memory.
*/
#define DMA_BLOCK_START_ADDR                0x00100000
#define DMA_BLOCK_START_ADDR_CHANNEL0       0x00100000
#define DMA_BLOCK_START_ADDR_CHANNEL1       0x00120000
#define DMA_BLOCK_START_ADDR_CHANNEL2       0x00140000
#define DMA_BLOCK_START_ADDR_CHANNEL3       0x00160000
#define DMA_BLOCK_START_ADDR_CHANNEL4       0x00180000
#define DMA_BLOCK_START_ADDR_CHANNEL5       0x001A0000
#define DMA_BLOCK_START_ADDR_CHANNEL6       0x001C0000
#define DMA_BLOCK_START_ADDR_CHANNEL7       0x001F0000
#define DMA_BLOCK_END_ADDR                  0x00200000

/* ASM Subroutines */
/**
 * @brief Loads the address of the given page directory
 *        into CR3.
*/
extern void load_page_directory(uint32_t* page_dir);

/**
 * @brief Sets appropriate flags in control registers to enable paging
*/
extern void enable_paging(void);

/**
 * @brief Grabs the linear address after a page fault has occurred
 * 
 * @return uint32_t linear_address : linear address that caused page fault
*/
extern uint32_t page_fault_linear_address(void);

/* ASM Data Structures */
typedef union pde_t {
    uint32_t raw_pde;
    struct {
        uint8_t kpresent          : 1;   /* 1 = mapped | 0 = Not mapped */
        uint8_t kread_write       : 1;   /* 1 = R/W | 0 = Read only */
        uint8_t kuser_supervisor  : 1;   /* 1 = User | 0 = Supervisor */
        uint8_t kwrite_through    : 1;   /* 1 = Write-through | 0 = Write-back */
        uint8_t kcache_disabled   : 1;   /* 1 = No caching | 0 = caching enabled */
        uint8_t kaccessed         : 1;   /* 1 = Page accessed | 0 = not accessed */
        uint8_t kreserved0        : 1;   /* Always 0 */
        uint8_t kpage_size        : 1;   /* 1 = 4MB | 0 = 4KB */
        uint8_t kglobal_page      : 1;   /* 1 = Global | 0 = Non-global */
        uint8_t kavailable        : 3;   /* Not Used */
        uint32_t kpt_base_address : 20;  /* Page table physical address */
    } __attribute__ ((packed));
    struct {
        uint8_t mpresent          : 1;   /* 1 = mapped | 0 = Not mapped */
        uint8_t mread_write       : 1;   /* 1 = R/W | 0 = Read only */
        uint8_t muser_supervisor  : 1;   /* 1 = User | 0 = Supervisor */
        uint8_t mwrite_through    : 1;   /* 1 = Write-through | 0 = Write-back */
        uint8_t mcache_disabled   : 1;   /* 1 = No caching | 0 = caching enabled */
        uint8_t maccessed         : 1;   /* 1 = Page accessed | 0 = not accessed */
        uint8_t mdirty            : 1;   /* 1 = Written by processor | 0 = clean */
        uint8_t mpage_size        : 1;   /* 1 = 4MB | 0 = 4KB */
        uint8_t mglobal_page      : 1;   /* 1 = Global | 0 = Non-global */
        uint8_t mavailable        : 3;   /* Not Used */
        uint8_t mpt_attribute     : 1;   /* Flag for page table attribute (PAT) */
        uint16_t mreserved        : 9;   /* Not Used */
        uint32_t mpg_base_address : 10;  /* Page physical address */
    } __attribute__ ((packed));
} pde_t;

typedef union pte_t {
    uint32_t raw_pte;
    struct {
        uint8_t kpresent          : 1;   /* 1 = mapped | 0 = Not mapped */
        uint8_t kread_write       : 1;   /* 1 = R/W | 0 = Read only */
        uint8_t kuser_supervisor  : 1;   /* 1 = User | 0 = Supervisor */
        uint8_t kwrite_through    : 1;   /* 1 = Write-through | 0 = Write-back */
        uint8_t kcache_disabled   : 1;   /* 1 = No caching | 0 = caching enabled */
        uint8_t kaccessed         : 1;   /* 1 = Page accessed | 0 = not accessed */
        uint8_t kdirty            : 1;   /* 1 = Written by processor | 0 = clean */
        uint8_t kpt_attribute     : 1;   /* Flag for page table attribute (PAT) */
        uint8_t kglobal_page      : 1;   /* 1 = Global | 0 = Non-global */
        uint8_t kavailable        : 3;   /* Not Used */
        uint32_t kpg_base_address : 20;  /* Page physical address */
    } __attribute__ ((packed));
} pte_t;

/* Paging data structure for each process */
typedef struct proc_page_t {
    /* Main page directory */
    pde_t proc_pdirectory[PAGING_ENTRY_NUM] __attribute__((aligned (4096)));

    /* Kernel Video Memory */
    pte_t proc_ptable[PAGING_ENTRY_NUM] __attribute__((aligned (4096)));

    /* Vidmap Video Memory */
    pte_t proc_ptable2[PAGING_ENTRY_NUM] __attribute__((aligned (4096)));

    /* Kmem Cache Page */
    pte_t proc_ptable3[PAGING_ENTRY_NUM] __attribute__((aligned (4096)));
} proc_page_t;

/* Function Prototypes */
/**
 * @brief Entrypoint function to initialize all paging structures and 
 *        system settings. 
*/
extern void init_paging(void);

/**
 * @brief Initializes the page array for all the given processes in the system.
*/
void init_proc_paging(void);

/**
 * @brief Getter function for pointers to page directories given a pid
 * 
 * @param pid Process ID, index to processes page directory
*/
const proc_page_t* get_proc_page(uint8_t pid);

/**
 * @brief Setup user program video memory mapped at 132MB
 * 
 * @param pid Process ID, index to processes page directory
*/
void proc_user_vidmap(uint8_t pid);

typedef enum map_page_flags_e {
    ALLOC_4KB = 0x1,
    ALLOC_4MB = 0x2,
    ALLOC_USER = 0x4,
    ALLOC_KERNEL = 0x8,
    ALLOC_SLAB = 0x10,
    ALLOC_PAGE = 0x20
} map_page_flags_e;

/**
 * @brief Map virtual memory page to physical address
 * 
 * @param va : virtual memory address
 * @param pa : physical memory address
 * @param pid : process id number
 * @param flags : page flags
*/
void map_page(uint8_t* va, uint8_t* pa, uint8_t pid, map_page_flags_e flags);

/**
 * @brief Check if the virtual memory page is mapped, if so
 *        mark the page as not present.
 * 
 * @param va : virtual memory address
 * @param pid : process id number
*/
void mark_page_not_present(uint8_t* va, uint8_t pid);

/**
 * @brief Allocate page for non-display memory for a given process
 * 
 * @details The formula (0x1000 * pid) + 0xB9000 gives the base address 
 *          corresponding to non-active video memory. This is choosen to 
 *          
*/
void activate_proc_vidmem(uint8_t pid);

/**
 * @brief Deallocate page for non-display memory for a given process
*/
void deactivate_proc_vidmem(uint8_t pid);

#endif /* _PAGE_H */
