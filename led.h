//
//  EGRE 364
//  Spring 2019
//
//  led.h
//
//  written by Xander Will / George Constantine
//
//  'LED wrapper for Discovery Board'
//

#pragma once

#include "stm32l476xx.h"

#include "gpio.h"

GPIO_Pin_Info *RedLEDInit(GPIO_SPEEDR_ENUM speed) {
	return GPIO_PinInit('B', 2, MODER_DO, OTYPER_PP, speed, PUPDR_N);
}
GPIO_Pin_Info *GreenLEDInit(GPIO_SPEEDR_ENUM speed) {
	return GPIO_PinInit('E', 8, MODER_DO, OTYPER_PP, speed, PUPDR_N);
}
void RedLEDOn() {
	GPIOB->ODR |= GPIO_ODR_ODR_2;
}
void RedLEDOff() {
	GPIOB->ODR &= ~GPIO_ODR_ODR_2;
}
void GreenLEDOn() {
	GPIOE->ODR |= GPIO_ODR_ODR_8;
}
void GreenLEDOff() {
	GPIOE->ODR &= ~GPIO_ODR_ODR_8;
}
