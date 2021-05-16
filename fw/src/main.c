/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#include <stm32f1xx.h>

#include "printf.h"

#include "flash.h"
#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "usb.h"

#include "util/data.h"
#include "util/types.h"

#include "tusb.h"

/* Variables */
static const struct gpio_pin_t power_glitch_io = {.port = GPIO_PORT_A, .pin = 0};
static const struct gpio_pin_t power_io = {.port = GPIO_PORT_A, .pin = 2};

static const struct gpio_pin_t user_led_io = {.port = GPIO_PORT_C, .pin = 13};

static volatile u32 glitch_width = 4; /* default to 500ns (125ns steps) */
static volatile u32 time_to_glitch = 24000; /* default to 63ms (125ns steps) */
static volatile u32 reset_width = 800000; /* default to 100ms (125ns steps) */

/* Terminal strings */
#define STR_(X) 			#X
#define STR(X) 				STR_(X)
#define HEADER_STR			"%s v:%s\n\r"
#define COMMAND_PROMPT_STR	"command (h for help): "
#define GLITCH_WIDTH_STR	"\n\rsetting glitch pulse width to %0.3fus\n\r"
#define RESET_WIDTH_STR		"\n\rsetting rest pulse width to %0.3fus\n\r"
#define TIME_TO_GLITCH_STR	"\n\rsetting time to glitch to %0.3fus\n\r"
#define RESET_STR			"Reseting target\n\r"
#define GLITCH_STR			"glitching target\n\r"
#define COMMAND_LIST		"\n\r\
generic\n\r\
 h    print this\n\r\
 r    reset target\n\r\
 g    glitch target(includes reset)\n\r\
\n\r\
config\n\r\
 w    glitch pulse width (format: command%%d, 125ns steps, default 4/500ns)\n\r\
 l    reset pulse width (format: command%%d, 125ns steps, default 800000/100ms)\n\r\
 d    time between reset and glitch (format: command%%d, 125ns steps, default 24000/3ms)\n\r\
\n\r"
#define UNEXPECTED_STR			"Something unexpected happened.\n\r"

/* Funcion prototypes */
void cdc_task();
s32 get_user_value();
void reset_target();
void glitch_target();
void config_glitch_width(u32 width);
void config_reset_width(u32 width);
void config_time_to_glitch(u32 time);

/* main flow */
void main()
{
	flash_latency_config(72000000);

	rcc_init(EXTERNAL_CLOCK_VALUE);

	systick_init();

	struct gpio_config_t gpio_config;
	gpio_init_config(&gpio_config);

	/* USB io */
	struct gpio_pin_t usb_dm_io = {.port = GPIO_PORT_A, .pin = 11};
	struct gpio_pin_t usb_dp_io = {.port = GPIO_PORT_A, .pin = 12};
	struct gpio_pin_t usb_dp_pu_io = { .port = GPIO_PORT_A, .pin = 12 }; // bluepill

	gpio_setup_pin(&gpio_config, usb_dm_io, GPIO_MODE_INPUT | GPIO_CNF_INPUT_FLOATING, 0); /* USB DM */
	gpio_setup_pin(&gpio_config, usb_dp_io, GPIO_MODE_INPUT | GPIO_CNF_INPUT_FLOATING, 0); /* USB DP */
	gpio_setup_pin(&gpio_config, usb_dp_pu_io, GPIO_MODE_OUTPUT_50MHZ | GPIO_CNF_OUTPUT_GENERAL_PUSH_PULL, 0); /* USB DP PU */

	/* output io */
	gpio_setup_pin(&gpio_config, power_glitch_io, GPIO_MODE_OUTPUT_50MHZ | GPIO_CNF_OUTPUT_GENERAL_PUSH_PULL, 0);
	gpio_setup_pin(&gpio_config, power_io, GPIO_MODE_OUTPUT_50MHZ | GPIO_CNF_OUTPUT_GENERAL_PUSH_PULL, 1);

	gpio_setup_pin(&gpio_config, user_led_io, GPIO_MODE_OUTPUT_10MHZ | GPIO_CNF_OUTPUT_GENERAL_PUSH_PULL, 0);

	gpio_apply_config(gpio_config);

	usb_init();

	u64 led_timestamp = systick_get_ticks() + 500;

	for (;;) {
		tud_task();
		cdc_task();

		if(systick_get_ticks() > led_timestamp)
		{
			gpio_toggle(user_led_io);

			if(tud_cdc_connected())
				led_timestamp = systick_get_ticks() + 200;
			else
				led_timestamp = systick_get_ticks() + 500;
		}
	}
}

void cdc_task(void)
{
  /* connected() check for DTR bit */
  /* Most but not all terminal client set this when making connection */

	static u8 connected;

	if(tud_cdc_connected())
	{
		if(!connected)
		{
			printf(HEADER_STR, PROJECT_NAME, PROJECT_VERSION);
			printf(COMMAND_PROMPT_STR);

			connected = 1;
		}

		/* connected and there is data available */
		if (tud_cdc_available())
		{
			s32 arg = 0;

			u8 cmd = (u8)tud_cdc_read_char();

			printf("%c\n\r", cmd);

			switch(cmd)
			{
				case 'h':
					printf(HEADER_STR, PROJECT_NAME, PROJECT_VERSION);
					printf(COMMAND_LIST);
					break;
				case 'r':
					printf(RESET_STR);
					reset_target();
					break;
				case 'g':
					printf(GLITCH_STR);
					glitch_target();
					break;
				case 'w':
					arg = get_user_value();
					if(arg < 0)
					{
						printf(UNEXPECTED_STR);
					}
					else
					{
						printf(GLITCH_WIDTH_STR, (float)arg * 0.125f);
						config_glitch_width(arg);
					}
					break;
				case 'l':
					arg = get_user_value();
					if(arg < 0)
					{
						printf(UNEXPECTED_STR);
					}
					else
					{
						printf(RESET_WIDTH_STR, (float)arg * 0.125f);
						config_reset_width(arg);
					}
					break;
				case 'd':
					arg = get_user_value();
					if(arg < 0)
					{
						printf(UNEXPECTED_STR);
					}
					else
					{
						printf(TIME_TO_GLITCH_STR, (float)arg * 0.125f);
						config_time_to_glitch(arg);
					}
					break;
				default:
					printf(UNEXPECTED_STR);
					break;
			}

			printf(COMMAND_PROMPT_STR);
		}

		tud_cdc_write_flush();
	}
	else
	{
		connected = 0;
	}
}

s32 get_user_value()
{
	u64 endticks = systick_get_ticks() + 5000;

	s32 value = 0;

	while(1)
	{
		if(systick_get_ticks() > endticks)
			return -1;

		tud_cdc_write_flush();
		tud_task();

		if(tud_cdc_available())
		{
			u8 number = (u8)tud_cdc_read_char();

			printf("%c", number);

			if(number > 0b00101111 && number < 0b00111010) /* number chars range from 0b00110000 to 0b00111001*/
			{
				number &= 0x0F;
				value = (value * 10) + number;
			}
			else
				return value;
		}

	}
}

void reset_target()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		gpio_set(power_io, 0);
		gpio_set(power_glitch_io, 1); /* to help discharge vcc */
		delay_125ns(reset_width);
		gpio_set(power_glitch_io, 0);
		gpio_set(power_io, 1);
	}
}
void glitch_target()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		reset_target();

		delay_125ns(time_to_glitch);

		gpio_set(power_glitch_io, 1);
		//delay_125ns(glitch_width);	//too much overhead, this gives roughly 600ns pulses (I was able to get down to 200ns)
		gpio_set(power_glitch_io, 0);
	}
}
void config_glitch_width(u32 width)
{
	glitch_width = width;
}
void config_reset_width(u32 width)
{
	reset_width = width;
}
void config_time_to_glitch(u32 time)
{
	time_to_glitch = time;
}