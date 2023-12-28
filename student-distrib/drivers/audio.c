#include "audio.h"
#include "audio_sample.h"

/* Pause Time */
#define WAIT()                              \
do {                                        \
    int i;                                  \
    for (i = 0; i < 10000000; i++) { }      \
} while(0)  

static uint8_t major_DSP_version;   /* Major Version: 4 */
static uint8_t minor_DSP_version;   /* Minor Version: 5 */
static uint8_t soundblaster_available = 0;

/**
 * @brief Read from DSP Read Port
*/
uint8_t read_DSP()
{
    /* Check bit 7 of Read Status Port for available data */
    uint8_t read_status;
    do {
        read_status = inb(DSP_READ_STATUS_PORT);
    } while(!(read_status & 0x80));
    
    /* Return data */
    return inb(DSP_READ_PORT);
}

/**
 * @brief Write to DSP Port
*/
void write_DSP(uint8_t data)
{
    /* Check bit 7 of Write Status Port for ready signal */
    uint8_t read_status;
    do {
        read_status = inb(DSP_WRITE_PORT);
    } while(read_status & 0x80);

    outb(data, DSP_WRITE_PORT);
}

/**
 * @brief Read Mixer Registers
*/
uint8_t read_mixer(uint32_t reg)
{
    /* Write index of mixer register to Address Port */
    outb(reg, DSP_MIXER_ADDR_PORT);

    /* Read mixer register value from Data Port */
    return inb(DSP_MIXER_DATA_PORT);
}

/**
 * @brief Write Mixer Registers
*/
void write_mixer(uint8_t data, uint32_t reg)
{
    /* Write index of mixer register to Address Port */
    outb(reg, DSP_MIXER_ADDR_PORT);

    /* Write mixer register value from Data Port */
    outb(data, DSP_MIXER_DATA_PORT);
}

/**
 * @brief Initialize SoundBlaster 16 Audio Card
*/
void initialize_audio(void)
{
    /* Reset DSP */
    reset_soundblaster();

    /* Verify that SoundBlaster 16 is available */
    if (read_DSP() == DSP_AVAILABLE) {
        soundblaster_available = 1;
    } else {
        return;
    }

    /* Read DSP Version */
    read_DSP_version();

    /* Initialize DMA channel #5: 16-bit transfers */
    /* Mask channel #5 */
    outb(0x05, DMA_REG_CHANNEL_MASK_16);

    /* Reset Flip-Flop port */
    outb(0x01, DMA_REG_FLIP_FLOP_RESET_16);

    /* Set single DMA transfer mode, peripheral read*/
    outb(AUTO_TRANSFER_PERIPHERAL_READ_MODE, DMA_REG_MODE_16);

    /* Set external page base address */
    outb(((DMA_BLOCK_START_ADDR_CHANNEL5 & 0xFF0000) >> 16), DMA_REG_PAGE_ADDR_5);      /* Page Number */
    outb((DMA_BLOCK_START_ADDR_CHANNEL5 & 0x0000FF), DMA_REG_START_ADDR_5_16);          /* Low Bits */
    outb(((DMA_BLOCK_START_ADDR_CHANNEL5 & 0x00FF00) >> 8), DMA_REG_START_ADDR_5_16);   /* High Bits */

    /* Set transfer length */
    outb(DMA_TRANSFER_COUNT & 0x00FF, DMA_REG_COUNT_5_16);          /* Low Bits */
    outb((DMA_TRANSFER_COUNT & 0xFF00) >> 8, DMA_REG_COUNT_5_16);   /* High Bits */

    /* Enable Channel #5 */
    outb(0x01, DMA_REG_CHANNEL_MASK_16);

    /* Initialize Sound Blaster 16 */
    /* Set IRQ Number */
    write_mixer(DSP_IRQ5, MIXER_REG_SET_IRQ);
    if (read_mixer(MIXER_REG_SET_IRQ) != DSP_IRQ5) {
        return;
    }
    enable_irq(SOUNDBLASTER_IRQ);

    /* Set DMA Channel */
    write_mixer(0x20, MIXER_REG_SET_DMA);

    /* Turn on speaker*/
    write_DSP(DSP_CMD_TURN_ON_SPEAKER);

    /* Set max volume */
    write_mixer(0xFF, MIXER_REG_MASTER_VOLUME);

    /* Set Input Sample Rate */
    write_DSP(DSP_CMD_SET_IN_SAMPLE_RATE);
    write_DSP((SAMPLE_RATE & 0xFF00) >> 8);     /* High Bits */
    write_DSP(SAMPLE_RATE & 0x00FF);            /* Low Bits */

    /* Set Output Sample Rate */
    write_DSP(DSP_CMD_SET_OUT_SAMPLE_RATE);
    write_DSP((SAMPLE_RATE & 0xFF00) >> 8);     /* High Bits */
    write_DSP(SAMPLE_RATE & 0x00FF);            /* Low Bits */
}

/**
 * @brief Reset the DSP Processor in the SoundBlaster 16 Audio Card
*/
void reset_soundblaster(void)
{
    /* Reset sound blaster */
    outb(0x01, DSP_RESET_PORT);
    /* Wait at least 3ms */
    WAIT();
    outb(0x00, DSP_RESET_PORT);
}

/**
 * @brief Read the DSP Version for SoundBlaster 16 Audio Card
*/
void read_DSP_version(void)
{
    write_DSP(DSP_CMD_GET_VERSION);
    major_DSP_version = read_DSP();
    minor_DSP_version = read_DSP();
}

/**
 * @brief Start playing audio
*/
void play_audio(void)
{
    write_DSP(DSP_16_BIT_AUTO_PLAYBACK);
    write_DSP(0x10);    /* Set Sound Data Type: Mono, Unsigned */
    write_DSP(DSP_TRANSFER_COUNT & 0x00FF);    /* Set Count Length */ /* Low Bits */
    write_DSP((DSP_TRANSFER_COUNT & 0xFF00) >> 8);    /* High Bits */
}

/**
 * @brief Stop playing audio
*/
void stop_audio(void)
{
    write_DSP(DSP_16_BIT_PLAYBACK);
    write_DSP(0x10);    /* Set Sound Data Type: Mono, Unsigned */
    write_DSP(0x00);    /* Set Count Length */ /* Low Bits */
    write_DSP(0x00);    /* High Bits */
}

/**
 * @brief Load sine wave of certain frequency into DMA buffer
*/
void load_sine_wave(uint32_t frequency)
{
    if (0 <= frequency && frequency <= 100) {
        memcpy((uint8_t*)DMA_BLOCK_START_ADDR_CHANNEL5, (uint8_t*)sine_wave_100, (uint32_t)(2 * DMA_TRANSFER_COUNT));
    }
    else if (100 <= frequency && frequency <= 400) {
        memcpy((uint8_t*)DMA_BLOCK_START_ADDR_CHANNEL5, (uint8_t*)sine_wave_400, (uint32_t)(2 * DMA_TRANSFER_COUNT));
    }
    else if (400 <= frequency && frequency <= 1000) {
        memcpy((uint8_t*)DMA_BLOCK_START_ADDR_CHANNEL5, (uint8_t*)sine_wave_1000, (uint32_t)(2 * DMA_TRANSFER_COUNT));
    }
    else {
        memcpy((uint8_t*)DMA_BLOCK_START_ADDR_CHANNEL5, (uint8_t*)sine_wave_4000, (uint32_t)(2 * DMA_TRANSFER_COUNT));
    }
}

/**
 * @brief Interrupt handler
*/
void soundblaster_handler(void)
{
    /* Acknowledge 16-bit interrupt */
    ack_interrupt(read_mixer(MIXER_REG_IRQ_STATUS));

    send_eoi(SOUNDBLASTER_IRQ);
}

/**
 * @brief Acknowledge interrupt
*/
void ack_interrupt(uint8_t irq_type)
{
    if (irq_type == DSP_16_BIT_IRQ_TYPE) {
        inb(DSP_IRQ_16_ACK_PORT);
    }
}
