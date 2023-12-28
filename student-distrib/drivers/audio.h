#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "dma.h"
#include "../lib.h"
#include "../page.h"
#include "../i8259.h"

/**
 * @brief Reference: https://wiki.osdev.org/Sound_Blaster_16
 * 
 * @details Note: "-soundhw sb16" must be added to QEMU command line
*/

/**
 * @brief DSP Processor Operations
*/
#define DSP_MIXER_ADDR_PORT                 0x224
#define DSP_MIXER_DATA_PORT                 0x225
#define DSP_RESET_PORT                      0x226
#define DSP_READ_PORT                       0x22A
#define DSP_WRITE_PORT                      0x22C
#define DSP_READ_STATUS_PORT                0x22E
#define DSP_IRQ_16_ACK_PORT                 0x22F

#define DSP_CMD_SET_TIME                    0x40
#define DSP_CMD_SET_OUT_SAMPLE_RATE         0x41
#define DSP_CMD_SET_IN_SAMPLE_RATE          0x42
#define DSP_CMD_TURN_ON_SPEAKER             0xD1
#define DSP_CMD_TURN_OFF_SPEAKER            0xD3
#define DSP_CMD_STOP_8_AUDIO_CHANNEL        0xD0
#define DSP_CMD_RESUME_8_AUDIO_CHANNEL      0xD4
#define DSP_CMD_STOP_16_AUDIO_CHANNEL       0xD5
#define DSP_CMD_RESUME_16_AUDIO_CHANNEL     0xD6
#define DSP_CMD_GET_VERSION                 0xE1
#define MIXER_REG_MASTER_VOLUME             0x22
#define MIXER_REG_SET_IRQ                   0x80
#define MIXER_REG_SET_DMA                   0x81
#define MIXER_REG_IRQ_STATUS                0x82

#define DSP_16_BIT_AUTO_PLAYBACK            0xB6
#define DSP_16_BIT_PLAYBACK                 0xB0
#define DSP_8_BIT_PLAYBACK                  0xC0

#define DSP_AVAILABLE                       0xAA
#define DSP_IRQ2                            0x01
#define DSP_IRQ5                            0x02
#define DSP_IRQ7                            0x04
#define DSP_IRQ10                           0x08
#define DSP_16_BIT_IRQ_TYPE                 0x02

#define SOUNDBLASTER_IRQ                    0x05
#define SOUNDBLASTER_HANDLER                0x20 + 5

#define SAMPLE_RATE                         0xAC44

#define DMA_TRANSFER_COUNT                  0x7FFF
#define DSP_TRANSFER_COUNT                  (DMA_TRANSFER_COUNT - 1)

/**
 * @brief Read from DSP Read Port
*/
uint8_t read_DSP();

/**
 * @brief Write to DSP Port
*/
void write_DSP(uint8_t data);

/**
 * @brief Read Mixer Registers
*/
uint8_t read_mixer(uint32_t reg);

/**
 * @brief Write Mixer Registers
*/
void write_mixer(uint8_t data, uint32_t reg);

/**
 * @brief Initialize SoundBlaster 16 Audio Card
*/
void initialize_audio(void);

/**
 * @brief Reset the DSP Processor in the SoundBlaster 16 Audio Card
*/
void reset_soundblaster(void);

/**
 * @brief Read the DSP Version for SoundBlaster 16 Audio Card
*/
void read_DSP_version(void);

/**
 * @brief Start playing audio
*/
void play_audio(void);

/**
 * @brief Stop playing audio
*/
void stop_audio(void);

/**
 * @brief Load sine wave of certain frequency into DMA buffer
*/
void load_sine_wave(uint32_t frequency);

/**
 * @brief Interrupt handler
*/
void soundblaster_handler(void);

/**
 * @brief Acknowledge interrupt
*/
void ack_interrupt(uint8_t irq_type);

#endif /* _AUDIO_H_ */
