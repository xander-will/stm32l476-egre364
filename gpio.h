//
//  gpio.h
//
//  written by Xander Will
//
//  'GPIO pin wrapper'
//

#pragma once

#include <stdlib.h>

#include "stm32l476xx.h"
#include "stdint.h"
#include "general.h"

#define TWOBIT_MASK 3ul

typedef enum {
	CLOCK_ENABLE 	= 0ul,	// Clock enable
	CLOCK_DISABLE 	= 1ul	// Clock disable
} GPIO_CLOCK_ENUM;

typedef enum {
	MODER_DI 	= 0ul,		// Digital input
	MODER_DO 	= 1ul,		// Digital output
	MODER_AF	= 2ul,		// Alternative function
	MODER_AM 	= 3ul		// Analog mode
} GPIO_MODER_ENUM;

typedef enum {
	OTYPER_PP	= 0ul,	// Push-pull
	OTYPER_OD	= 1ul	// Open-drain
} GPIO_OTYPER_ENUM;

typedef enum {
	OSPEEDR_LS	= 0ul,	// Low speed
	OSPEEDR_MS	= 1ul,	// Medium speed
	OSPEEDR_FS	= 2ul,	// Fast speed
	OSPEEDR_HS	= 3ul	// High speed
} GPIO_SPEEDR_ENUM;

typedef enum {
	PUPDR_N		= 0ul,	// No pull-up, no pull-down
	PUPDR_PU	= 1ul,	// Pull-up
	PUPDR_PD	= 2ul,	// Pull-down
	PUPDR_R		= 3ul	// Reserved
} GPIO_PUPDR_ENUM;

typedef enum {
	ODR_LOW		= 0ul,
	ODR_HIGH	= 1ul
} GPIO_ODR_ENUM;

struct GPIO_Pin_Struct {
	GPIO_TypeDef* 		port;
	char				port_char;
	uint8_t 			pin_num;
	GPIO_MODER_ENUM		mode;
	GPIO_OTYPER_ENUM  	type;
	GPIO_SPEEDR_ENUM  	speed;
	GPIO_PUPDR_ENUM		pupd;
};

typedef struct GPIO_Pin_Struct *GPIO_Pin;

static uint32_t ModeMaskGenerator(uint32_t mode, uint32_t pin_mask) {
	uint32_t i, new_mode, mask = 0;
	
	new_mode = mode & 3;
	if (new_mode == 0)
		return 0;
	
	for (i = 0; i <= 15; i++)
		mask |= new_mode << (2*i);
	return mask & pin_mask;
}

GPIO_Pin GPIO_PinInit(char port, uint8_t pin, GPIO_MODER_ENUM mode, GPIO_OTYPER_ENUM type, GPIO_SPEEDR_ENUM speed, GPIO_PUPDR_ENUM pupd) {
	/* tell it what port/pin/options you want
		 and it will initialize the pin, and return
		 a GPIO_Pin_Info struct for you to use */
	char a_char;
	uint32_t clock_enable_pin, pin2;
	GPIO_TypeDef *gpio_port;
	GPIO_Pin new_pin_info;
	
	a_char = port >= 'a' ? 'a' : 'A'; 
	
	if (port < a_char || 'h' < port || 'H' < port)
		return NULL;
	gpio_port = (GPIO_TypeDef*)(AHB2PERIPH_BASE + (port - a_char)*0x0400);	// each port map is 1kB wide
	clock_enable_pin = 1ul << (port - a_char);
				
	if (pin < 0 || 15 < pin)
		return NULL;
	if (mode < MODER_DI || MODER_AM < mode)
		return NULL;
	if (type < OTYPER_PP || OTYPER_OD < type)
		return NULL;
	if (speed < OSPEEDR_LS || OSPEEDR_HS < speed)
		return NULL;
	if (pupd < PUPDR_N || PUPDR_R < pupd)
		return NULL;
	
	pin2 = pin * 2;		// used for registers with two bits per pin
	RCC->AHB2ENR |= clock_enable_pin;		// turn on peripheral clock
	gpio_port->MODER &= ~(TWOBIT_MASK << pin2);
	gpio_port->MODER |= mode << pin2;
	if (type == OTYPER_PP)
		gpio_port->OTYPER &= ~(1 << pin);
	else	// must be open-drain
		gpio_port->OTYPER |= 1 << pin;
	gpio_port->OSPEEDR &= ~(TWOBIT_MASK << pin2);
	gpio_port->OSPEEDR |= speed << pin2;
	gpio_port->PUPDR &= ~(TWOBIT_MASK << pin2);
	gpio_port->PUPDR |= pupd << pin2;
	
	new_pin_info = malloc(sizeof(struct GPIO_Pin_Struct));
	new_pin_info->port = gpio_port;
	new_pin_info->port_char = port >= 'a' ? port - 0x20 : port;
	new_pin_info->pin_num = pin;
	new_pin_info->mode = mode;
	new_pin_info->type = type;
	new_pin_info->speed = speed;
	new_pin_info->pupd = pupd;
		
	return new_pin_info;
}

GPIO_Pin GPIO_PinModify(GPIO_Pin pin_info, GPIO_MODER_ENUM mode, GPIO_OTYPER_ENUM type, GPIO_SPEEDR_ENUM speed, GPIO_PUPDR_ENUM pupd) {
	uint8_t pin, pin2;
	GPIO_TypeDef *gpio_port;
	
	pin = pin_info->pin_num;
	gpio_port = pin_info->port;
	
	if (mode < MODER_DI || MODER_AM < mode)
		return NULL;
	if (type < OTYPER_PP || OTYPER_OD < type)
		return NULL;
	if (speed < OSPEEDR_LS || OSPEEDR_HS < speed)
		return NULL;
	if (pupd < PUPDR_N || PUPDR_R < pupd)
		return NULL;
	
	pin2 = pin * 2;
	gpio_port->MODER &= ~(TWOBIT_MASK << pin2);
	gpio_port->MODER |= mode << pin2;
	if (type == OTYPER_PP)
		gpio_port->OTYPER &= ~(1 << pin);
	else	// must be open-drain
		gpio_port->OTYPER |= 1 << pin;
	gpio_port->OSPEEDR &= ~(TWOBIT_MASK << pin2);
	gpio_port->OSPEEDR |= speed << pin2;
	gpio_port->PUPDR &= ~(TWOBIT_MASK << pin2);
	gpio_port->PUPDR |= pupd << pin2;
	
	pin_info->mode = mode;
	pin_info->type = type;
	pin_info->speed = speed;
	pin_info->pupd = pupd;

	return pin_info;
}

GPIO_Pin *GPIO_PinBatchInit(char port, uint8_t first_pin, uint8_t last_pin, GPIO_MODER_ENUM mode, GPIO_OTYPER_ENUM type, GPIO_SPEEDR_ENUM speed, GPIO_PUPDR_ENUM pupd) {
	/* can initialize a row of pins on the
		 same port all at once, and returns
		 an array of Pin_Info pointers from
		 littlest to biggest pin */
	char a_char;
	uint32_t i, clock_enable_pin, num_of_pins, mask, mask2;
	GPIO_TypeDef *gpio_port;
	GPIO_Pin *new_pin_array;
	
	a_char = port >= 'a' ? 'a' : 'A';
	num_of_pins = last_pin - first_pin + 1;
	
	if (port < a_char || 'h' < port || 'H' < port)
		return NULL;
	gpio_port = (GPIO_TypeDef*)(AHB2PERIPH_BASE + (port - a_char)*0x0400);	// each port map is 1kB wide
	clock_enable_pin = 1ul << (port - a_char);
	
	if (first_pin > last_pin)
		return NULL;
	if (first_pin < 0 || 15 < last_pin)
		return NULL;
	if (mode < MODER_DI || MODER_AM < mode)
		return NULL;
	if (type < OTYPER_PP || OTYPER_OD < type)
		return NULL;
	if (speed < OSPEEDR_LS || OSPEEDR_HS < speed)
		return NULL;
	if (pupd < PUPDR_N || PUPDR_R < pupd)
		return NULL;
	
	mask = ((1 << (last_pin + 1)) - 1) - ((1 << first_pin) - 1); // trust me, it works
	mask2 = ((1 << (last_pin*2+2)) - 1) - ((1 << (first_pin*2)) - 1); // TRUST ME, IT WORKS
	RCC->AHB2ENR |= clock_enable_pin;		// turn on peripheral clock
	gpio_port->MODER &= ~mask2;
	gpio_port->MODER |= ModeMaskGenerator(mode, mask2);
	if (type == OTYPER_PP)
		gpio_port->OTYPER &= ~mask;
	else	// must be open-drain
		gpio_port->OTYPER |= mask;
	gpio_port->OSPEEDR &= ~mask2;
	gpio_port->OSPEEDR |= ModeMaskGenerator(speed, mask2);
	gpio_port->PUPDR &= ~mask2;
	gpio_port->PUPDR |= ModeMaskGenerator(pupd, mask2);
	
	new_pin_array = malloc(sizeof(struct GPIO_Pin_Struct)*num_of_pins);
	for (i = 0; i < num_of_pins; i++) {
		new_pin_array[i] = malloc(sizeof(GPIO_Pin_Info));
		new_pin_array[i]->port = gpio_port;
		new_pin_array[i]->port_char = port >= 'a' ? port - 0x20 : port;
		new_pin_array[i]->pin_num = i + first_pin;
		new_pin_array[i]->mode = mode;
		new_pin_array[i]->type = type;
		new_pin_array[i]->speed = speed;
		new_pin_array[i]->pupd = pupd;
	}
		
	return new_pin_array;
}

void GPIO_PinOn(const GPIO_Pin pin) {
	/* turns on whatever pin is put in */
	pin->port->ODR |= (1ul << pin->pin_num);
}

void GPIO_PinOff(const GPIO_Pin pin) {
	/* turns off whatever pin is put in */
	pin->port->ODR &= ~(1ul << pin->pin_num);
}

bool GPIO_PinCheck(const GPIO_Pin pin) {
	/* returns a boolean result for an input pin */
	uint32_t mask = (1ul << pin->pin_num);
	return (pin->port->IDR & mask) == mask;
}
