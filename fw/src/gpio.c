/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2020 Rafael Silva <perigoso@riseup.net>
 */

#include <stm32f1xx.h>

#include "gpio.h"
#include "util/data.h"

/*
 * resolves to the base address of the GPIO peripheral
 * 0x00000400 is the size/offset of each GPIO peripheral in memory
 */
#define _GPIO(port) ((GPIO_TypeDef *) (GPIOA_BASE + ((port & 0b11) * 0x00000400UL)))

/* initiates all pins to a safe known state, inputs pull down */
void gpio_init_config(struct gpio_config_t *config)
{
	for (u8 port = 0; port < 4; port++) {
		config->port[port].ODR = 0;
		for (u8 pin = 0; pin < 16; pin++) {
			config->port[port].MODE_CNF[pin] = GPIO_MODE_INPUT | GPIO_CNF_INPUT_PULL;
		}
	}
}

void gpio_apply_config(struct gpio_config_t config)
{
	/* reset port A */
	RCC->APB2RSTR |= RCC_APB2RSTR_IOPARST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_IOPARST;
	/* enable port A peripheral clock */
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	/* configure port A */
	GPIOA->CRL = 0;
	GPIOA->CRH = 0;
	for (size_t i = 0; i < 8; i++) GPIOA->CRL |= config.port[GPIO_PORT_A].MODE_CNF[i] << (i * 4);
	for (size_t i = 0; i < 8; i++) GPIOA->CRH |= config.port[GPIO_PORT_A].MODE_CNF[8 + i] << (i * 4);
	GPIOA->ODR = config.port[GPIO_PORT_A].ODR & 0xFFFF;

	/* reset port B */
	RCC->APB2RSTR |= RCC_APB2RSTR_IOPBRST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_IOPBRST;
	/* enable port B peripheral clock */
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Enable GPIOB peripheral clock
	/* configure port B */
	GPIOB->CRL = 0;
	GPIOB->CRH = 0;
	for (size_t i = 0; i < 8; i++) GPIOB->CRL |= config.port[GPIO_PORT_B].MODE_CNF[i] << (i * 4);
	for (size_t i = 0; i < 8; i++) GPIOB->CRH |= config.port[GPIO_PORT_B].MODE_CNF[8 + i] << (i * 4);
	GPIOB->ODR = config.port[GPIO_PORT_B].ODR & 0xFFFF;

	/* reset port C */
	RCC->APB2RSTR |= RCC_APB2RSTR_IOPCRST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_IOPCRST;
	/* enable port C peripheral clock */
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; // Enable GPIOC peripheral clock
	/* configure port C */
	GPIOC->CRL = 0;
	GPIOC->CRH = 0;
	for (size_t i = 0; i < 8; i++) GPIOC->CRL |= config.port[GPIO_PORT_C].MODE_CNF[i] << (i * 4);
	for (size_t i = 0; i < 8; i++) GPIOC->CRH |= config.port[GPIO_PORT_C].MODE_CNF[8 + i] << (i * 4);
	GPIOC->ODR = config.port[GPIO_PORT_C].ODR & 0xFFFF;

	/* reset port D */
	RCC->APB2RSTR |= RCC_APB2RSTR_IOPDRST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_IOPDRST;
	/* enable port D peripheral clock */
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; // Enable GPIOD peripheral clock
	/* configure port D */
	GPIOD->CRL = 0;
	GPIOD->CRH = 0;
	for (size_t i = 0; i < 8; i++) GPIOD->CRL |= config.port[GPIO_PORT_D].MODE_CNF[i] << (i * 4);
	for (size_t i = 0; i < 8; i++) GPIOD->CRH |= config.port[GPIO_PORT_D].MODE_CNF[8 + i] << (i * 4);
	GPIOD->ODR = config.port[GPIO_PORT_D].ODR & 0xFFFF;

	/* AFIO */
	RCC->APB2RSTR |= RCC_APB2RSTR_AFIORST; // Reset peripheral
	RCC->APB2RSTR &= ~RCC_APB2RSTR_AFIORST;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Enable AFIO peripheral clock

	/* Pin Remapping */
	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
}

void gpio_setup_pin(struct gpio_config_t *config, struct gpio_pin_t pin, u8 mode_cnf, u8 out)
{
	config->port[pin.port].MODE_CNF[pin.pin] = (mode_cnf & 0xF);
	config->port[pin.port].ODR = (config->port[pin.port].ODR & ~BIT(pin.pin)) | (!!out) << pin.pin;
}

void gpio_set(struct gpio_pin_t pin, u8 out)
{
	_GPIO(pin.port)->ODR = (_GPIO(pin.port)->ODR & ~BIT(pin.pin)) | (!!out << pin.pin);
}

void gpio_toggle(struct gpio_pin_t pin)
{
	_GPIO(pin.port)->ODR ^= BIT(pin.pin);
}

u8 gpio_get(struct gpio_pin_t pin)
{
	return !!(_GPIO(pin.port)->IDR & BIT(pin.pin));
}
