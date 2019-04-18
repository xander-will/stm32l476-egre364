//
//  EGRE 364
//  Spring 2019
//
//  usart.h
//
//  written by Xander Will / George Constantine
//
//  'USART communication wrapping'
//

/* 
   this isn't finished :/
   wanna eventually get it
	 so that it works like
	 the python serial module.
	 DMA-enabled circular buffer
	 with readline() and read()
	 support, write() should 
	 work pretty much like it 
	 already works here
*/

#pragma once

#include "stm32l476xx.h"
#include "stdint.h"

#include "gpio.h"

typedef enum {
	BITS_7 = 0x10000000ul,
	BITS_8 = 0ul,
	BITS_9 = 0x1000ul
} USART_WORDLEN_ENUM;

typedef enum {
	NOT_OVER8 = 0ul, 
	OVER8 		= 0x8000ul
} USART_OVER8_ENUM;

typedef enum {
	STOP_1 		= 0ul,
	STOP_0_5	= 0x1000ul,
	STOP_2		= 0x2000ul,
	STOP_1_5  = 0x3000ul
} USART_STOPBIT_ENUM;

typedef enum {
	PARITY_OFF	= 0ul,
	PARITY_ON		= 0x400ul
} USART_PARITYEN_ENUM;

typedef enum {
	PARITY_EVEN	= 0ul,
	PARITY_ODD	= 0x200ul
} USART_PARITY_ENUM;

void USART_Init(GPIO_Pin_Info *tx_pin, USART_TypeDef *USARTx, uint32_t baud_rate, USART_WORDLEN_ENUM wordlen, USART_STOPBIT_ENUM stopbit, USART_PARITYEN_ENUM par_en, USART_PARITY_ENUM parity, USART_OVER8_ENUM over8) {
	uint32_t af, usartdiv; 
	uint8_t low_nyb;
	
	switch ((uint32_t)USARTx) {
		case (uint32_t)USART1:
			RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
			RCC->CCIPR &= ~RCC_CCIPR_USART1SEL;
			RCC->CCIPR |= RCC_CCIPR_USART1SEL_0;
			af = 0x77;	break;
		case (uint32_t)USART2:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
			RCC->CCIPR &= ~RCC_CCIPR_USART2SEL;
			RCC->CCIPR |= RCC_CCIPR_USART2SEL_0;
			af = 0x77;	break;
		case (uint32_t)USART3:
			RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
			RCC->CCIPR &= ~RCC_CCIPR_USART3SEL;
			RCC->CCIPR |= RCC_CCIPR_USART3SEL_0;
			af = 0x77;	break;
		case (uint32_t)UART4:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART4EN;
			RCC->CCIPR &= ~RCC_CCIPR_UART4SEL;
			RCC->CCIPR |= RCC_CCIPR_UART4SEL_0;
			af = 0x88;	break;
		case (uint32_t)UART5:
			RCC->APB1ENR1 |= RCC_APB1ENR1_UART5EN;
			RCC->CCIPR &= ~RCC_CCIPR_UART5SEL;
			RCC->CCIPR |= RCC_CCIPR_UART5SEL_0;
			af = 0x88;	break;
		default: return;
	}

	if (tx_pin->pin_num <= 6)
		tx_pin->port->AFR[0] |= af << (4 * tx_pin->pin_num);
	else
		tx_pin->port->AFR[1] |= af << (4 * (tx_pin->pin_num - 8));
	
	USARTx->CR1 &= ~USART_CR1_UE; // disable
	USARTx->CR1 &= ~USART_CR1_M;	// word length
	USARTx->CR1 |= wordlen;
	USARTx->CR2 &= ~USART_CR2_STOP; // stop bit
	USARTx->CR2 |= stopbit;
	USARTx->CR1 &= ~USART_CR1_PCE;  // parity enable
	USARTx->CR1 |= par_en;
	USARTx->CR1 &= ~USART_CR1_PS;   // choose parity
	USARTx->CR1 |= parity;
	USARTx->CR1 &= ~USART_CR1_OVER8;
	USARTx->CR1 |= over8;
	if (over8) {
		usartdiv = (16000000 / baud_rate);
		low_nyb = (usartdiv & 0xFul) >> 1;
		USARTx->BRR = (usartdiv & ~0xFul) & low_nyb;
	}
	else
		USARTx->BRR = 8000000 / baud_rate;
	
	USARTx->CR1  |= (USART_CR1_RE | USART_CR1_TE);  			
	USARTx->CR1  |= USART_CR1_UE;  
		
	while ((USARTx->ISR & USART_ISR_TEACK) == 0);
	while ((USARTx->ISR & USART_ISR_REACK) == 0);
}

static void *data_pointers[5];
static uint32_t lengths[5];
void USART_InterruptInit(USART_TypeDef *USARTx, void *data, uint32_t len) {
	switch ((uint32_t)USARTx) {
		case (uint32_t)USART1:
			NVIC_SetPriority(USART1_IRQn, 0); 
			NVIC_EnableIRQ(USART1_IRQn); 
			data_pointers[0] = data; 
			lengths[0] = len; break;
		case (uint32_t)USART2:
			NVIC_SetPriority(USART2_IRQn, 0); 
			NVIC_EnableIRQ(USART2_IRQn); 
			data_pointers[1] = data; 
			lengths[1] = len; break;
		case (uint32_t)USART3:
			NVIC_SetPriority(USART3_IRQn, 0);
			NVIC_EnableIRQ(USART3_IRQn); 
			data_pointers[2] = data; 
			lengths[2] = len; break;
		case (uint32_t)UART4:
			NVIC_SetPriority(UART4_IRQn, 0); 
			NVIC_EnableIRQ(UART4_IRQn); 
			data_pointers[3] = data; 
			lengths[3] = len; break;
		case (uint32_t)UART5:
			NVIC_SetPriority(UART5_IRQn, 0); 
			NVIC_EnableIRQ(UART5_IRQn);
			data_pointers[4] = data; 
			lengths[4] = len; break;
		default: return;
	}
}
void USART_Send(USART_TypeDef *USARTx, void *data, uint32_t len) {
	USARTx->CR1 &= ~USART_CR1_PEIE;
	USARTx->CR1 |= USART_CR1_TXEIE;
	
	USART_InterruptInit(USARTx, data, len); 
	
	USARTx->TDR = ((uint8_t*)data)[0] & 0xFF;
}
void USART_Read(USART_TypeDef *USARTx, void *buffer, uint32_t len) {
	USARTx->CR1 |= USART_CR1_PEIE;
	USARTx->CR1 &= ~USART_CR1_TXEIE;
	
	USART_InterruptInit(USARTx, buffer, len);
	
	((uint8_t*)buffer)[0] = USARTx->RDR & 0xFF;
}
void send(USART_TypeDef *USARTx, uint8_t *data, uint32_t counter) {
	if (USARTx->ISR & USART_ISR_TXE)
		USARTx->TDR = data[counter] & 0xFF;
}
void read(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t counter) {
	if (USARTx->ISR & USART_ISR_RXNE)
		buffer[counter] = USARTx->RDR & 0xFF;
}

void USART1_IRQHandler() {
	static int counter = 0;
	// write
	if (USART1->CR1 & USART_CR1_TXEIE) {
		if (counter >= lengths[0]) {
			counter = 1;
			USART1->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			if (counter == 0)
				counter = 1;
			send(USART1, (uint8_t*)data_pointers[0], counter++);
		}
	}
	// read
	if (USART1->CR1 & USART_CR1_PEIE) {
		if (counter > lengths[0]) {
			counter = 0; // for circular buffer
			// examine how the book does it... I'll have
			// to decide on the best way to disable read
		}
		else {
			if (counter == 0)
				counter = 1;
			read(USART1, (uint8_t*)data_pointers[0], counter++);
		}
	}
}
void USART2_IRQHandler() {
	static int counter = 0;
	// write
	if (USART2->CR1 & USART_CR1_TXEIE) {
		if (counter >= lengths[0]) {
			counter = 1;
			USART2->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			if (counter == 0)
				counter = 1;
			send(USART2, (uint8_t*)data_pointers[0], counter++);
		}
	}
	// read
	if (USART2->CR1 & USART_CR1_PEIE) {
		if (counter > lengths[0]) {
			counter = 0; // for circular buffer
			// examine how the book does it... I'll have
			// to decide on the best way to disable read
		}
		else {
			if (counter == 0)
				counter = 1;
			read(USART2, (uint8_t*)data_pointers[0], counter++);
		}
	}
}
void USART3_IRQHandler() {
	static int counter = 0;
	// write
	if (USART3->CR1 & USART_CR1_TXEIE) {
		if (counter >= lengths[0]) {
			counter = 1;
			USART3->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			if (counter == 0)
				counter = 1;
			send(USART3, (uint8_t*)data_pointers[0], counter++);
		}
	}
	// read
	if (USART3->CR1 & USART_CR1_PEIE) {
		if (counter > lengths[0]) {
			counter = 0; // for circular buffer
			// examine how the book does it... I'll have
			// to decide on the best way to disable read
		}
		else {
			if (counter == 0)
				counter = 1;
			read(USART3, (uint8_t*)data_pointers[0], counter++);
		}
	}
}
void UART4_IRQHandler() {
	static int counter = 0;
	// write
	if (UART4->CR1 & USART_CR1_TXEIE) {
		if (counter >= lengths[0]) {
			counter = 1;
			UART4->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			if (counter == 0)
				counter = 1;
			send(UART4, (uint8_t*)data_pointers[0], counter++);
		}
	}
	// read
	if (UART4->CR1 & USART_CR1_PEIE) {
		if (counter > lengths[0]) {
			counter = 0; // for circular buffer
			// examine how the book does it... I'll have
			// to decide on the best way to disable read
		}
		else {
			if (counter == 0)
				counter = 1;
			read(UART4, (uint8_t*)data_pointers[0], counter++);
		}
	}
}
void UART5_IRQHandler() {
	static int counter = 0;
	// write
	if (UART5->CR1 & USART_CR1_TXEIE) {
		if (counter >= lengths[0]) {
			counter = 1;
			UART5->CR1 &= ~USART_CR1_TXEIE;
		}
		else {
			if (counter == 0)
				counter = 1;
			send(UART5, (uint8_t*)data_pointers[0], counter++);
		}
	}
	// read
	if (UART5->CR1 & USART_CR1_PEIE) {
		if (counter > lengths[0]) {
			counter = 0; // for circular buffer
			// examine how the book does it... I'll have
			// to decide on the best way to disable read
		}
		else {
			if (counter == 0)
				counter = 1;
			read(UART5, (uint8_t*)data_pointers[0], counter++);
		}
	}
}
