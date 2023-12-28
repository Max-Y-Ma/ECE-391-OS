#include "../types.h"
/* RTC.h - Initialization and interrupt handling of the RTC.
 */

#ifndef _RTC_H
#define _RTC_H

#define RTC_IRQ 8

/* See os dev RTC page for details */
#define RTC_CONTROL_PORT 0x70
#define RTC_DATA_PORT 0x71
#define RTC_ICWA 0x8A
#define RTC_ICWB 0x8B
#define RTC_ICWC 0x0C

#define RTC_MAX_DIVIDER 3

/* We tried this with 8192, but it was too fast, so we set it back to 1024. */
#define RTC_1024_SET 6 /* This was originally 3*/
#define RTC_2HZ_ROLLOVER 512 /* This was originally 4096*/
#define RTC_ROLLOVER_MAX 1024 /* This was originally 8192 */

// extern int rtc_flag;

extern device_op_table_t RTC_op_table;

/**
 * @brief Initializes the RTC.
 */
void initialize_RTC(void);

/**
 * @brief Handles RTC interrupts.
 */
void RTC_handler(void);

/**
 * @brief Open RTC, set frequency = 2Hz
 */
int32_t RTC_open(const uint8_t* filename);

/**
 * @brief Close RTC
 */
int32_t RTC_close(int32_t fd);

/**
 * @brief Read RTC, wait till interrupt
 */
int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes);

/**
 * @brief Write to RTC, modify rate
 * 
 * @param buf stores the new frequency rate
 */
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes);

/**
 * @brief Get virtualized frequency rollover
*/
uint32_t get_rollover_freq(void);

/**
 * @brief Get virtualized frequency rollover
*/
uint32_t get_counter_val(void);

/**
 * @brief Get virtualized frequency rollover
*/
uint32_t get_interrupt_occ(void);

/**
 * @brief Set virtualized frequency rollover
*/
void set_rollover_freq(uint32_t freq);

/**
 * @brief Set virtualized frequency rollover
*/
void set_counter_val(uint32_t val);

/**
 * @brief Set virtualized frequency rollover
*/
void set_interrupt_occ(uint32_t val);

#endif
