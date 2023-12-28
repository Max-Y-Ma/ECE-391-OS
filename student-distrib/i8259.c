/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Static variable for the number of spurious interrupt */
static int num_spurious;

/* Initialize the 8259 PIC */
void i8259_init() {
    num_spurious = 0;
    master_mask = MASK_ALL_INT; // set both so all interrupt lines are masked
    slave_mask = MASK_ALL_INT;

    /*We would normally spin lock here, but we don't have that yet. So, until threading is enabled, no spinlock is grabbed*/

    cli(); /* I would call this to mask all interrupts. sti() will be called once the function exits.*/

    outb(ICW1, PIC1_COMMAND); /*Send the first control word to the Command port of the master*/
    io_wait();
    outb(ICW2_MASTER, PIC1_DATA); /* Send the second control word to data port of the master*/
    io_wait();
    outb(ICW3_MASTER, PIC1_DATA); /* Send the third control word to data port of the master*/
    io_wait();
    // if (eoi) { // we probably don't need this
    outb(ICW4, PIC1_DATA); /*Send the fourth control word to data port of the master*/
    io_wait();
    //} else {
    // outb(ICW4, PIC1_DATA);
    //}

    outb(ICW1, PIC2_COMMAND);      /*Send first cw to command port of slave*/
    io_wait();
    outb(ICW2_SLAVE, PIC2_DATA); /*Send second to data of slave*/
    io_wait();
    outb(ICW3_SLAVE, PIC2_DATA); /*Send third to data of slave*/
    io_wait();
    outb(ICW4, PIC2_DATA); /* Send fourth to data of slave*/
    io_wait();


    outb(master_mask, PIC1_DATA); /* This writes the byte 0xff to the port 0x21 (pic1 data), where the master PIC is connected to 
                        the processor, therefore masking all of the pic interrupt lines. */
    io_wait();
    outb(slave_mask, PIC2_DATA);  /* Same thing, but to the port 0xa1 (pic2 data), where the slave PIC is connected to the processor.*/
    io_wait();

    enable_irq(PIC2_IRQ);
    io_wait();
    
}

/* Enable (unmask) the specified IRQ */
/*Most of this code is derived from OSDev, but the ideas are explained in the comments*/
void enable_irq(uint32_t irq_num) {
    if (irq_num < 0 || irq_num > 15) {
        /* Assert general protection fault */
        ///TODO: not sure if this is the correct protocol...
        asm volatile("int $0x0D");
    }
    uint16_t data_port; // port that we send data to. the data will be a single byte conveying which irqs to mask and not to mask
    if (irq_num < 8) { // if it's between 0-7, we send to the master pic. 
        data_port = PIC1_DATA; // data port is now the port associated with the master pic's data
        master_mask &= ~(1 << irq_num ); /*force the (irq_num)th bit of master_mask to be 0. We use 1 to give us a 
        number that starts with 0000 0001, then do the left shifting by irq_num bits. doing this unmasks that irq num*/
        
        outb(master_mask, data_port); //In addition to setting the mask, write to that data_port
        io_wait();

    } else if (irq_num  < 16) { // if it's between 8-15, we send to the slave pic. we then need to subtract 8 to account for that offset.
        data_port = PIC2_DATA; // data port is now the port associated with slave pic's data
        irq_num -= 8; // explained two comments earlier
        slave_mask &= ~(1 << irq_num);  /*force the (irq_num)th bit of slave_mask to be 0. We use 1 to give us a 
        number that starts with 0000 0001, then do the left shifting by irq_num bits. Doing this unmasks that irq num*/ 
        outb(slave_mask, data_port); // in addition to setting the mask, write to that data_port
        io_wait();
    } else { // if it's not between 0-15, what would I do? This is not applicable
        return;
    }

    

}

/* Disable (mask) the specified IRQ */
/*Most of this code is derived from OSDev, but the ideas are explained in the comments*/

void disable_irq(uint32_t irq_num) {
    if (irq_num < 0 || irq_num > 15) {
        /* Assert general protection fault */
        ///TODO: not sure if this is the correct protocol...
        asm volatile("int $0x0D");
    }
    uint16_t data_port; // port that we send data to. the data will be a single byte conveying which irqs to mask and not to mask
    if (irq_num < 8) { // if it's between 0-7, we send to the master pic. 
        data_port = PIC1_DATA; // data port is now the port associated with the master pic's data
        master_mask |= (1 << irq_num ); /*force the (irq_num)th bit of master_mask to be 1. We use 1 to give us a 
        number that starts with 0000 0001, then do the left shifting by irq_num bits. doing this masks that irq num*/
        
        outb(master_mask, data_port); //In addition to setting the mask, write to that data_port
        io_wait();

    } else if (irq_num  < 16) { // if it's between 8-15, we send to the slave pic. we then need to subtract 8 to account for that offset.
        data_port = PIC2_DATA; // data port is now the port associated with slave pic's data
        irq_num -= 8; // explained two comments earlier
        slave_mask |= (1 << irq_num);  /*force the (irq_num)th bit of slave_mask to be 1. We use 1 to give us a 
        number that starts with 0000 0001, then do the left shifting by irq_num bits. Doing this masks that irq num*/ 
        outb(slave_mask, data_port); // in addition to setting the mask, write to that data_port
        io_wait();
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if (irq_num < 0 || irq_num > 15) {
        /* Assert general protection fault */
        ///TODO: not sure if this is the correct protocol...
        asm volatile("int $0x0D");
    }
    if (irq_num >= 8) { /* If the irq_num is greater than 7, we also need to send this to the slave PIC*/
        
        if (irq_num == 15) {
            uint16_t isr_word = get_irq_reg(PIC_READ_ISR,PIC2_COMMAND);
            if (is_spurious(irq_num, isr_word)) {
                num_spurious += 1;
                outb( EOI | (PIC2_IRQ),PIC1_COMMAND);
                io_wait();
                return;
            }
        }

        
        outb( EOI | (irq_num - 8), PIC2_COMMAND);
        io_wait();
        outb( EOI | PIC2_IRQ, PIC1_COMMAND);
        io_wait();
    }
    else {
        if (irq_num == 7) {
            uint16_t isr_word = get_irq_reg(PIC_READ_ISR,PIC2_COMMAND);
            if (is_spurious(irq_num, isr_word)) {
                num_spurious += 1;
                io_wait();
                return;
            }
        }

        outb( EOI | irq_num, PIC1_COMMAND); // do I do EOI | irq_num or just 0x20 as said in OSDev? probably the former, as it's suggested in the h file
        io_wait();
    }

}

/* This gets us the register value for the isr or the irr.*/
uint16_t get_irq_reg(int cont_word, int pic_cmd){
    if (pic_cmd == PIC1_COMMAND) { /*For master pic*/
        outb(cont_word, PIC1_COMMAND);
        return inb(PIC1_COMMAND); 
    } else if (pic_cmd == PIC2_COMMAND){ /* For slave pic*/
        outb(cont_word, PIC2_COMMAND);
        return inb(PIC2_COMMAND);
    }  else { /* This line should never execute*/
        return 0;
    }
}

uint16_t get_irr(int pic_cmd) {
        if (pic_cmd == PIC1_COMMAND) {
            return get_irq_reg(PIC_READ_IRR, PIC1_COMMAND);
        } else if (pic_cmd == PIC2_COMMAND){
            return get_irq_reg(PIC_READ_IRR, PIC2_COMMAND);
        } else {
            return 0;
        }
}

uint16_t get_isr(int pic_cmd) {
    if (pic_cmd == PIC1_COMMAND) {
            return get_irq_reg(PIC_READ_ISR, PIC1_COMMAND);
        } else if (pic_cmd == PIC2_COMMAND){
            return get_irq_reg(PIC_READ_ISR, PIC2_COMMAND);
        } else {
            return 0;
        }

}
unsigned char is_spurious(uint32_t irq_num, uint16_t isr_word) {
    unsigned char look_at_bit = ( unsigned char)(isr_word) &  (1 << irq_num)    ; /* See if this bit corresponding to irq_num is set*/
    if (look_at_bit == 0) {
        return 1; /* the bit being 0 means spurious.  */
    }
    return 0; /* the bit being 1 means not spurious. */
}

