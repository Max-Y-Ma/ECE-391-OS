#ifndef _DMA_H_
#define _DMA_H_

#include "../lib.h"

/**
 * @brief Reference: https://osdev.org/ISA_DMA
*/

/**
 * @brief The DMA controllers (8237A) are setup in cascading mode
 *        Each 8237A has 18 registers that are addressed via I/O Port bus.
 * 
 * @details Channels 0-3 are 8 bit transfer capable, while channels 4-7 are
 *          16 bit transfer capable. Channel 4 is dedicated to cascading. 
*/

/**
 * @brief Channel 0-3 Register Ports,
 *        _8 indicates support of 8 bit transfer
*/
#define DMA_REG_START_ADDR_0_8          0x00
#define DMA_REG_COUNT_0_8               0x01
#define DMA_REG_START_ADDR_1_8          0x02
#define DMA_REG_COUNT_1_8               0x03
#define DMA_REG_START_ADDR_2_8          0x04
#define DMA_REG_COUNT_2_8               0x05
#define DMA_REG_START_ADDR_3_8          0x06
#define DMA_REG_COUNT_3_8               0x07
#define DMA_REG_STATUS_8                0x08
#define DMA_REG_COMMAND_8               0x08
#define DMA_REG_REQUEST_8               0x09
#define DMA_REG_CHANNEL_MASK_8          0x0A
#define DMA_REG_MODE_8                  0x0B
#define DMA_REG_FLIP_FLOP_RESET_8       0x0C
#define DMA_REG_INTERMEDIATE_8          0x0D
#define DMA_REG_MASTER_RESET_8          0x0D
#define DMA_REG_MASK_RESET_8            0x0E
#define DMA_REG_MULTICHANNEL_MASK_8     0x0F

/**
 * @brief Channel 4-7 Register Ports,
 *        _16 indicates support of 16 bit transfer
*/
#define DMA_REG_START_ADDR_4_16         0xC0
#define DMA_REG_COUNT_4_16              0xC2
#define DMA_REG_START_ADDR_5_16         0xC4
#define DMA_REG_COUNT_5_16              0xC6
#define DMA_REG_START_ADDR_6_16         0xC8
#define DMA_REG_COUNT_6_16              0xCA
#define DMA_REG_START_ADDR_7_16         0xCC
#define DMA_REG_COUNT_7_16              0xCE
#define DMA_REG_STATUS_16               0xD0
#define DMA_REG_COMMAND_16              0xD0
#define DMA_REG_REQUEST_16              0xD2
#define DMA_REG_CHANNEL_MASK_16         0xD4
#define DMA_REG_MODE_16                 0xD6
#define DMA_REG_FLIP_FLOP_RESET_16      0xD8
#define DMA_REG_INTERMEDIATE_16         0xDA
#define DMA_REG_MASTER_RESET_16         0xDA
#define DMA_REG_MASK_RESET_16           0xDC
#define DMA_REG_MULTICHANNEL_MASK_16    0xDE

/**
 * @brief Channel R/W Page Address Registers
*/
#define DMA_REG_PAGE_ADDR_0             0x87
#define DMA_REG_PAGE_ADDR_1             0x83
#define DMA_REG_PAGE_ADDR_2             0x81
#define DMA_REG_PAGE_ADDR_3             0x82
#define DMA_REG_PAGE_ADDR_4             0x8F
#define DMA_REG_PAGE_ADDR_5             0x8B
#define DMA_REG_PAGE_ADDR_6             0x89
#define DMA_REG_PAGE_ADDR_7             0x8A

#define AUTO_TRANSFER_PERIPHERAL_READ_MODE     0x59

#endif
