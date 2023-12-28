# page.S - Subroutines to setup and enable paging

#define ASM 1

PG_FLAG:    .long 0x80000000    /* Bit 31 of Control Register 0 */
PAE_FLAG:   .long 0x00000020    /* Bit 5 of Control Register 4 */
N_PAE_FLAG: .long 0xFFFFFFDF    /* Negative of Bit 5 for mask */
PSE_FLAG:   .long 0x00000010    /* Bit 4 of Control Register 4 */

.text

.globl page_fault_err_code
page_fault_err_code:        .long 0x00

.align 4
.globl load_page_directory
load_page_directory:
    # Setup
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx     

    # Move address of given page directory to CR3
    movl %cr3, %ebx
    andl $0x000000FF, %ebx
    orl 8(%ebp), %ebx
    movl %ebx, %cr3

    # Teardown
	popl  %ebx

	leave
	ret

.align 4
.globl enable_paging
enable_paging:
    # Setup
	pushl %ebp
	movl %esp, %ebp

	pushl %ebx          

    # !!!! Order matters here. We must modify CR4 first         !!!!
    # !!!! to support simultaneous 4MB & 4KB paging.            !!!! 
    # !!!! Please do not modify, unless you want big headache   !!!!

    # Clear PAE Flag in CR4 / Set PSE Flag in CR4
    movl %cr4, %ebx
    orl PSE_FLAG, %ebx
    andl N_PAE_FLAG, %ebx
    movl %ebx, %cr4

    # Set PG Flag in CR0
    movl %cr0, %ebx
    orl PG_FLAG, %ebx
    movl %ebx, %cr0

    # Teardown
	popl  %edi

	leave
	ret

.align 4
.globl page_fault_linear_address
page_fault_linear_address:
    # Setup
	pushl %ebp
	movl %esp, %ebp

	# Callee Save
	pushl %ebx          
	pushl %esi              
  	pushl %edi

    # Return linear page address in eax
    movl %cr2, %eax

    # Teardown
    popl  %edi
	popl  %esi
	popl  %ebx

	leave
	ret
