//
//  general.h
//
//  Written by Xander Will
//
//  "Generally useful functions and typedefs"
//

typedef enum {
	false = 0ul,
	true  = 1ul	
} bool;

void ClockInit() {
	// Enable High Speed Internal Clock (HSI = 16 MHz)
  RCC->CR |= ((uint32_t)RCC_CR_HSION);
	
  // wait until HSI is ready
  while ( (RCC->CR & (uint32_t) RCC_CR_HSIRDY) == 0 ) {;}
	
  // Select HSI as system clock source 
  RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_HSI;  //01: HSI16 oscillator used as system clock

  // Wait till HSI is used as system clock source 
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) == 0 ) {;}
}

void FreeArray(void **a, int len) {
	/* it's just an array free lol */
	int i;
	for (i = 0; i < len; i++)
		free(a[i]);
}