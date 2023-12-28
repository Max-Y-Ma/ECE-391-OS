#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"

#define SPIN_LOCK_UNLOCKED 0

/* Spinlock data type */
typedef volatile uint32_t spinlock_t;

/**
 * @brief Spin until lock is acquired
 * 
 * @param lock Lock variable
 * 
 * @return Function status
*/
int32_t spin_lock(spinlock_t* lock);

/**
 * @brief Release spin lock
 * 
 * @param lock Lock variable
 * 
 * @return Function status
*/
int32_t spin_unlock(spinlock_t* lock);

/**
 * @brief Save flags and CLI, then spin until lock is acquired
 * 
 * @param lock Lock variable
 * 
 * @return Function status
*/
int32_t spin_lock_irqsave(spinlock_t* lock, uint32_t* flags);

/**
 * @brief Restore flags, the release spin lock
 * 
 * @param lock Lock variable
 * 
 * @return Function status
*/
int32_t spin_lock_irqrestore(spinlock_t* lock, uint32_t* flags);

/* Spin lock helper functions */
extern uint32_t try_spin_lock(spinlock_t* lock);

#endif /* _SPINLOCK_H_ */
