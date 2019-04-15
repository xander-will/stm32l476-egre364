//
//	EGRE 364
//  Spring 2019
//
//  exti.h
//
//  written by Xander Will / George Constantine
//
//  'External interrupt wrapper'
//

#pragma once

#include "stm32l476xx.h"
#include "stdint.h"

#include "gpio.h"

typedef void (*interrupt_func)(void);

static interrupt_func exti_callbacks[16] = {NULL}; // 

void EXTI_RegisterPin(const GPIO_Pin_Info* pin, interrupt_func callback, uint32_t priority, char edge) {
	/* registers a pin to have an external interrupt,
		 and registers a callback function for said
		 interrupt to call. use carefully: using
		 consecutive interrupts on the same pin #
		 will overwrite previous interrupt registrations */
	char a_char;
	uint32_t offset, pin_offset, IRQnum;
		
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	offset = pin->pin_num / 4ul;		// 4 pins per index in EXTICR
	pin_offset = (4ul*(pin->pin_num - 4*offset));
	SYSCFG->EXTICR[offset] &= ~(SYSCFG_EXTICR1_EXTI0 << pin_offset);
	SYSCFG->EXTICR[offset] |= (pin->port_char - 'A') << pin_offset;
	
	offset = 1ul << pin->pin_num;
	switch (edge) {								// rising/falling edge selector
		case 'r': case 'R': default:
			EXTI->RTSR1 |= offset;
			EXTI->FTSR1 &= ~offset;
			break;
		case 'f': case 'F':
			EXTI->FTSR1 |= offset;
			EXTI->RTSR1 &= ~offset;
			break;
		case 'b': case 'B':
			EXTI->RTSR1 |= offset;
			EXTI->FTSR1 |= offset;
			break;
	}
	EXTI->IMR1 |= offset;	// interrupt mask enable
	
	if (5 <= pin->pin_num && pin->pin_num <= 9)
		IRQnum = EXTI9_5_IRQn;
	else if (10 <= pin->pin_num && pin->pin_num <= 15)
		IRQnum = EXTI15_10_IRQn;
	else
		IRQnum = EXTI0_IRQn + pin->pin_num;
	exti_callbacks[pin->pin_num] = callback;
	NVIC_SetPriority(IRQnum, priority);
	NVIC_EnableIRQ(IRQnum);
}

void EXTI_ChangeCallback(const GPIO_Pin_Info *pin, interrupt_func new_cb) {
	exti_callbacks[pin->pin_num] = new_cb;
}

// these override the given IRQHandlers in startup_stm32l476xx.s
void EXTI0_IRQHandler() {
	if (exti_callbacks[0])
		exti_callbacks[0]();
	EXTI->PR1 |= EXTI_PR1_PIF0;	// clear the pending status
}
void EXTI1_IRQHandler() {
	if (exti_callbacks[1])
		exti_callbacks[1]();
	EXTI->PR1 |= EXTI_PR1_PIF1;	
}
void EXTI2_IRQHandler() {
	if (exti_callbacks[2])
		exti_callbacks[2]();
	EXTI->PR1 |= EXTI_PR1_PIF2;	
}
void EXTI3_IRQHandler() {
	if (exti_callbacks[3])
		exti_callbacks[3]();
	EXTI->PR1 |= EXTI_PR1_PIF3;	
}
void EXTI4_IRQHandler() {
	if (exti_callbacks[4])
		exti_callbacks[4]();
	EXTI->PR1 |= EXTI_PR1_PIF4;	
}
void EXTI9_5_IRQHandler() {
	uint32_t i, pr1;
	
	pr1 = EXTI->PR1;	// want the current state of PR1, not the ongoing state
	for (i = 5; i <= 9; i++) {
		if (pr1 & 1ul << i) {
			if (exti_callbacks[i]) {
				exti_callbacks[i]();
				break;
			}
		}
	}
	if (i > 9)
		EXTI->PR1 |= 0x3E0;	// clear all pending 5 - 9 interrupts
	else
		EXTI->PR1 |= 1ul << i;
}
void EXTI15_10_IRQHandler() {
	uint32_t i, pr1;
	
	pr1 = EXTI->PR1;
	for (i = 10; i <= 15; i++) {
		if (pr1 & 1ul << i) {
			if (exti_callbacks[i]) {
				exti_callbacks[i]();
				break;
			}
		}
	}
	if (i > 15)
		EXTI->PR1 |= 0xFC00;	// clear all pending 15 - 10 interrupts
	else
		EXTI->PR1 |= 1ul << i;
}