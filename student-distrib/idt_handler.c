#include "lib.h"
#include "x86_desc.h"
#include "idt_handler.h"
#include "syscalls.h"

/**
 * @brief Handler desciptor table. Holds exception, interrupt, and system call
 *        handlers called from common_interupt linkage.
 */
void(*handler_array[NUM_VEC])(void);

/* Initializes IDTR and the IDT descriptor for each interrupt/exception handler. */
void init_handlers(void)
{
    /* Load IDTR with base address of IDT in x86_desc.S */
    lidt(idt_desc_ptr);

    /* Initialize all descriptors in IDT with linkage stubs */
    /* Exception IDTs*/
    SET_STUB_DESCRIPTOR_TIDT(0x00);
    SET_STUB_DESCRIPTOR_TIDT(0x01);
    SET_STUB_DESCRIPTOR_TIDT(0x02);
    SET_STUB_DESCRIPTOR_TIDT(0x03);
    SET_STUB_DESCRIPTOR_TIDT(0x04);
    SET_STUB_DESCRIPTOR_TIDT(0x05);
    SET_STUB_DESCRIPTOR_TIDT(0x06);
    SET_STUB_DESCRIPTOR_TIDT(0x07);
    SET_STUB_DESCRIPTOR_TIDT(0x08);
    SET_STUB_DESCRIPTOR_TIDT(0x09);
    SET_STUB_DESCRIPTOR_TIDT(0x0A);
    SET_STUB_DESCRIPTOR_TIDT(0x0B);
    SET_STUB_DESCRIPTOR_TIDT(0x0C);
    SET_STUB_DESCRIPTOR_TIDT(0x0D);
    SET_STUB_DESCRIPTOR_TIDT(0x0E);
    SET_STUB_DESCRIPTOR_TIDT(0x0F);
    SET_STUB_DESCRIPTOR_TIDT(0x10);
    SET_STUB_DESCRIPTOR_TIDT(0x11);
    SET_STUB_DESCRIPTOR_TIDT(0x12);
    SET_STUB_DESCRIPTOR_TIDT(0x13);

    /* Interrupt IDTs */
    SET_STUB_DESCRIPTOR_IIDT(0x20);
    SET_STUB_DESCRIPTOR_IIDT(0x21);
    SET_STUB_DESCRIPTOR_IIDT(0x22);
    SET_STUB_DESCRIPTOR_IIDT(0x23);
    SET_STUB_DESCRIPTOR_IIDT(0x24);
    SET_STUB_DESCRIPTOR_IIDT(0x25);
    SET_STUB_DESCRIPTOR_IIDT(0x26);
    SET_STUB_DESCRIPTOR_IIDT(0x27);
    SET_STUB_DESCRIPTOR_IIDT(0x28);
    SET_STUB_DESCRIPTOR_IIDT(0x29);
    SET_STUB_DESCRIPTOR_IIDT(0x2A);
    SET_STUB_DESCRIPTOR_IIDT(0x2B);
    SET_STUB_DESCRIPTOR_IIDT(0x2C);
    SET_STUB_DESCRIPTOR_IIDT(0x2D);
    SET_STUB_DESCRIPTOR_IIDT(0x2E);
    SET_STUB_DESCRIPTOR_IIDT(0x2F);

    /* System Calls */
    SET_STUB_DESCRIPTOR_SIDT(0x80);

    /* Register specific handlers to be called by do_handler() */
    handler_array[ZERO_DIVSION_HANDLER] = zero_division_handler_0x00;
    handler_array[PAGE_FAULT_HANDLER] = page_fault_handler_0x0E;
    handler_array[ASSERTION_HANDLER] = assertion_handler_0x0F;
    handler_array[KEYBOARD_HANDLER] = keyboard_handler;
    handler_array[RTC_HANDLER] = RTC_handler;
    handler_array[PIT_HANDLER] = pit_handler;
    handler_array[SOUNDBLASTER_HANDLER] = soundblaster_handler;

    handler_array[0x01] = empty_handler_0x01;
    handler_array[0x02] = empty_handler_0x02;
    handler_array[0x03] = empty_handler_0x03;
    handler_array[0x04] = empty_handler_0x04;
    handler_array[0x05] = empty_handler_0x05;
    handler_array[0x06] = empty_handler_0x06;
    handler_array[0x07] = empty_handler_0x07;
    handler_array[0x08] = empty_handler_0x08;
    handler_array[0x09] = empty_handler_0x09;
    handler_array[0x0A] = empty_handler_0x0A;
    handler_array[0x0B] = empty_handler_0x0B;
    handler_array[0x0C] = empty_handler_0x0C;
    handler_array[0x0D] = empty_handler_0x0D;
    handler_array[0x10] = empty_handler_0x10;
    handler_array[0x11] = empty_handler_0x11;
    handler_array[0x12] = empty_handler_0x12;
    handler_array[0x13] = empty_handler_0x13;
}

#if C_LINKAGE
/* Pulls vector number from stack and calls the handler in jump table */
void do_handler(pt_regs_t pt_regs)
{
    /* Call system call handler, with special args */
    if (pt_regs.vec_num == 0x80) {
        syscall_handler_0x80(pt_regs);
    } else {
        /* Call to exception and void handlers */
        handler_array[pt_regs.vec_num]();
    }
}
#endif /* C_LINKAGE */

/* System Interrupt/Exception Handlers */

/* TODO: Max thinks that exceptions should halt the current program
        and return execution to the parent process.

    Max thinks that we need to execute the halt system call from the exception handlers.
*/

/* Quits the kernel upon divison by zero.  */
extern void zero_division_handler_0x00(void)
{
    uint32_t eflags;
    cli_and_save(eflags);
    
    KDEBUG("Exception 0x00 Called!\n");

    restore_flags(eflags);

    system_halt (256);
}

/* Handles page faults raised for vector #14 in IDT. */
void page_fault_handler_0x0E(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    /* Handle page fault by allocating new physical page */
    uint32_t linear_address = page_fault_linear_address();
    KDEBUG("Page Fault: Invalid Memory Location 0x%x\n", linear_address);
    KDEBUG("ErrCode: %x\n", page_fault_err_code);

    /* Another page fault can potentially occur during execution of the page-fault handler; 
    the handler should save the contents of the CR2 register before a second page fault can occur. */

    restore_flags(eflags);

    system_halt (256);
}

/* Handles exceptions raise for vector #15 in IDT. */
void assertion_handler_0x0F(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x0F Called: Assertion!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x01(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x01 Called!\n");

    /* Attempt multiple exceptions */
	asm volatile("int $2");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x02(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x02 Called!\n");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x03(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x03 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x04(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x04 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x05(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x05 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x06(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x06 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x07(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x07 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x08(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x08 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x09(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x09 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x0A(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x0A Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x0B(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x0B Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x0C(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x0C Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x0D(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x0D Called: General Protection Fault!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x10(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x10 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x11(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x11 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x12(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x12 Called!");

    restore_flags(eflags);

    system_halt (256);
}

extern void empty_handler_0x13(void)
{
    uint32_t eflags;
    cli_and_save(eflags);

    KDEBUG("Exception 0x13 Called!");

    restore_flags(eflags);

    system_halt (256);
}
