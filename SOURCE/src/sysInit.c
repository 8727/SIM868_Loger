#include "sysInit.h"

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
void Sysinit(void){
  _Bool status = 0x00;
  
  #if defined DEBUG
    printf("\n\r\t\tStart setting\n\r\n");
  #endif

  status = SysTick_Config(SystemCoreClock / 1000);   //1ms
  
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  RCC->APB1ENR |= RCC_APB1ENR_BKPEN;
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

  AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
  
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
  
  GPIOB->CRL &= ~GPIO_CRL_CNF5; //LED
  GPIOB->CRL |= GPIO_CRL_MODE5;
  LED_OFF;
  
  #if defined DEBUG
  if(status){ printf("Sysinit........ERROR\r\n");
  }else{ printf("Sysinit...........OK\r\n");}
  #endif
}

//--------------------------------------------------------------------------------------------------------------------//
