//
//  EGRE 364
//  Spring 2019
//
//  adc.h
//
//  written by Xander Will / George Constantine
//
//  'Analog->Digital Converter driver'
//

#pragma once

#include "stm32l476xx.h"
#include "gpio.h"

#define WAIT_LEN 20 * (80000000 / 10000000)

typedef struct {
	ADC_TypeDef		*ADCx;
	GPIO_Pin_Info *pin;
	int						channel;
	int						resolution;
} ADC_Channel;

void ADC_Init(ADC_TypeDef *ADCx, int resolution) {
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	ADCx->CR &= ~ADC_CR_ADEN;
	SYSCFG->CFGR1 |= SYSCFG_CFGR1_BOOSTEN;	// voltage booster
	ADC123_COMMON->CCR |= ADC_CCR_VREFEN;		// conversion of internal channels
	ADC123_COMMON->CCR &= ~(0xF << 18);			// set clock frequency to "not divided" (0000)
	ADC123_COMMON->CCR &=	~(0x3 << 16);			// set clock sync to HCLK/1 (01)
	ADC123_COMMON->CCR |= 1 << 16;		
	ADC123_COMMON->CCR &= ~0x1F;						// set ADCs as independent (00000)
	
	ADCx->CFGR &= ~(0x3 << 3);	// selecting resolution
	switch (resolution) {
		default: case 12:
			ADCx->CFGR |= 0;
			break;
		case 10:
			ADCx->CFGR |= (1 << 3);
			break;
		case 8:
			ADCx->CFGR |= (2 << 3);
			break;
		case 6:
			ADCx->CFGR |= (3 << 3);
			break;
	}
	ADCx->CFGR &= ~ADC_CFGR_ALIGN;
}
void ADC_Enable(ADC_TypeDef *ADCx) {
	int wait_time;
	
	// ADC wakeup, as written by Yifeng Zu
	if ((ADCx->CR & ADC_CR_DEEPPWD) == ADC_CR_DEEPPWD)
		ADCx->CR &= ~ADC_CR_DEEPPWD;
	ADCx->CR |= ADC_CR_ADVREGEN;	// enable internal voltage regulator
	wait_time = WAIT_LEN;
	while (wait_time != 0)
		wait_time--;
}
static int pin_to_channel(GPIO_Pin_Info *pin) {
	switch (pin->port_char) {
		case 'A': default:
			switch (pin->pin_num) {
				case 0: default:
					return 5;	// ADC12_IN 5
				case 1:
					return 6; // ADC12_IN 6
				case 2:
					return 7; // ADC12_IN 7
				case 3:
					return 8; // ADC12_IN 8
				case 4:
					return 9; // ADC12_IN 9
				case 5:
					return 10; // ADC12_IN 10
				case 6:
					return 11; // ADC12_IN 11
				case 7:
					return 12; // ADC12_IN 12
			}
			break;
		case 'B':
			switch (pin->pin_num) {
				case 0: default:
					return 15; // ADC12_IN 15
				case 1:
					return 16; // ADC12_IN 16
			}
			break;
		case 'C':
			switch (pin->pin_num) {
				case 0: default:
					return 1; // ADC123_IN 1
				case 1:
					return 2; // ADC123_IN 2
				case 2:
					return 3; // ADC123_IN 3
				case 3:
					return 4; // ADC123_IN 4
				case 4:
					return 13; // ADC12_IN 13
				case 5:
					return 14; // ADC12_IN 14
			}
			break;
		case 'F':
			switch (pin->pin_num) {
				case 3: default:
					return 6; // ADC3_IN 6
				case 4:
					return 7; // ADC3_IN 7
				case 5:
					return 8; // ADC3_IN 8
				case 6:
					return 9; // ADC3_IN 9
				case 7:
					return 10; // ADC3_IN 10
				case 8:
					return 11; // ADC3_IN 11
				case 9:
					return 12; // ADC3_IN 12
				case 10:
					return 13; // ADC3_IN 13
			}
			break;
	}
}
GPIO_Pin_Info *ADC_PinInit(char port, uint8_t pin, GPIO_PUPDR_ENUM pupd) {
	GPIO_Pin_Info *p = GPIO_PinInit(port, pin, MODER_AF, OTYPER_PP, OSPEEDR_HS, pupd);
	p->port->ASCR |= 1ul << p->pin_num;
	return p; 
}
ADC_Channel *ADC_CreateChannel(ADC_TypeDef *ADCx, GPIO_Pin_Info *pin) {
	ADC_Channel *a;
	
	a = malloc(sizeof(ADC_Channel));
	a->ADCx = ADCx;
	a->pin	= pin;
	a->channel = pin_to_channel(pin);
	ADCx->DIFSEL |= 1ul << a->channel;
	return a;
}

uint16_t ADC_GetData(ADC_Channel *channel) {
	
}