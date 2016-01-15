/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"

static volatile bool spi0_transfer_completed = true;
static volatile bool spi1_transfer_completed = true;
static volatile bool spi2_transfer_completed = true;

void alloc_spi_buffer(spi_buffer_t * spi_buffer, uint16_t num_leds)
{
	spi_buffer->buff = malloc(num_leds * 12);
	spi_buffer->length = num_leds * 12;
	spi_buffer->sector_size = LED_SECTOR_SIZE * 12; 
}

void form_spi_sector(spi_buffer_t spi_buffer)
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

void ws2812b_driver_spi_init(uint8_t id, ws2812b_driver_spi_t * spi)
{
	nrf_drv_spi_handler_t handler;

	nrf_drv_spi_config_t spi_config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = APP_IRQ_PRIORITY_HIGH,
        .frequency    = NRF_DRV_SPI_FREQ_4M,
        .mode         = NRF_DRV_SPI_MODE_1,
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
				.miso_pin 		= NRF_DRV_SPI_PIN_NOT_USED,
    };

		switch(id)
		{
			case(0):
				spi_config.sck_pin  = SPIM0_SCK_PIN;
				spi_config.mosi_pin = SPIM0_MOSI_PIN;
				handler = spi0_event_handler;
				spi->transfer_completed = &spi0_transfer_completed;
			break;
			case(1):
				spi_config.sck_pin  = SPIM1_SCK_PIN;
				spi_config.mosi_pin = SPIM1_MOSI_PIN;
				handler = spi1_event_handler;
				spi->transfer_completed = &spi1_transfer_completed;
				break;
			case(2):
				spi_config.sck_pin  = SPIM2_SCK_PIN;
				spi_config.mosi_pin = SPIM2_MOSI_PIN;
				handler = spi2_event_handler;
				spi->transfer_completed = &spi2_transfer_completed;
				break;
		}
		nrf_drv_spi_init(&(spi->spi), &spi_config, handler);

}	

void spi0_event_handler(nrf_drv_spi_evt_t const * event)
{
		spi0_transfer_completed = true;
}

void spi1_event_handler(nrf_drv_spi_evt_t const * event)
{
		spi1_transfer_completed = true;
}

void spi2_event_handler(nrf_drv_spi_evt_t const * event)
{
		spi2_transfer_completed = true;
}


void ws2812b_driver_xfer(rgb_led_t * led_array, spi_buffer_t spi_buffer, ws2812b_driver_spi_t spi_base)
{
		nrf_drv_spi_t spi = spi_base.spi;
	
	  nrf_drv_spi_xfer_desc_t xfer_desc;

		set_buff(led_array,spi_buffer);
		form_spi_sector(spi_buffer);
	
		xfer_desc.p_tx_buffer = spi_buffer.buff;
		xfer_desc.tx_length    = spi_buffer.sector_size;
		xfer_desc.p_rx_buffer = NULL;
		xfer_desc.rx_length    = NULL;
		
 		uint16_t rest = spi_buffer.length;
			
		// SPI transfer loop
		while(rest > spi_buffer.sector_size)
		{
			*spi_base.transfer_completed = false;
			nrf_drv_spi_xfer(&spi, &xfer_desc, 0);
			while (! *spi_base.transfer_completed) {}

			xfer_desc.p_tx_buffer += spi_buffer.sector_size;
			rest -= spi_buffer.sector_size;
		}
		// final buffer
		xfer_desc.tx_length    = rest;
		*spi_base.transfer_completed = false;
		nrf_drv_spi_xfer(&spi, &xfer_desc, 0);
		while (! *spi_base.transfer_completed) {}

}

void ws2812b_driver_current_cap(rgb_led_t * led_array, uint16_t num_leds, uint32_t limit)
{
	uint32_t sum0 = ws2812b_driver_calc_current(led_array, num_leds);
	if ( sum0 > limit ) {
		// fact = (limit - num_leds) / (sum0 - num_leds);
		int32_t factn = limit - num_leds;
		if ( factn < 0 )
		{
			factn = 1;
		}
		int32_t factd = sum0 - num_leds;
		if ( factd < 0 )
		{
			factd = 1;
		}

		rgb_led_t * p = led_array;	
		rgb_led_t dnext;
		for(uint16_t i=0;i<num_leds;i++)
		{
			dnext.green = p->green * factn / factd;
			dnext.red   = p->red   * factn / factd;
			dnext.blue  = p->blue  * factn / factd;

			if ( dnext.green == 0 && p->green > 0 )
			{
				dnext.green = 1;
			}
			if ( dnext.red   == 0 && p->red   > 0 ) 
			{
				dnext.red   = 1;
			}
			if ( dnext.blue  == 0 && p->blue  > 0 )
			{
				dnext.blue  = 1;
			}
			*p = dnext;
			p++;
		} // i
 	}	
}

uint32_t ws2812b_driver_calc_current(rgb_led_t * led_array, uint16_t num_leds)
{
	uint32_t sum = 0;
	rgb_led_t * p = led_array;
	for(uint16_t i=0;i<num_leds;i++)
	{
		sum += p->green + p->red + p->blue;
		p++;
	}
	
	return(num_leds + (sum*45)/(255*3)); // mA
}
