#include "spinlock.h"
#include "lib.h"

/**
 * @brief Spin until lock is acquired
 * 
 * @param lock Lock variable
 * 
  * @return Function status
*/
int32_t spin_lock(spinlock_t* lock)
{
    /* Parse argument */
    if (lock == NULL) { return -1; }

    /* Spin until lock is acquired, which is when zero is returned */
    while(try_spin_lock(lock));

    return 0;
}

/**
 * @brief Release spin lock
 * 
 * @param lock Lock variable
 * 
  * @return Function status
*/
int32_t spin_unlock(spinlock_t* lock)
{
    /* Parse argument */
    if (lock == NULL) { return -1; }

    /* Release of the spinlock doesn't require atomicity */
    *lock = SPIN_LOCK_UNLOCKED;

    return 0;
}

/**
 * @brief Save flags and CLI, then spin until lock is acquired
 * 
 * @param lock Lock variable
 * 
  * @return Function status
*/
int32_t spin_lock_irqsave(spinlock_t* lock, uint32_t* flags)
{
    /* Parse argument */
    if (lock == NULL || flags == NULL) { return -1; }

    /* Save flags and cli */
    cli_and_save(*flags);

    /* Acquire spinlock */
    while(try_spin_lock(lock));

    return 0;
}

/**
 * @brief Restore flags, the release spin lock
 * 
 * @param lock Lock variable
 * 
  * @return Function status
*/
int32_t spin_lock_irqrestore(spinlock_t* lock, uint32_t* flags)
{
    /* Parse argument */
    if (lock == NULL || flags == NULL) { return -1; }

    /* Unlock spinlock */
    spin_unlock(lock);

    /* Restore flags */
    restore_flags(*flags);

    return 0;
}
