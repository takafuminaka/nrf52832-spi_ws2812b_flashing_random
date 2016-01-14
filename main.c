/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
* @defgroup spi_ws2812b_main main.c
* @{
* @ingroup spi_ws2812b
*
* @brief WS2812B drive by SPI Master sample main file.
*
* This file contains the source code for a sample application using SPI.
*
*/

#include "nrf_delay.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include  "stdlib.h"

#include "ws2812b_driver.h"


const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

#define DELAY_MS                 	(0)                  ///< Timer Delay in milli-seconds.
#define NUM_LEDS									(240)
#define MAX_INTENSE								(16)
#define MAX_INTENSE2							(255)
#define MIN_INTENSE								(1)
#define DECAY_STEP								(30)
#define	PRAB_FLASH								(20000)
#define STEP_SRIDE1								(20)
#define STEP_SRIDE2								(19)

static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);

static volatile bool m_transfer_completed = true;

void volatile spi_master_x_event_handler(nrf_drv_spi_evt_t const * event)
{
		m_transfer_completed = true;
}


/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] p_instance    Pointer to SPI master driver instance.
 * @param[in] lsb           Bits order LSB if true, MSB if false.
 */
static void spi_master_init(nrf_drv_spi_t const * p_instance)
{
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_spi_config_t config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = APP_IRQ_PRIORITY_HIGH,
        .orc          = 0xff,
        .frequency    = NRF_DRV_SPI_FREQ_4M,
        .mode         = NRF_DRV_SPI_MODE_3,
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
    };

    config.sck_pin  = SPIM0_SCK_PIN;
    config.mosi_pin = SPIM0_MOSI_PIN;
    config.miso_pin = NULL;
    nrf_drv_spi_init(p_instance, &config, spi_master_x_event_handler);

}




/** @brief Function for main application entry.
 */
int main(void)
{
		spi_buffer_t spi_buffer_color;
	  nrf_drv_spi_xfer_desc_t xfer_desc0;
		xfer_desc0.p_rx_buffer = NULL;
		xfer_desc0.rx_length   = 0;
	
		rgb_led_t led_array[NUM_LEDS];				// array for base color
		rgb_led_t led_array_flash1[NUM_LEDS]; // array for flash right-up to left-down
		rgb_led_t led_array_flash2[NUM_LEDS]; // array for flash left-up to right-down
		rgb_led_t led_array_work[NUM_LEDS];
	
		uint16_t rest;
		int16_t nextc;
		
    // Configure on-board LED-pins as outputs.
    LEDS_CONFIGURE(LEDS_MASK);

    nrf_drv_spi_t const * p_instance;

    p_instance = &m_spi_master_0;

		// initialize led_array (base color array)
		for(uint16_t i=0;i<NUM_LEDS;i++) {
			int c = (i % 7) + 1;
				led_array[i].green = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&4)>>2);
				led_array[i].red   = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&2)>>1);
				led_array[i].blue  = (MAX_INTENSE+MIN_INTENSE)/2 * ((c&1)>>0);
		}

		// initialize led_array for flash 
		for(uint16_t i=0;i<NUM_LEDS;i++) {
				led_array_flash1[i].green = 0;
				led_array_flash1[i].red   = 0;
				led_array_flash1[i].blue  = 0;

  			led_array_flash2[i].green = 0;
				led_array_flash2[i].red   = 0;
				led_array_flash2[i].blue  = 0;
		}
		
		alloc_spi_buffer(&spi_buffer_color, NUM_LEDS);

		spi_master_init(p_instance);
	
		for (;;)
		{
			LEDS_ON(1 << leds_list[0]);
			LEDS_ON(1 << leds_list[1]);
			LEDS_ON(1 << leds_list[2]);
			LEDS_ON(1 << leds_list[3]);

			LEDS_INVERT(1 << leds_list[0]);
			
			for(uint16_t i=0;i<NUM_LEDS;i++) {
				nextc = led_array[i].green + rand()%3 -1;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE )
				{
					nextc = MAX_INTENSE;
				}
				led_array[i].green = nextc;
				
				nextc = led_array[i].red + rand()%3 -1;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE )
				{
					nextc = MAX_INTENSE;
				}
				led_array[i].red = nextc;

				nextc = led_array[i].blue + rand()%3 -1;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE )
				{
					nextc = MAX_INTENSE;
				}
				led_array[i].blue = nextc;
			}

			// Update led_array_flash1
			for(uint16_t i=0;i<NUM_LEDS;i++)
			{
				if ( rand()%PRAB_FLASH == 0 )
				{
					led_array_flash1[i].green = MAX_INTENSE2;
					led_array_flash1[i].red   = MAX_INTENSE2;
					led_array_flash1[i].blue  = MAX_INTENSE2;
				}
				else if ( i + STEP_SRIDE1 >= NUM_LEDS ) 
				{
					led_array_flash1[i].green = 0;
					led_array_flash1[i].red   = 0;
					led_array_flash1[i].blue  = 0;
				}
				else
				{
					nextc = led_array_flash1[i+STEP_SRIDE1].green - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash1[i].green = nextc;
				
					nextc = led_array_flash1[i+STEP_SRIDE1].red - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash1[i].red = nextc;
				
						nextc = led_array_flash1[i+STEP_SRIDE1].blue - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash1[i].blue = nextc;
				}
			}
			
			// Update led_array_flash2
			for(uint16_t i=0;i<NUM_LEDS;i++)
			{
				if ( rand()%PRAB_FLASH == 0 )
				{
					led_array_flash2[i].green = MAX_INTENSE2;
					led_array_flash2[i].red   = MAX_INTENSE2;
					led_array_flash2[i].blue  = MAX_INTENSE2;
				}
				else if ( i + STEP_SRIDE2 >= NUM_LEDS ) 
				{
					led_array_flash2[i].green = 0;
					led_array_flash2[i].red   = 0;
					led_array_flash2[i].blue  = 0;
				}
				else
				{
					nextc = led_array_flash2[i+STEP_SRIDE2].green - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash2[i].green = nextc;
				
					nextc = led_array_flash2[i+STEP_SRIDE2].red - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash2[i].red = nextc;
				
						nextc = led_array_flash2[i+STEP_SRIDE2].blue - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash2[i].blue = nextc;
				}
			}

			// Merge led arrays	
			for(uint16_t i=0;i<NUM_LEDS;i++)
			{
				nextc = led_array[i].green + led_array_flash1[i].green + led_array_flash2[i].green;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE2 )
				{
					nextc = MAX_INTENSE2;
				}
				led_array_work[i].green = nextc;
				
				nextc = led_array[i].red + led_array_flash1[i].red + led_array_flash2[i].red;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE2 )
				{
					nextc = MAX_INTENSE2;
				}
				led_array_work[i].red = nextc;

				nextc = led_array[i].blue + led_array_flash1[i].blue + led_array_flash2[i].blue;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE2 )
				{
					nextc = MAX_INTENSE2;
				}
				led_array_work[i].blue = nextc;
			}

			// LED update
			set_buff(led_array_work, spi_buffer_color);	// set up SPI buffer
			form_spi_sector(spi_buffer_color);          // set up SPI buffer (EOS)
				
			xfer_desc0.p_tx_buffer = spi_buffer_color.buff;
			xfer_desc0.tx_length    = spi_buffer_color.sector_size;
			rest = spi_buffer_color.length;

			// SPI transfer loop
			while(rest > spi_buffer_color.sector_size)
			{
				m_transfer_completed = false;
				nrf_drv_spi_xfer(p_instance, &xfer_desc0, 0);
				while (! m_transfer_completed) {}

				xfer_desc0.p_tx_buffer += spi_buffer_color.sector_size;
				rest -= spi_buffer_color.sector_size;
			}
			// final buffer
			xfer_desc0.tx_length    = rest;
			m_transfer_completed = false;
			nrf_drv_spi_xfer(p_instance, &xfer_desc0, 0);
			while (! m_transfer_completed) {}
			
			// LED update transfer finished
				
			// delay (LED will be updated this period)
			// nrf_delay_ms(DELAY_MS);
				
			LEDS_INVERT(1 << leds_list[0]); // toggle on-board LED
		}
}

/** @} */
