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
#include "bsp.h"
#include "app_timer.h"
#include "nordic_common.h"
#include  "stdlib.h"

#include "ws2812b_driver.h"

const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;

//#define APP_TIMER_PRESCALER      0                      ///< Value of the RTC1 PRESCALER register.
//#define APP_TIMER_MAX_TIMERS     BSP_APP_TIMERS_NUMBER  ///< Maximum number of simultaneously created timers.
//#define APP_TIMER_OP_QUEUE_SIZE  2                      ///< Size of timer operation queues.

#define DELAY_MS                 (1)                   ///< Timer Delay in milli-seconds.
#define NUM_LEDS									(240)
#define MAX_INTENSE								(16)
#define MAX_INTENSE2							(255)
#define MIN_INTENSE								(1)
#define DECAY_STEP								(5)
#define	PRAB_FLASH								(10000)
#define STEP_SRIDE								(20)

static volatile bool m_transfer_completed = true;

static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);

/**@brief Handler for SPI0 master events.
 *
 * @param[in] event SPI master event.
 */
nrf_drv_spi_handler_t spi_master_0_event_handler(nrf_drv_spi_evt_t * event)
{
		m_transfer_completed = true;
}


/**@brief Function for initializing a SPI master driver.
 *
 * @param[in] p_instance    Pointer to SPI master driver instance.
 * @param[in] lsb           Bits order LSB if true, MSB if false.
 */
static void spi_master_init(nrf_drv_spi_t const * p_instance, bool lsb)
{
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_spi_config_t config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = APP_IRQ_PRIORITY_HIGH,
        .orc          = 0xff,
        .frequency    = NRF_DRV_SPI_FREQ_4M,
        .mode         = NRF_DRV_SPI_MODE_3,
        .bit_order    = (lsb ?
            NRF_DRV_SPI_BIT_ORDER_LSB_FIRST : NRF_DRV_SPI_BIT_ORDER_MSB_FIRST),
    };

    config.sck_pin  = SPIM0_SCK_PIN;
    config.mosi_pin = SPIM0_MOSI_PIN;
    config.miso_pin = NULL;
    err_code = nrf_drv_spi_init(p_instance, &config, spi_master_0_event_handler);

}




/** @brief Function for main application entry.
 */
int main(void)
{

		spi_buffer_t spi_buffer_blank;
		spi_buffer_t spi_buffer_color;
		rgb_led_t led_array[NUM_LEDS];
		rgb_led_t led_array_flash[NUM_LEDS];
		rgb_led_t led_array_work[NUM_LEDS];
		uint16_t rest;
		int16_t nextc;
		
    // Configure LED-pins as outputs.
    LEDS_CONFIGURE(LEDS_MASK);


    nrf_drv_spi_t const * p_instance;

    p_instance = &m_spi_master_0;
	
		alloc_spi_buffer(&spi_buffer_blank, NUM_LEDS);
		set_blank(spi_buffer_blank);
		form_spi_sector(spi_buffer_blank);
	
		for(uint16_t i=0;i<NUM_LEDS;i++) {
			int c = (i % 7) + 1;
			if ( c&4 )
			{
				led_array[i].green = 32;
			}
			else
			{
				led_array[i].green = 0;
			}
				
			if ( c&2 )
			{
				led_array[i].red = 32;
			}
			else
			{
				led_array[i].red = 0;
			}
			if ( c&1 )
			{
				led_array[i].blue = 32;
			}
			else
			{
				led_array[i].blue = 0;
			}
		}

		for(uint16_t i=0;i<NUM_LEDS;i++) {
				led_array_flash[i].green = 0;
				led_array_flash[i].red   = 0;
				led_array_flash[i].blue  = 0;
		}
		alloc_spi_buffer(&spi_buffer_color, NUM_LEDS);
 		set_buff(led_array, spi_buffer_color);
		form_spi_sector(spi_buffer_color);

	
		spi_master_init(p_instance, false);
		nrf_spim_shorts_enable(p_instance->p_registers,true);	
	
	  nrf_drv_spi_xfer_desc_t xfer_desc0;


		xfer_desc0.p_rx_buffer = NULL;
		xfer_desc0.rx_length   = 0;

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

			for(uint16_t i=0;i<NUM_LEDS;i++) {
				if ( rand()%PRAB_FLASH == 0 )
				{
					led_array_flash[i].green = MAX_INTENSE2;
					led_array_flash[i].red   = MAX_INTENSE2;
					led_array_flash[i].blue  = MAX_INTENSE2;
				}
				else if ( i + STEP_SRIDE >= NUM_LEDS ) 
				{
					led_array_flash[i].green = 0;
					led_array_flash[i].red   = 0;
					led_array_flash[i].blue  = 0;
				}
				else
				{
					nextc = led_array_flash[i+STEP_SRIDE].green - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash[i].green = nextc;
				
					nextc = led_array_flash[i+STEP_SRIDE].red - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash[i].red = nextc;
				
						nextc = led_array_flash[i+STEP_SRIDE].blue - DECAY_STEP;
					if ( nextc < 0 )
					{
						nextc = 0;
					}
					led_array_flash[i].blue = nextc;
				}
			}
			
				for(uint16_t i=0;i<NUM_LEDS;i++) {
				nextc = led_array[i].green + led_array_flash[i].green;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE2 )
				{
					nextc = MAX_INTENSE2;
				}
				led_array_work[i].green = nextc;
				
				nextc = led_array[i].red + led_array_flash[i].red;
				if ( nextc < MIN_INTENSE )
				{
					nextc = MIN_INTENSE;
				}
				
				if ( nextc > MAX_INTENSE2 )
				{
					nextc = MAX_INTENSE2;
				}
				led_array_work[i].red = nextc;

				nextc = led_array[i].blue + led_array_flash[i].blue;
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

			set_buff(led_array_work, spi_buffer_color);
			form_spi_sector(spi_buffer_color);
				
			xfer_desc0.p_tx_buffer = spi_buffer_color.buff;
			xfer_desc0.tx_length    = spi_buffer_color.sector_size;
			rest = spi_buffer_color.length;

			while(rest > spi_buffer_color.sector_size)
			{
				m_transfer_completed = false;
				nrf_drv_spi_xfer(p_instance, &xfer_desc0, 0);
				while (! m_transfer_completed) {}

				xfer_desc0.p_tx_buffer += spi_buffer_color.sector_size;
				rest -= spi_buffer_color.sector_size;
			}
			xfer_desc0.tx_length    = rest;
			m_transfer_completed = false;
  		nrf_drv_spi_xfer(p_instance, &xfer_desc0, 0);
 		  while (! m_transfer_completed) {}
			
			
      nrf_delay_ms(DELAY_MS);
			LEDS_INVERT(1 << leds_list[0]);
    }
}

/** @} */
