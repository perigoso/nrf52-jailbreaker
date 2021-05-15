/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#include <stm32f1xx.h>

#include "gpio.h"
#include "usb.h"
#include "util/types.h"

#include "tusb.h"

void usb_init()
{
	/* Enable USB peripheral clock */
	RCC->APB1RSTR |= RCC_APB1RSTR_USBRST; /* Reset peripheral clock */
	RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;
	RCC->APB1ENR |= RCC_APB1ENR_USBEN; /* Enable USB peripheral clock */

	/* Init USB stack */
	tusb_init(); /* USB Stack handles the rest of the peripheral init */
}

/* USB ISR mapping */

void _usb_hp_can_tx_isr()
{
	tud_int_handler(0);
}

void _usb_lp_can_rx0_isr()
{
	tud_int_handler(0);
}

void _usb_wakeup_isr()
{
	tud_int_handler(0);
}

/* TinyUSB port* */

void dcd_connect(u8 rhport)
{
	(void) rhport;

	struct gpio_pin_t usb_dp_pu_io = { .port = GPIO_PORT_A, .pin = 12 }; // bluepill

	gpio_set(usb_dp_pu_io, 1);
}

void dcd_disconnect(u8 rhport)
{
	(void) rhport;

	struct gpio_pin_t usb_dp_pu_io = { .port = GPIO_PORT_A, .pin = 12 }; // bluepill

	gpio_set(usb_dp_pu_io, 1);
}

/* TinyUSB Callbacks */

/* Invoked when device is mounted */
void tud_mount_cb(void)
{
}

/* Invoked when device is unmounted */
void tud_umount_cb(void)
{
}

/* Invoked when usb bus is suspended */
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void) remote_wakeup_en;
}

/* Invoked when usb bus is resumed */
void tud_resume_cb(void)
{
}
