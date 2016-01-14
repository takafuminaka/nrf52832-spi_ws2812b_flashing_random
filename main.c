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
#include "nrf_drv_spi.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include  "stdlib.h"

#include "ws2812b_driver.h"


const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

#define DELAY_MS                 	(5)                  ///< Timer Delay in milli-seconds.
#define NUM_LEDS									(240)
#define MAX_INTENSE								(16)
#define MAX_INTENSE2							(255)
#define MIN_INTENSE								(1)
#define DECAY_STEP								(30)
#define	PRAB_FLASH								(5000)
#define STEP_SRIDE1								(-20)
#define STEP_SRIDE2								(-18)

#define SPI_INSTANCE  0 /**< SPI instance index. */

static ws2812b_driver_spi_t spi0 = {
	.spi = NRF_DRV_SPI_INSTANCE(0)
};
static ws2812b_driver_spi_t spi1 = {
	.spi = NRF_DRV_SPI_INSTANCE(1)
};
static ws2812b_driver_spi_t spi2 = {
	.spi = NRF_DRV_SPI_INSTANCE(2)
};


//void * spi0_event_handler(nrf_drv_spi_evt_t const * event);
//void * spi1_event_handler(nrf_drv_spi_evt_t const * event);
//void * spi2_event_handler(nrf_drv_spi_evt_t const * event);

//void * spi0_event_handler(nrf_drv_spi_evt_t const * event)
//{
//		spi0_transfer_completed = true;
//}

/** @brief Function for main application entry.
 */
int main(void)
{
		spi_buffer_t spi0_buffer;
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

		// Initialize spi0 I/F
		ws2812b_driver_spi_init(SPI_INSTANCE, &spi0);

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
		
		alloc_spi_buffer(&spi0_buffer, NUM_LEDS);

		// spi_master_init(p_instance);
	
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
				led_array_work[i] = led_array_flash1[i];
			}
			for(uint16_t i=0;i<NUM_LEDS;i++)
			{
				if ( rand()%PRAB_FLASH == 0 )
				{
					led_array_flash1[i].green = MAX_INTENSE2;
					led_array_flash1[i].red   = MAX_INTENSE2;
					led_array_flash1[i].blue  = MAX_INTENSE2;
				}
				else if ( i + STEP_SRIDE1 >= NUM_LEDS || i + STEP_SRIDE1 < 0 ) 
				{
					led_array_flash1[i].green = 0;
					led_array_flash1[i].red   = 0;
					led_array_flash1[i].blue  = 0;
				}
				else
				{
					nextc = led_array_work[i+STEP_SRIDE1].green - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash1[i].green = nextc;
				
					nextc = led_array_work[i+STEP_SRIDE1].red - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash1[i].red = nextc;
				
						nextc = led_array_work[i+STEP_SRIDE1].blue - DECAY_STEP;
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
					led_array_work[i] = led_array_flash2[i];
			}
				for(uint16_t i=0;i<NUM_LEDS;i++)
			{
				if ( rand()%PRAB_FLASH == 0 )
				{
					led_array_flash2[i].green = MAX_INTENSE2;
					led_array_flash2[i].red   = MAX_INTENSE2;
					led_array_flash2[i].blue  = MAX_INTENSE2;
				}
				else if ( i + STEP_SRIDE2 >= NUM_LEDS || i + STEP_SRIDE2 < 0 ) 
				{
					led_array_flash2[i].green = 0;
					led_array_flash2[i].red   = 0;
					led_array_flash2[i].blue  = 0;
				}
				else
				{
					nextc = led_array_work[i+STEP_SRIDE2].green - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash2[i].green = nextc;
				
					nextc = led_array_work[i+STEP_SRIDE2].red - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash2[i].red = nextc;
				
						nextc = led_array_work[i+STEP_SRIDE2].blue - DECAY_STEP;
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
      ws2812b_driver_xfer(led_array_work, spi0_buffer, spi0);
				
			// delay (LED will be updated this period)
			nrf_delay_ms(DELAY_MS);
				
			LEDS_INVERT(1 << leds_list[0]); // toggle on-board LED
		}
}

/** @} */
