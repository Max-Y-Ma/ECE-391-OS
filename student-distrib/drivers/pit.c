#include "pit.h"
#include "../lib.h"
#include "../i8259.h"
#include "terminal.h"

/**
 * @brief Initialize programmable interval timer frequency and IRQ
*/
void initialize_pit(void) {
    /* http://kernelx.weebly.com/programmable-interval-timer.html */

    int divisor = PIT_FREQ / 100;
    // int divisor = 10;
    outb(PIT_COMMAND_WORD, PIT_COMMAND_REG);
    outb(divisor & 0xFF, PIT_CHANNEL_0);
    outb(divisor >> 8, PIT_CHANNEL_0);

    enable_irq(PIT_IRQ);
}

/**
 * @brief Interrupt handler for programmable interval timer on IRQ0
*/
void pit_handler(void) {
    
    send_eoi(PIT_IRQ);

    context_switch();
}
