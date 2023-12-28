#include "alloc.h"

/* Slab Cache Data Structures */
static slab_cache_t slab_cache_table[MAX_SLAB_CACHES];
static slab_bitmap_t slab_bitmap_table0[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_0)];
static slab_bitmap_t slab_bitmap_table1[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_1)];
static slab_bitmap_t slab_bitmap_table2[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_2)];
static slab_bitmap_t slab_bitmap_table3[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_3)];
static slab_bitmap_t slab_bitmap_table4[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_4)];
static slab_bitmap_t slab_bitmap_table5[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_5)];
static slab_bitmap_t slab_bitmap_table6[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_6)];
static slab_bitmap_t slab_bitmap_table7[NUM_OBJECT_ENTRIES][PAGE_SIZE_BYTES / (BITMAP_ENTRY_SIZE * SLAB_OBJECT_SIZE_7)];

/* Function Prototypes */
uint32_t log2i(uint32_t num);
uint32_t slab_cache_index(uint32_t size);

/**
 * @brief Main dynamic memory allocation interface, directs
 *        to slab cache or page allocation interfaces.
 * 
 * @param size Number of bytes requested
 * @param flags Type of dynamic memory operation
 * 
 * @return Void pointer to allocated physical memory address
*/
void* kmalloc(uint32_t size, kmem_flags_e flags)
{
    /* Atomic allocation check */
    uint32_t sysflags;
    if (flags & KMEM_ATOMIC) {
        cli_and_save(sysflags);
    }

    /* Slab cache allocations */
    void* kptr = NULL;
    uint16_t alloc_type = 0;
    if (size <= MAX_SLAB_OBJECT_SIZE) {
        alloc_type = ALLOC_SLAB;
        kptr = kcache_alloc(size);
    }
    /* Page allocations */
    else {
        /* Determine page order */
        uint32_t log2_order = log2i(size - 1);
        uint8_t order = 0;
        
        /* Single page allocations */
        if (0 <= log2_order && log2_order < 12) {
            order = 0;
        } 
        /* Multipage allocations */
        else if (log2_order < 24) {
            order = log2_order - 11;
        } 
        /* Invalid size allocations*/
        else {
            /* Return Null, but could send signal */
            return NULL;
        }

        alloc_type = ALLOC_PAGE;
        kptr = kpage_alloc(order);
    }

    /* Page directory management */
    if (flags & KMEM_KERNEL) {
        uint8_t pid;
        for (pid = 0; pid < NUM_PROCESS; pid++) {
            map_page((uint8_t*)kptr, (uint8_t*)kptr, pid, ALLOC_4KB | ALLOC_KERNEL | alloc_type);
        }
    }
    else if (flags & KMEM_USER) {
        map_page((uint8_t*)kptr + USER_SPACE_HEAP_OFFSET, (uint8_t*)kptr, (get_curr_pcb())->id, ALLOC_4KB | ALLOC_USER | alloc_type);
        kptr += USER_SPACE_HEAP_OFFSET;
    }
    else {
        /* Return Null, but could send signal */
        return NULL;
    }

    /* Flush TLB */
    // load_page_directory((uint32_t*)get_proc_page(get_curr_pcb()->id)->proc_pdirectory);

    /* Atomic allocation check */
    if (flags & KMEM_ATOMIC) {
        restore_flags(sysflags);
    }

    return kptr;
}

/**
 * @brief Main dynamic memory deallocation interface, directs
 *        to slab cache or page deallocation interfaces.
 * 
 * @param ptr Pointer to physical memory address to be freed
 * 
 * @return Void
*/
void kfree(void* kptr, kmem_flags_e flags)
{
    /* Atomic allocation check */
    uint32_t sysflags;
    if (flags & KMEM_ATOMIC) {
        cli_and_save(sysflags);
    }

    /* User space address translations */
    if (flags & KMEM_USER) {
        kptr -= USER_SPACE_HEAP_OFFSET;
    }

    /* Kernel memory address translations */
    /* Kernel memory pointer are 1-to-1 with physical addresses */
    if ((flags & KMEM_KERNEL) || (flags & KMEM_USER)) {
        if (KMEM_CACHE_START <= (uint32_t)kptr && (uint32_t)kptr < KMEM_CACHE_END) {
            kcache_free(kptr);
        }
        else if (KMEM_PAGE_START <= (uint32_t)kptr && (uint32_t)kptr < KMEM_PAGE_END) {
            kpage_free(kptr);
        }
        /* Invalid Pointer*/
        else {
            /* Return, but could send signal */
            return;
        }
    }
    /* Invalid flags */
    else {
        /* Return, but could send signal */
        return;
    }

    /* Page directory management */
    if (flags & KMEM_KERNEL) {
        uint8_t pid;
        for (pid = 0; pid < NUM_PROCESS; pid++) {
            mark_page_not_present((uint8_t*)kptr, pid);
        }
    }
    else if (flags & KMEM_USER) {
        mark_page_not_present((uint8_t*)kptr + USER_SPACE_HEAP_OFFSET, (get_curr_pcb())->id);
    }
    else {
        /* Return, but could send signal */
        return;
    }

    /* Flush TLB */
    // load_page_directory((uint32_t*)get_proc_page(get_curr_pcb()->id)->proc_pdirectory);

    /* Atomic allocation check */
    if (flags & KMEM_ATOMIC) {
        restore_flags(sysflags);
    }

    return;
}

/**
 * @brief Initialize all slab cache data structures
*/
void init_kcache(void)
{
    uint32_t i, j, index;

    /* Initialize slab cache table */
    i = 0;
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_0;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table0[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_1;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table1[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_2;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table2[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_3;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table3[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_4;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table4[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_5;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table5[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_6;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table6[j];
    } i++; /* Increment i as index to object number */
    for (j = 0; j < NUM_OBJECT_ENTRIES; j++) {
        index = i * NUM_OBJECT_ENTRIES + j;
        slab_cache_table[index].object_size = SLAB_OBJECT_SIZE_7;
        slab_cache_table[index].cache = (uint8_t*)(index * PAGE_SIZE_BYTES + KMEM_CACHE_START);
        slab_cache_table[index].bitmap = slab_bitmap_table7[j];
    } i++; /* Increment i as index to object number */
}

/**
 * @brief Allocate memory from slab cache 
 * 
 * @param size : Object size in bytes
 * 
 * @return Void pointer to physical memory address
*/
void* kcache_alloc(uint32_t size)
{
    /* Grab table starting index*/
    uint32_t slab_table_start_index = slab_cache_index(size);

    /* Default to NULL for unavailable allocations */
    uint8_t* kptr = NULL;
    
    /* Traverse object cache table section and return physical address */
    uint32_t i, bit;
    uint32_t num_bits_in_table = PAGE_SIZE_BYTES / (slab_cache_table[slab_table_start_index].object_size);
    for (i = 0; i < NUM_OBJECT_ENTRIES; i++) {
        int32_t obj_idx = -1;
        uint8_t bitmap_table_idx, bit_position;
        slab_bitmap_t* bitmap_byte;

        /* Start Critical Section: Writes and reads from bitmap */
        spin_lock(&slab_cache_table[i + slab_table_start_index].lock);

        /* Traverse bitmap, searching for valid position */
        for (bit = 0; bit < num_bits_in_table; bit++) {
            /* Calculate indices */
            bitmap_table_idx = bit / BITMAP_ENTRY_SIZE;
            bit_position = bit % BITMAP_ENTRY_SIZE;
            bitmap_byte = &slab_cache_table[i + slab_table_start_index].bitmap[i] + bitmap_table_idx;

            /* Check availability */
            if ((*bitmap_byte & (0x01 << bit_position)) == OBJECT_FLAG_FREE) {
                /* Set allocated */
                *bitmap_byte |= (0x01 << bit_position);

                /* Set index */
                obj_idx = bit;

                break;
            }
        }

        /* End Critical Section: Writes and reads from bitmap */
        spin_unlock(&slab_cache_table[i + slab_table_start_index].lock);

        /* Set pointer to physical address upon valid position */
        if (obj_idx != -1) {
            kptr = slab_cache_table[i + slab_table_start_index].cache + (slab_cache_table[i + slab_table_start_index].object_size * obj_idx);
            break;
        }
    }

    return kptr;
}

/**
 * @brief Free memory from slab cache
 * 
 * @param kptr : Pointer to physical memory address
 *               to be deallocated
 * 
 * @details Bits [22:12] : Index into the slab cache table,
 *                         considering the object size.
 *          Bits [11:0] : Index into the object/bitmap table
 * 
 * @return None
*/
void kcache_free(void* kptr)
{
    /* Decode physical address pointer to cache indices */
    uint32_t slab_cache_index = (((uint32_t)kptr & KMEM_SLAB_MASK) >> PAGE_SIZE_LOG2);
    uint32_t object_index = ((uint32_t)kptr & KMEM_OBJECT_MASK) / slab_cache_table[slab_cache_index].object_size;

    /* Start Critical Section: Writes and reads from bitmap */
    spin_lock(&slab_cache_table[slab_cache_index].lock);

    /* Calculate indices */
    uint32_t bitmap_table_index = object_index / BITMAP_ENTRY_SIZE;
    uint32_t bit_position = object_index % BITMAP_ENTRY_SIZE;

    /* Set deallocated */
    slab_cache_table[slab_cache_index].bitmap[bitmap_table_index] &= ~(0x01 << bit_position);

    /* End Critical Section: Writes and reads from bitmap */
    spin_unlock(&slab_cache_table[slab_cache_index].lock);
}

/**
 * @brief Allocate page of memory using buddy system
 * 
 * @param order : Number of pages = 2**order
 * 
 * @return void* to physical memory address 
*/
void* kpage_alloc(uint8_t order)
{
    return NULL;
}

/**
 * @brief Free page of memory using buddy system
 * 
 * @param kptr : Pointer to physical page
 * 
 * @return None
*/
void kpage_free(void* kptr)
{
    return;
}

/**
 * @brief Determine the integer log2()
*/
uint32_t log2i(uint32_t num) {
    uint32_t ret = 0;

    /* Count the highest position of 1 bit */
    uint32_t i;
    for (i = 0; i < 32; i++) {
        if (num & 0x01)
            ret = i;
        
        num = num >> 1;
    }
    
    return ret;
}

uint32_t slab_cache_index(uint32_t size)
{
    if (size <= SLAB_OBJECT_SIZE_0) {
        return 0 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_1) {
        return 1 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_2) {
        return 2 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_3) {
        return 3 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_4) {
        return 4 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_5) {
        return 5 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_6) {
        return 6 * NUM_OBJECT_ENTRIES;
    }
    else if (size <= SLAB_OBJECT_SIZE_7) {
        return 7 * NUM_OBJECT_ENTRIES;
    }
    else {
        return -1;
    }
}
