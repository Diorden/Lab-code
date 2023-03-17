#include "stdio.h"

#include "stm32f407xx.h"


void initialiseGPIO(void);
void EXTI9_5_IRQHandler (void);


void initialiseGPIO() {
	RCC->AHB1ENR  = (RCC->AHB1ENR  & ~RCC_AHB1ENR_GPIOEEN_Msk) | (0x01 << RCC_AHB1ENR_GPIOEEN_Pos);  // Enables GPIOE clock
	RCC->APB2ENR  = (RCC->APB2ENR  & ~RCC_APB2ENR_SYSCFGEN_Msk)  | (0x01 << RCC_APB2ENR_SYSCFGEN_Pos);
	
  GPIOE->MODER  = (GPIOA->MODER  & ~GPIO_MODER_MODER6_Msk)    | (0x00 << GPIO_MODER_MODER6_Pos);    // Sets PE6 as an input
	GPIOE->PUPDR  = (GPIOA->PUPDR  & ~GPIO_PUPDR_PUPD6_Msk)     | (0x00 << GPIO_PUPDR_PUPD6_Pos);    // Enables pull-up resistor
	
	SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI6_Msk) | (0x04 << SYSCFG_EXTICR2_EXTI6_Pos); // Configure inturrupts on PE6
	EXTI->IMR         = (EXTI->IMR         & ~EXTI_IMR_MR6_Msk)         | (0x01 << EXTI_IMR_MR6_Pos);         // Enables inturrupts on line 6
	EXTI->FTSR        = (EXTI->FTSR        & ~EXTI_FTSR_TR6_Msk)        | (0x01 << EXTI_FTSR_TR6_Pos);        // Triggers interrupts on falling edge
}


void EXTI9_5_IRQHandler (void) {
    EXTI->PR = EXTI_PR_PR6;
	  GPIOD->BSRR = 0x2000;
}

int main() {
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	initialiseGPIO();
  
	
	
  while (1) {

		}
	
}
