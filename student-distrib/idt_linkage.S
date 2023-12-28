#define ASM 1
#include "idt_handler.h"
#include "x86_desc.h"

#define DO_CALL(name,number)     \
.globl name                     ;\
name:   pushl	%ebx            ;\
	    movl	$number,%eax    ;\
	    movl	8(%esp),%ebx    ;\
	    movl	12(%esp),%ecx   ;\
	    movl	16(%esp),%edx   ;\
	    int	    $0x80           ;\
	    popl	%ebx            ;\
	    ret

DO_CALL(system_halt_wrapper, 1)
DO_CALL(system_execute_wrapper, 2)
DO_CALL(system_read_wrapper, 3)
DO_CALL(system_write_wrapper, 4)
DO_CALL(system_open_wrapper, 5)
DO_CALL(system_close_wrapper, 6)
DO_CALL(system_getargs_wrapper, 7)
DO_CALL(system_vidmap_wrapper, 8)
DO_CALL(system_set_handler_wrapper, 9)
DO_CALL(system_sigreturn_wrapper, 10)
DO_CALL(system_malloc_wrapper, 11)
DO_CALL(system_free_wrapper, 12)
DO_CALL(system_ioctl_wrapper, 13)

# System call functions
.globl system_halt          # 1
.globl system_execute       # 2
.globl system_read          # 3
.globl system_write         # 4
.globl system_open          # 5
.globl system_close         # 6
.globl system_getargs       # 7
.globl system_vidmap        # 8
.globl system_set_handler   # 9
.globl system_sigreturn     # 10
.globl system_malloc        # 11
.globl system_free          # 12
.globl system_ioctl         # 13

# Writing linkage from specific IDT vector to a C function that handles the corresponding interrupt
# Inputs: Interrupt number, arguments (for syscalls)
# Outputs: N.A.
# Effects: Pushes registers onto the stack to save them according to Intel standards
# Calls the right interrupt handler
.text

.align 4

# Linkage Stubs for Exceptions
LINKAGE_STUB(0x00)
LINKAGE_STUB(0x01)
LINKAGE_STUB(0x02)
LINKAGE_STUB(0x03)
LINKAGE_STUB(0x04)
LINKAGE_STUB(0x05)
LINKAGE_STUB(0x06)
LINKAGE_STUB(0x07)
LINKAGE_STUB(0x08)
LINKAGE_STUB(0x09)
LINKAGE_STUB(0x0A)
LINKAGE_STUB(0x0B)
LINKAGE_STUB(0x0C)
LINKAGE_STUB(0x0D)

# Linkage stub for IDT Page Faults
.globl interrupt_0x0E_linkage
interrupt_0x0E_linkage:
    # Set and pop current page fault error code
    pushl %eax
    movl 4(%esp), %eax 
    movl %eax, page_fault_err_code
    popl %eax
    addl $4, %esp
    pushl $0x0E
    jmp common_interrupt_handler

LINKAGE_STUB(0x0F)
LINKAGE_STUB(0x10)
LINKAGE_STUB(0x11)
LINKAGE_STUB(0x12)
LINKAGE_STUB(0x13)

# Linkage Stubs for IRQs
LINKAGE_STUB(0x20)
LINKAGE_STUB(0x21)
LINKAGE_STUB(0x22)
LINKAGE_STUB(0x23)
LINKAGE_STUB(0x24)
LINKAGE_STUB(0x25)
LINKAGE_STUB(0x26)
LINKAGE_STUB(0x27)
LINKAGE_STUB(0x28)
LINKAGE_STUB(0x29)
LINKAGE_STUB(0x2A)
LINKAGE_STUB(0x2B)
LINKAGE_STUB(0x2C)
LINKAGE_STUB(0x2D)
LINKAGE_STUB(0x2E)
LINKAGE_STUB(0x2F)
    
# To be used by all interrupts and exceptions
common_interrupt_handler:
    pushl %es
    pushl %ds
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    # Call handler at vector number $esp + 36
    # Vector number was pushed by linkage stubs
    movl 36(%esp), %ebx 
    call *handler_array(, %ebx, 4)

    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    popl %ds
    popl %es
    addl $4, %esp   # pop vector number
    iret

# System Calls
.globl interrupt_0x80_linkage
interrupt_0x80_linkage:
    # Push gen-purpose registers besides eax
    pushl %es
    pushl %ds
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    # Set Kernel DS
    pushl %eax
    movw $KERNEL_DS, %ax
    movw %ax, %ds
    popl %eax

    # Call system call dispatcher
    call syscall_handler_0x80

    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %ds
    popl %es
    iret

# Syscall jumptable: Calls the actual syscall depending on the number in EAX
.globl syscall_handler_0x80
syscall_handler_0x80:
    # Check bounds: 0 < number <= 13
    cmpl $13, %eax
    ja invalid_sys
    cmpl $0, %eax
    je invalid_sys
    addl $-1, %eax
    # No issue with number, jump to right function
    jmp *system_jumptable(, %eax, 4)

invalid_sys:
    movl $-1, %eax
	ret

.globl fake_iret
fake_iret:
    iret

# Jumptable to syscall
.align 4
system_jumptable:
.long system_halt           # 1
.long system_execute        # 2
.long system_read           # 3
.long system_write          # 4
.long system_open           # 5
.long system_close          # 6
.long system_getargs        # 7
.long system_vidmap         # 8
.long system_set_handler    # 9
.long system_sigreturn      # 10
.long system_malloc         # 11
.long system_free           # 12
.long system_ioctl          # 13
