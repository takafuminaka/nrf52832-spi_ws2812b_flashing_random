/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef WS2812B_DRIVER_H__
#define WS2812B_DRIVER_H__

#include "nrf_drv_spi.h"
#include "stdlib.h"

#define PATTERN_0 (0x08)			// Bit pattern for data "0"
#define PATTERN_1 (0x0e)      // Bit pattern for data "1"
#define PATTERN_0_EOS (0x02)  // BIt pattern for data "0" for end of sector
#define PATTERN_1_EOS (0x0e)  // BIt pattern for data "1" for end of sector

#define	LED_SECTOR_SIZE	(21)	// number of LEDs which is sent in one SPIM transfer (<=21)

typedef struct
{
    uint8_t   green; // Brightness of green (0 to 255)
    uint8_t   red;   // Brightness of red   (0 to 255)
    uint8_t   blue;  // Brightness of blue  (0 to 255)
} rgb_led_t;

typedef struct
{
		uint8_t* buff; // data buffer
		uint16_t length; // buffer size
		uint8_t sector_size; // buffer size per one SPI transaction
} spi_buffer_t;

void alloc_spi_buffer(spi_buffer_t * spi_buffer, uint16_t num_leds);

void sectorize_spi_buffer(spi_buffer_t spi_buffer);

void set_blank(spi_buffer_t spi_buffer);

void set_buff(rgb_led_t* rgb_led, spi_buffer_t spi_buffer);

void form_spi_sector(spi_buffer_t spi_buffer);

#endif // WS2812B_DRIVER_H__

