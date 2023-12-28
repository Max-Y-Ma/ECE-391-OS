#include "RTC.h"
#include "../i8259.h"
#include "../lib.h"
#include "terminal.h"

/* Global Terminal Control Block */
extern terminal_info_t tcb[MAX_NUM_TERMINAL];

/* RTC op table for PCB jumps, see types.h */
device_op_table_t RTC_op_table = {
    RTC_read,
    RTC_write,
    RTC_open,
    RTC_close
};

/* Initializes the RTC. */
void initialize_RTC(void)
{
    char prev;
    
    /* We must disable NMI to prevent RTC from entering an undefined state. */
    NMI_disable();

    /* set the bottom 4 bits of reg A to 3, this is the max freq: 8 KHz */
    outb(RTC_ICWA, RTC_CONTROL_PORT);
    prev = inb(RTC_DATA_PORT);
    outb(RTC_ICWA, RTC_CONTROL_PORT);
    outb((prev & 0xF0) | (RTC_1024_SET & 0x0F), RTC_DATA_PORT);

    /* Enable periodic interrupts */
    outb(RTC_ICWB, RTC_CONTROL_PORT);
    prev = inb(RTC_DATA_PORT);
    outb(RTC_ICWB, RTC_CONTROL_PORT);

    /* Set bit 7, which enables periodic interrupts. */
    outb(prev | 0x40, RTC_DATA_PORT);
    NMI_enable();

    enable_irq(RTC_IRQ);
}

/* Handles RTC interrupts. */
void RTC_handler(void)
{
    /* Only handle when we reach the frequency rollover */
    int c;
    for (c = 0; c < MAX_NUM_TERMINAL; c++) {
        tcb[c].rtc_freq_counter++;
        if (tcb[c].rtc_freq_counter >= tcb[c].rtc_freq_rollover){
            /* Do stuff */
            tcb[c].rtc_freq_counter = 0;
            if(tcb[c].rtc_interrupt_occurred == 0)
                tcb[c].rtc_interrupt_occurred = 1;
        }   
    }

    /* Read from reg C to continue sending interrupts */
    outb(RTC_ICWC, RTC_CONTROL_PORT);
    inb(RTC_DATA_PORT);

    send_eoi(RTC_IRQ);
}

/**
 * @brief Open RTC, set frequency = 2Hz
 */
int32_t RTC_open(const uint8_t* filename){
    if(filename == NULL)
        return -1;
    int idx = tcb_get_curr_idx();
    tcb[idx].rtc_freq_rollover = RTC_2HZ_ROLLOVER;
    return 0;
}

/**
 * @brief Close RTC
 */
int32_t RTC_close(int32_t fd){
    int idx = tcb_get_curr_idx();

    tcb[idx].rtc_freq_rollover = RTC_2HZ_ROLLOVER;
    return 0;
}

/**
 * @brief Read RTC, wait till interrupt
 */
int32_t RTC_read(int32_t fd, void* buf, int32_t nbytes) {

    tcb[tcb_get_curr_idx()].rtc_interrupt_occurred = 0;
    tcb[tcb_get_curr_idx()].rtc_freq_counter = 0;
    while(!tcb[tcb_get_curr_idx()].rtc_interrupt_occurred){

    }
    tcb[tcb_get_curr_idx()].rtc_interrupt_occurred = 0;
    return 0;
}

/**
 * @brief Write to RTC, modify rate
 */
int32_t RTC_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(buf == NULL)
        return -1;
    
    int32_t value = *(int32_t*)buf;

    //Too high a rate or too low not allowed
    if(value == 0 || value >= RTC_ROLLOVER_MAX)
        return -1;
    //If value is not a power of 2
    if(value & (value - 1))
        return -1;
    
    //Set rate
    tcb[tcb_get_curr_idx()].rtc_freq_rollover = RTC_ROLLOVER_MAX / value;
    return 0;
}

/**
 * @brief Get virtualized frequency rollover
*/
uint32_t get_rollover_freq(void)
{
    
    return tcb[tcb_get_curr_idx()].rtc_freq_rollover;
}

/**
 * @brief Set virtualized frequency rollover
*/
void set_rollover_freq(uint32_t freq)
{
    tcb[tcb_get_curr_idx()].rtc_freq_rollover = freq;
}

/**
 * @brief Get virtualized counter
*/
uint32_t get_counter_val(void){
    return tcb[tcb_get_curr_idx()].rtc_freq_counter;
}

/**
 * @brief (re)Set virtualized rollover counter
*/
void set_counter_val(uint32_t val){
    tcb[tcb_get_curr_idx()].rtc_freq_counter = val;
}

/**
 * @brief Get virtualized frequency rollover
*/
uint32_t get_interrupt_occ(void){
    return tcb[tcb_get_curr_idx()].rtc_interrupt_occurred;
}

/**
 * @brief Set virtualized frequency rollover
*/
void set_interrupt_occ(uint32_t val){
    tcb[tcb_get_curr_idx()].rtc_interrupt_occurred = val;
}
