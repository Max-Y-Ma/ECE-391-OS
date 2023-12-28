/* idt_handler.h - Defines for idt handlers
 */

#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#define STUB_NAME(num) interrupt_ ## num ## _linkage

#ifndef LINKAGE_STUB
#define LINKAGE_STUB(hex_num)       \
.globl STUB_NAME(hex_num);          \
STUB_NAME(hex_num):                 \
    pushl %eax;                     \
    movw $KERNEL_DS, %ax;           \
    movw %ax, %ds;                  \
    popl %eax;                      \
    pushl $ ## hex_num;             \
    jmp common_interrupt_handler
#endif

#ifndef ASM

#include "drivers/keyboard.h"
#include "drivers/RTC.h"
#include "drivers/pit.h"
#include "drivers/audio.h"
#include "i8259.h"
#include "page.h"

#define C_LINKAGE 0

/* Macros for IDT Initialization */
#define SET_STUB_DESCRIPTOR_IIDT(stub_num)                      \
do {                                                            \
    IDT_INTERRUPT_GATE(idt[stub_num], STUB_NAME(stub_num));     \
} while(0)

#define SET_STUB_DESCRIPTOR_TIDT(stub_num)                      \
do {                                                            \
    IDT_TRAP_GATE(idt[stub_num], STUB_NAME(stub_num));          \
} while(0)

#define SET_STUB_DESCRIPTOR_SIDT(stub_num)                      \
do {                                                            \
    IDT_SYSCALL_GATE(idt[stub_num], STUB_NAME(stub_num));      \
} while(0)

#define ZERO_DIVSION_HANDLER 0

#define PAGE_FAULT_HANDLER 14
extern uint32_t page_fault_err_code;

#define ASSERTION_HANDLER 15

#define SYS_CALL_HANDLER 0x80

/* External linkage stubs */
extern void interrupt_0x00_linkage(void);
extern void interrupt_0x01_linkage(void);
extern void interrupt_0x02_linkage(void);
extern void interrupt_0x03_linkage(void);
extern void interrupt_0x04_linkage(void);
extern void interrupt_0x05_linkage(void);
extern void interrupt_0x06_linkage(void);
extern void interrupt_0x07_linkage(void);
extern void interrupt_0x08_linkage(void);
extern void interrupt_0x09_linkage(void);
extern void interrupt_0x0A_linkage(void);
extern void interrupt_0x0B_linkage(void);
extern void interrupt_0x0C_linkage(void);
extern void interrupt_0x0D_linkage(void);
extern void interrupt_0x0E_linkage(void);
extern void interrupt_0x0F_linkage(void);
extern void interrupt_0x10_linkage(void);
extern void interrupt_0x11_linkage(void);
extern void interrupt_0x12_linkage(void);
extern void interrupt_0x13_linkage(void);

extern void interrupt_0x20_linkage(void);
extern void interrupt_0x21_linkage(void);
extern void interrupt_0x22_linkage(void);
extern void interrupt_0x23_linkage(void);
extern void interrupt_0x24_linkage(void);
extern void interrupt_0x25_linkage(void);
extern void interrupt_0x26_linkage(void);
extern void interrupt_0x27_linkage(void);
extern void interrupt_0x28_linkage(void);
extern void interrupt_0x29_linkage(void);
extern void interrupt_0x2A_linkage(void);
extern void interrupt_0x2B_linkage(void);
extern void interrupt_0x2C_linkage(void);
extern void interrupt_0x2D_linkage(void);
extern void interrupt_0x2E_linkage(void);
extern void interrupt_0x2F_linkage(void);

extern void interrupt_0x80_linkage(void);

#define KEYBOARD_HANDLER KEYBOARD_IRQ + ICW2_MASTER
/* -8 here because we are on the first PIC's perspective */
#define RTC_HANDLER RTC_IRQ + ICW2_SLAVE - 8
#define PIT_HANDLER PIT_IRQ + ICW2_MASTER

/**
 * @brief ******************** ATTENTION ********************
 *        Naming scheme for all exceptions and interrupt handlers:
 *        
 *        <what the handler does>_handler_<index>
 *        
 *        Example: void assertion_handler_15(void)
 * 
 *        ******************** ATTENTION ********************
*/

/**
 * @brief Macro to initialize an interrupt/exception handler
 *        Layout for interrupt gate in Intel IA-32 manual: Page 156
 * 
 * @param idt_desc : descriptor entry in IDT
 * @param idt_handler : function pointer to interrupt/exception handler
*/
#define IDT_TRAP_GATE(idt_desc, idt_handler)                                        \
do {                                                                                \
    SET_IDT_ENTRY(idt_desc, idt_handler);   /* Sets handler func pointer */         \
    idt_desc.seg_selector = KERNEL_CS;      /* Set code segment */                  \
    idt_desc.reserved4 = 0x00;              /* 0x00 for reserved byte */            \
    idt_desc.reserved3 = 0x01;              /* Set trap gate: 0x7 */                \
    idt_desc.reserved2 = 0x01;                                                      \
    idt_desc.reserved1 = 0x01;                                                      \
    idt_desc.size = 0x01;                   /* 1 = 32-bit */                        \
    idt_desc.reserved0 = 0x00;              /* 0x0 for reserved bit */              \
    idt_desc.dpl = 0x00;                    /* 0x0 = kernel privledge */            \
    idt_desc.present = 0x01;                /* 0x1 = handler defined */             \
} while(0)

/**
 * @brief Macro to initialize an interrupt/exception handler
 *        Layout for interrupt gate in Intel IA-32 manual: Page 156
 * 
 * @param idt_desc : descriptor entry in IDT
 * @param idt_handler : function pointer to interrupt/exception handler
*/
#define IDT_INTERRUPT_GATE(idt_desc, idt_handler)                                   \
do {                                                                                \
    SET_IDT_ENTRY(idt_desc, idt_handler);   /* Sets handler func pointer */         \
    idt_desc.seg_selector = KERNEL_CS;      /* Set code segment */                  \
    idt_desc.reserved4 = 0x00;              /* 0x00 for reserved byte */            \
    idt_desc.reserved3 = 0x00;              /* Set interrupt gate: 0x6 */           \
    idt_desc.reserved2 = 0x01;                                                      \
    idt_desc.reserved1 = 0x01;                                                      \
    idt_desc.size = 0x01;                   /* 1 = 32-bit */                        \
    idt_desc.reserved0 = 0x00;              /* 0x0 for reserved bit */              \
    idt_desc.dpl = 0x00;                    /* 0x0 = KERNEL privilege */            \
    idt_desc.present = 0x01;                /* 0x1 = handler defined */             \
} while(0)

#define IDT_SYSCALL_GATE(idt_desc, idt_handler)                                     \
do {                                                                                \
    SET_IDT_ENTRY(idt_desc, idt_handler);   /* Sets handler func pointer */         \
    idt_desc.seg_selector = KERNEL_CS;      /* Set code segment */                  \
    idt_desc.reserved4 = 0x00;              /* 0x00 for reserved byte */            \
    idt_desc.reserved3 = 0x01;              /* Set trap gate: 0x7 */                \
    idt_desc.reserved2 = 0x01;                                                      \
    idt_desc.reserved1 = 0x01;                                                      \
    idt_desc.size = 0x01;                   /* 1 = 32-bit */                        \
    idt_desc.reserved0 = 0x00;              /* 0x0 for reserved bit */              \
    idt_desc.dpl = 0x03;                    /* 0x3 = USER privilege */              \
    idt_desc.present = 0x01;                /* 0x1 = handler defined */             \
} while(0)

#if C_LINKAGE

/* Struct defining the stack upon call of do_handler */
typedef struct pt_regs_t {
    uint32_t ebx;           /* common_interrupt_handler: */
    uint32_t ecx;           /* common_interrupt_handler: */
    uint32_t edx;           /* common_interrupt_handler: */
    uint32_t esi;           /* common_interrupt_handler: */
    uint32_t edi;           /* common_interrupt_handler: */
    uint32_t ebp;           /* common_interrupt_handler: */
    uint32_t eax;           /* common_interrupt_handler: */
    uint32_t xds;           /* common_interrupt_handler: */
    uint32_t xes;           /* common_interrupt_handler: */
    uint32_t vec_num;       /* Linkage Stub: Vector number in IDT */
    uint32_t addr;          /* Processor: Return address */
    uint32_t xcs;           /* Processor: */
    uint32_t eflags;        /* Processor: */
    uint32_t esp;           /* Processor: */
    uint32_t xss;           /* Processor: */
} pt_regs_t;

/**
 * @brief Assembly linkage step, which calls the appropriate 
 *        irq or exception handler. Should support interrupt chaining
 *        and a level of synchonization in the future. 
 * 
 * @param pt_regs Layout of the stack during assembly linkage step
*/
void do_handler(pt_regs_t pt_regs);

#endif /* C_LINKAGE */

/**
 * @brief Initializes IDTR with the base address of the IDT. 
 *        Initializes the IDT descriptor for each interrupt 
 *        or exception handler. 
*/
void init_handlers(void);

/* System Interrupt/Exception Handlers */
extern void empty_handler_0x01(void);
extern void empty_handler_0x02(void);
extern void empty_handler_0x03(void);
extern void empty_handler_0x04(void);
extern void empty_handler_0x05(void);
extern void empty_handler_0x06(void);
extern void empty_handler_0x07(void);
extern void empty_handler_0x08(void);
extern void empty_handler_0x09(void);
extern void empty_handler_0x0A(void);
extern void empty_handler_0x0B(void);
extern void empty_handler_0x0C(void);
extern void empty_handler_0x0D(void);
extern void empty_handler_0x10(void);
extern void empty_handler_0x11(void);
extern void empty_handler_0x12(void);
extern void empty_handler_0x13(void);

/**
 * @brief Handles division by zero raised for vector 0x0E in IDT.
*/
void zero_division_handler_0x00(void);

/**
 * @brief Handles page faults raised for vector 0x0E in IDT.
*/
void page_fault_handler_0x0E(void);

/**
 * @brief Handles exceptions raise for vector 0x0F in IDT.
*/
void assertion_handler_0x0F(void);

extern void syscall_handler_0x80();

#endif /* ASM */

#endif /* _IDT_HANDLER_ */
