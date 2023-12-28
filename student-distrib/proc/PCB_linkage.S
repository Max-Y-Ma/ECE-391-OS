# PCB_linkage.S - PCB asm stuff

#define ASM 1

# mask the bottom 13 bits to reach the top of each kernel stack
PCB_MASK: .long 0xFFFFE000

.globl get_curr_pcb

# Get current pcb from the current esp
get_curr_pcb:
    PUSHL %EBP
    MOVL  %ESP, %EBP
    PUSHL %EBX
    PUSHL %ESI
    PUSHL %EDI
    PUSHL %ECX

    MOVL  %ESP, %EAX
    ANDL  PCB_MASK, %EAX

    POPL  %ECX
    POPL  %EDI
    POPL  %ESI
    POPL  %EBX
    LEAVE
    RET
