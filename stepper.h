//
//	EGRE 364
//  Spring 2019
//
//  stepper.h
//
//  written by Xander Will / George Constantine
//
//  'Stepper motor functions'
//

#pragma once

#include "stm32l476xx.h"

#include "gpio.h"
#include "systick.h"

typedef struct {
	SysTickCallback 	callback;
	GPIO_Pin_Info*		pinA;
	GPIO_Pin_Info*		pinB;
	GPIO_Pin_Info*		pinC;
	GPIO_Pin_Info*		pinD;
	int								delay;
	char							mode;
	int								curr_step;
	int								direction;
} Stepper_Info;

GPIO_Pin_Info *Stepper_PinInit(char port, uint8_t pin) {
	return GPIO_PinInit(port, pin, MODER_DO, OTYPER_PP, OSPEEDR_HS, PUPDR_N); 
}
GPIO_Pin_Info **Stepper_PinBatchInit(char port, uint8_t first_pin, uint8_t last_pin) {
	return GPIO_PinBatchInit(port, first_pin, last_pin, MODER_DO, OTYPER_PP, OSPEEDR_HS, PUPDR_N);
}

void Stepper_FullStep(void*);
void Stepper_HalfStep(void*);
Stepper_Info *Stepper_Init(GPIO_Pin_Info* pinA, 
													 GPIO_Pin_Info* pinB, 
													 GPIO_Pin_Info* pinC, 
													 GPIO_Pin_Info* pinD, 
													 int delay, char mode) {											 
	Stepper_Info *s; 
	systick_func f = NULL;
														 
  s = malloc(sizeof(Stepper_Info));
	switch (mode) {
		default: case 'f': case 'F':
			f = Stepper_FullStep; break;
		case 'h': case 'H':
			f = Stepper_HalfStep; break;	
		case 'm': case 'M':
			break;	// todo: add microstepping support
	}
	if ((s->callback = SysTick_RegisterCallback(f, delay, s)) == -1) {
		free(s);
		return NULL;
	}
														 
	s->pinA = pinA;
	s->pinB = pinB;
	s->pinC = pinC;
	s->pinD = pinD;
	s->mode = mode;
	s->delay = delay;
	s->curr_step = 0;
	s->direction = 1;
	
	return s;
}
void Stepper_Reverse(Stepper_Info *s) {
	s->direction = -s->direction;
}	
void Stepper_ChangeSpeed(Stepper_Info *s, int delay) {
	SysTick_ChangeCallbackTime(s->callback, delay);
	s->delay = delay;
}
													 
static uint8_t stepper_fullsteps[4] = {3, 6, 12, 9};
void Stepper_FullStep(void *p) {
	uint8_t step;
	Stepper_Info *s;
	
	s = (Stepper_Info*)p; 
	s->curr_step += s->direction;
	if (s->curr_step > 3)
		s->curr_step = 0;
	else if (s->curr_step < 0)
		s->curr_step = 3;
	
	step = stepper_fullsteps[s->curr_step];
	step & 1 ? GPIO_PinOn(s->pinA) : GPIO_PinOff(s->pinA);
	step & 2 ? GPIO_PinOn(s->pinB) : GPIO_PinOff(s->pinB);
	step & 4 ? GPIO_PinOn(s->pinC) : GPIO_PinOff(s->pinC);
	step & 8 ? GPIO_PinOn(s->pinD) : GPIO_PinOff(s->pinD);
}	

static uint8_t stepper_halfsteps[8] = {1, 3, 2, 6, 4, 12, 8, 9};
void Stepper_HalfStep(void *p) {
	uint8_t step;
	Stepper_Info *s;
	
	s = (Stepper_Info*)p; 
	s->curr_step += s->direction;
	if (s->curr_step > 7)
		s->curr_step = 0;
	else if (s->curr_step < 0)
		s->curr_step = 7;
	
	step = stepper_halfsteps[s->curr_step];
	step & 1 ? GPIO_PinOn(s->pinA) : GPIO_PinOff(s->pinA);
	step & 2 ? GPIO_PinOn(s->pinB) : GPIO_PinOff(s->pinB);
	step & 4 ? GPIO_PinOn(s->pinC) : GPIO_PinOff(s->pinC);
	step & 8 ? GPIO_PinOn(s->pinD) : GPIO_PinOff(s->pinD);
}	