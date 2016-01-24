/* Copyright (c) 2015 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#ifndef RUNNING_RAINBOW_H__
#define RUNNING_RAINBOW_H__

#include	"project.h"
#include "ws2812b_driver.h"
#include  "stdlib.h"

void running_rainbow_init();
void running_rainbow(rgb_led_t * led_array);

#endif // RUNNING_RAINBOW_H__

	