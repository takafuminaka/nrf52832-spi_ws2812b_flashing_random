/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"

void alloc_spi_buffer(spi_buffer_t * spi_buffer, uint16_t num_leds)
{
	spi_buffer->buff = malloc(num_leds * 12);
	spi_buffer->length = num_leds * 12;
	spi_buffer->sector_size = LED_SECTOR_SIZE * 12; 
}

void	form_spi_sector(spi_buffer_t spi_buffer)
{
	uint8_t* p = spi_buffer.buff-1;
	for(uint16_t i=0;i<spi_buffer.length;i+=spi_buffer.sector_size)
	{
		p += spi_buffer.sector_size;
		if ( ( *p & 0x0f ) == PATTERN_0 ) // if the end of sector bit is 0 
		{
			*p = ( *p & 0xf0 ) | PATTERN_0_EOS;
		}
		if ( ( *p & 0x0f ) == PATTERN_1 ) // if the end of sector bit is 1 
		{
			*p = ( *p & 0xf0 ) | PATTERN_1_EOS;
		}
	}
}	

void set_blank(spi_buffer_t spi_buffer)
{
	uint8_t* p =spi_buffer.buff;
	for(uint16_t i=0;i<spi_buffer.length;i++)
	{
		*p = (PATTERN_0 << 4) | PATTERN_0;
		p++;
	}
}

void set_buff(rgb_led_t * rgb_led, spi_buffer_t spi_buffer)
{
	uint8_t* p_spi = spi_buffer.buff;
	rgb_led_t* p_led = rgb_led; 
	for(uint16_t i_led=0;i_led<(spi_buffer.length/12);i_led++)
	{
		uint32_t rgb_data = (p_led->green << 16) | (p_led->red << 8 ) | p_led->blue;
		for(uint8_t i_rgb=0;i_rgb<12;i_rgb++)
		{
			switch(rgb_data & 0x00c00000 )
			{
				case ( 0x00400000 ):
					*p_spi = ( PATTERN_0 << 4 ) | PATTERN_1;
					break;
				case ( 0x00800000 ):
					*p_spi = ( PATTERN_1 << 4 ) | PATTERN_0;
					break;
				case ( 0x00c00000 ):
					*p_spi = ( PATTERN_1 << 4 ) | PATTERN_1;
					break;
				default:
					*p_spi = ( PATTERN_0 << 4 ) | PATTERN_0;
			}
			p_spi++;
			rgb_data <<= 2;
		}
		p_led++;
	}
}
