/* Spinlock assembly subroutines */

.text

.align 4
.globl try_spin_lock
try_spin_lock:
    # Setup
	pushl %ebp
	movl %esp, %ebp

    pushl %ebx

    # Load spinlock_t* lock
    movl 8(%ebp), %ebx
    
    # 1 = lock is taken, 0 = lock is free
    movl $1, %eax         

    # Swap eax and *lock
    xchgl %eax, (%ebx)      # xchg is an atomic operation

    # eax = 0 : lock must have been free, we put in 1 signifing it is now taken
    # eax = 1 : lock must have already been acquired, swapping with 1 doesn't affect status

    # Teardown
    popl %ebx

    leave
	ret
