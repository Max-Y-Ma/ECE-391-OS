/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define SLAVE_8259_PORT     0xA0

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* This is copied from OSDev, but these are basically the port numbers that the primary and 
secondary pics connect to for commands and data */
#define PIC1		MASTER_8259_PORT		/* IO base address for master PIC */
#define PIC2		SLAVE_8259_PORT 		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define MASK_ALL_INT 0xff 
/*Command word needed to read from the IRR or ISR*/
#define PIC_READ_IRR              0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */


#define PIC2_IRQ 2
#define LOWEST_PRIORITY 7
/*These are helper functions to help us get the irr and isr register values for the two pics*/
uint16_t get_irq_reg(int cont_word, int pic_cmd);
uint16_t get_irr(int pic_cmd);
uint16_t get_isr(int pic_cmd);
unsigned char is_spurious(uint32_t irq_num, uint16_t isr_word);

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init();
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
