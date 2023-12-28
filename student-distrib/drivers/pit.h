#ifndef _PIT_H
#define _PIT_H

#include "../types.h"
#include "../switch.h"

#define PIT_IRQ 0

#define PIT_FREQ 1193180

#define PIT_CHANNEL_0 0x40
#define PIT_COMMAND_REG 0x43

/* channel 0, lo/hi, mode 2 */
#define PIT_COMMAND_WORD 0x34

/**
 * @brief Initialize
*/
void initialize_pit(void);

void pit_handler(void);

#endif /* _PIT_H */
