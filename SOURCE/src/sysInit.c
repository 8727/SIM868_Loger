#include "sysInit.h"

static __IO uint32_t msTicks;
uint32_t upTime;

//--------------------------------------------------------------------------------------------------------------------//
#if defined ACTIVE_SWO
  struct __FILE { int handle; };
  FILE __stdout;
  int fputc(int ch, FILE *f){ if(DEMCR & TRCENA){ while(ITM_Port32(0x00) == 0x00);
    ITM_Port8(0x00) = ch; }
    return(ch);
  }
#endif

//--------------------------------------------------------------------------------------------------------------------//
void SysTick_Handler(void){ msTicks++; }

//--------------------------------------------------------------------------------------------------------------------//
void DelayMs(uint32_t ms){ uint32_t tickStart = msTicks;
  while((msTicks - tickStart) < ms){}
}

//--------------------------------------------------------------------------------------------------------------------//
void Sysinit(void){
  _Bool status = 0x00;
  
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
  PWR->CR1 |= PWR_CR1_DBP;

  
  RCC->APB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_AHB2ENR_GPIOBEN;
  RCC->APB2ENR |= RCC_AHB2ENR_GPIOCEN;
  RCC->APB2ENR |= RCC_AHB2ENR_GPIODEN;
  RCC->APB2ENR |= RCC_AHB2ENR_GPIODEN;
  
  status = SysTick_Config(SystemCoreClock / 1000);   //1ms
}

//--------------------------------------------------------------------------------------------------------------------//
