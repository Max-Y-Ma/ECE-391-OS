#ifndef _ALLOC_H_
#define _ALLOC_H_

#include "types.h"
#include "spinlock.h"
#include "lib.h"
#include "page.h"

/**
 * @brief Extra credit for dynamic memory allocation.
 * 
 * @details 4MB are set aside for slab cache allocations.
 *          96MB are set aside for page allocations using the buddy
 *          system. The buddy system is not supported currently.
 * 
 * @details Dynamic allocation calls within the kernel should be 
 *          directed towards kmalloc() and kfree(), while user space 
 *          programs should interface through malloc() and free().
 * 
 * @details User space programs have a heap memory mapped to start at
 *          136MB in virtual memory. Dynamic allocations are restrained
 *          to 32MB - 128MB in physical memory.
*/

#define USER_SPACE_HEAP_START       0x08800000
#define USER_SPACE_HEAP_OFFSET      USER_SPACE_HEAP_START - KMEM_CACHE_START

/* Call Interface */
typedef enum kmem_flags_e {
    KMEM_ATOMIC = 1,
    KMEM_KERNEL = 2,
    KMEM_USER = 4
} kmem_flags_e;

/**
 * @brief Main dynamic memory allocation interface, directs
 *        to slab cache or page allocation interfaces.
 * 
 * @param size Number of bytes requested
 * @param flags Type of dynamic memory operation
 * 
 * @return Void pointer to allocated physical memory address
*/
void* kmalloc(uint32_t size, kmem_flags_e flags);

/**
 * @brief Main dynamic memory deallocation interface, directs
 *        to slab cache or page deallocation interfaces.
 * 
 * @param ptr Pointer to physical memory address to be freed
 * 
 * @return Void
*/
void kfree(void* kptr, kmem_flags_e flags); 

/* Slab Cache: Data Structures and Function Prototypes */

#define PAGE_SIZE_BYTES         4096
#define PAGE_SIZE_LOG2          12
#define MAX_SLAB_CACHES         1024
#define NUM_OBJECT_ENTRIES      128
#define MAX_SLAB_OBJECT_SIZE    512
#define OBJECT_FLAG_ALLOC       1
#define OBJECT_FLAG_FREE        0

/* Size in bytes */
#define SLAB_OBJECT_SIZE_0      1
#define SLAB_OBJECT_SIZE_1      4
#define SLAB_OBJECT_SIZE_2      8
#define SLAB_OBJECT_SIZE_3      16
#define SLAB_OBJECT_SIZE_4      32
#define SLAB_OBJECT_SIZE_5      64
#define SLAB_OBJECT_SIZE_6      256
#define SLAB_OBJECT_SIZE_7      512
#define NUM_SLAB_OBJECTS        8

#define KMEM_CACHE_START        0x2000000
#define KMEM_CACHE_END          0x2400000

#define KMEM_OBJECT_MASK        0x00000FFF
#define KMEM_SLAB_MASK          0x003FF000

#define BITMAP_ENTRY_SIZE       8
typedef uint8_t slab_bitmap_t;

typedef struct slab_cache_t {
    uint16_t object_size;
    uint8_t* cache;
    slab_bitmap_t* bitmap;
    spinlock_t lock;
} slab_cache_t;

/**
 * @brief Initialize all slab cache data structures
*/
void init_kcache(void);

/**
 * @brief Allocate memory from slab cache 
 * 
 * @param size : Object size in bytes
 * 
 * @return Void pointer to physical memory address
*/
void* kcache_alloc(uint32_t size);

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
void kcache_free(void* kptr);

/* Page Allocations: Data Structures and Function Prototypes */

#define KMEM_PAGE_START     0x2400000
#define KMEM_PAGE_END       0x8000000

typedef struct page_node_t {
    uint8_t* page;
    struct page_node_t* next;
} page_node_t;

typedef struct free_block_t {
    page_node_t* free_list;
    uint32_t* map;
} free_block_t;

typedef free_block_t* zone_t;

/**
 * @brief Allocate page of memory using buddy system
 * 
 * @param order : Number of pages = 2**order
 * 
 * @return void* to physical memory address 
*/
void* kpage_alloc(uint8_t order);

/**
 * @brief Free page of memory using buddy system
 * 
 * @param kptr : Pointer to physical page
 * 
 * @return None
*/
void kpage_free(void* kptr);

#endif /* _ALLOC_H_ */
