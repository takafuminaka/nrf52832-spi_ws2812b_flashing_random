/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "ws2812b_driver.h"
#include "nrf_drv_spi.h"
//#include "app_error.h"
//#include "app_util_platform.h"
//#include "bsp.h"

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

//nrf_drv_spi_handler_t spi_master_x_event_handler(nrf_drv_spi_evt_t * event)
//{
//		m_transfer_completed = true;
//}

//void volatile spi_master_x_event_handler(nrf_drv_spi_evt_t const * event)
//{
//		m_transfer_completed = true;
//}


///**@brief Function for initializing a SPI master driver.
// *
// * @param[in] p_instance    Pointer to SPI master driver instance.
// * @param[in] lsb           Bits order LSB if true, MSB if false.
// */
//void spi_master_init(nrf_drv_spi_t const * p_instance)
//{
//    uint32_t err_code = NRF_SUCCESS;

//    nrf_drv_spi_config_t config =
//    {
//        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
//        .irq_priority = APP_IRQ_PRIORITY_HIGH,
//        .orc          = 0xff,
//        .frequency    = NRF_DRV_SPI_FREQ_4M,
//        .mode         = NRF_DRV_SPI_MODE_3,
//        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
//    };

//    config.sck_pin  = SPIM0_SCK_PIN;
//    config.mosi_pin = SPIM0_MOSI_PIN;
//    config.miso_pin = NULL;
//    nrf_drv_spi_init(p_instance, &config, spi_master_x_event_handler); 
//}

