#ifndef _SWITCH_H
#define _SWITCH_H

#include "types.h"

/**
 * @brief Perform a context switch to next process according to
 *        the round robin scheduler.
*/
void context_switch(void);

/**
 * @brief Sets up fake shells by mimicking execute without the iret.
 * 
 * @return 0 on success, -1 on failure
 */
int32_t setup_shell(void);

#endif /* _SWITCH_H */
