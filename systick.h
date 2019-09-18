//
//  systick.h
//
//  written by Xander Will
//
//  'SysTick initialization + task scheduling'
//

#pragma once

#include <stdlib.h>
#include "stm32l476xx.h"

typedef void (*systick_func)(void*);
typedef int SysTickCallback;

void SysTick_Init(int load_t, int priority){
	
	SysTick->CTRL = 0;				
	SysTick->LOAD = load_t - 1;  
	SysTick->VAL = 0;

	NVIC_SetPriority(SysTick_IRQn, priority);		
	NVIC_EnableIRQ(SysTick_IRQn);					

	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;		// external clock used
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  
}

#define NUM_OF_CALLBACKS 30
static systick_func systick_callbacks[NUM_OF_CALLBACKS] = {NULL};
static int systick_callback_time[NUM_OF_CALLBACKS];
static int systick_time_since_last[NUM_OF_CALLBACKS];
static void *systick_arguments[NUM_OF_CALLBACKS];
SysTickCallback SysTick_RegisterCallback(systick_func callback, int call_t, void *arg) {
	int i;
	
	for (i = 0; i < NUM_OF_CALLBACKS; i++) {
		if (systick_callbacks[i] == NULL) {
			systick_callbacks[i] = callback;
			systick_callback_time[i] = call_t;
			systick_time_since_last[i] = 0;
			systick_arguments[i] = arg;
			return i;
		}
	}
	return -1;
}
void SysTick_RemoveCallback(SysTickCallback callback) {
	systick_callbacks[callback] = NULL;
}
void SysTick_ChangeCallbackTime(SysTickCallback callback, int call_t) {
	if (call_t >= 0)
		systick_callback_time[callback] = call_t;
}
void SysTick_ChangeArg(SysTickCallback callback, void *arg) {
	systick_arguments[callback] = arg;
}

static int msTicks = 0;
void SysTick_Handler(void){
	int i;
	
	msTicks++;
	for (i = 0; i < NUM_OF_CALLBACKS; i++) {
		if (systick_callbacks[i] == NULL)
			continue;
		systick_time_since_last[i]++;
		if (systick_time_since_last[i] >= systick_callback_time[i]) {
			systick_callbacks[i](systick_arguments[i]);
			systick_time_since_last[i] = 0;
		}
	}	
}
void delay(uint32_t T){
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < T);
	
	msTicks = 0;
}