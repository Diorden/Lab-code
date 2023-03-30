#include "stdio.h"

#include "STM32F407xx.h"
#include "Board_LED.h"
#include "PB_LCD_Drivers.h"

void SysTick_Handler (void);
void initialiseLEDandButtonPorts(void);
void initialiseLab3GPIOStuff(void);
void LED_on(int);
void LED_off(int);
int button_is_pressed(void);

uint32_t LEDState = 0;

void SysTick_Handler () {
    LEDState = 1 - LEDState;
}
          
void initialiseLEDandButtonPorts() {
	// Enables the GPIOA, GPIOD and GPIOE clocks / Note that this will force the GPIOB clock low but it isn't needed
	RCC->AHB1ENR = (RCC->AHB1ENR & ~(RCC_AHB1ENR_GPIOAEN_Msk | RCC_AHB1ENR_GPIOCEN_Msk | RCC_AHB1ENR_GPIODEN_Msk | RCC_AHB1ENR_GPIOEEN_Msk))  | 0x1D;
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER0_Msk) | (0x00 << GPIO_MODER_MODER0_Pos);
	GPIOD->MODER = (GPIOD->MODER & ~0xFF000000) | (0x55 << 24);  
}

void initialiseGPIO() {
	GPIOE->AFR[0] = (GPIOE->AFR[0] & ~GPIO_AFRL_AFSEL7_Msk)   | (0x01 << GPIO_AFRL_AFSEL7_Pos);   // Sets PE7 to act as a timer1 clock
	RCC->APB2ENR  = (RCC->APB2ENR  & ~RCC_APB2ENR_TIM1EN_Msk) | (0x01 << RCC_APB2ENR_TIM1EN_Pos); // Enables TIM1 clock
	TIM1->SMCR    = (TIM1->SMCR    & ~TIM_SMCR_ECE_Msk)       | (0x01 << TIM_SMCR_ECE_Pos);       // Sets TIM1_ETR as clock source
  TIM1->CR1     = (TIM1->CR1     & ~TIM_CR1_CEN_Msk)        | (0x01 << TIM_CR1_CEN_Pos);        // Enables the counter
	GPIOE->MODER  = (GPIOE->MODER  & ~GPIO_MODER_MODER7_Msk)  | (0x02 << GPIO_MODER_MODER7_Pos);  // Sets PE7 to alternate function mode
	
	RCC->APB2ENR = (RCC->APB2ENR & ~RCC_APB2ENR_ADC1EN_Msk) | (0x01 << RCC_APB2ENR_ADC1EN_Pos); // Enables ADC1
	RCC->APB1ENR = (RCC->APB1ENR & ~RCC_APB1ENR_DACEN_Msk)  | (0x01 << RCC_APB1ENR_DACEN_Pos);  // Enables DAC
	GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODER4_Msk)  | (0x03 << GPIO_MODER_MODER4_Pos);  // Sets PC4 to act as ADC in 
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER4_Msk)  | (0x03 << GPIO_MODER_MODER4_Pos);  // Sets PA4 anologue out for DAC1
	ADC1->SQR3   = (ADC1->SQR3   & ~ADC_SQR3_SQ1_Msk)       | (0x0E << ADC_SQR3_SQ1_Pos);
	ADC1->CR2    = (ADC1->CR2    & ~ADC_CR2_ADON_Msk)       |  0x01;                            // Enables ADC
	ADC1->CR2    = (ADC1->CR2    & ~ADC_CR2_EOCS_Msk)       | (0x01 << ADC_CR2_EOCS_Pos);       // Enables end of conversion bit on each conversion
	ADC1->CR1    = (ADC1->CR1    & ~ADC_CR1_DISCEN_Msk)     | (0x01 << ADC_CR1_DISCEN_Pos);     // Enables discontinuous mode
	DAC->CR      = (DAC->CR      & ~DAC_CR_EN1_Msk)         | (0x01 << DAC_CR_EN1_Pos);         // Enables channel 1 of the DAC
}

int button_is_pressed() {
	// if button pressed return 1    
  if ((GPIOA->IDR & 0x01) == 0x00)
		return 0;
  else if ((GPIOA->IDR & 0x01) == 0x01)
		return 1;
	else;
	  return -1;
}

void LED_on(int num) {
	if (num == 1)
	    GPIOD->BSRR = 0x2000;
	else if (num == 2)
      GPIOD->BSRR = 0x1000;
	else if (num == 3)
      GPIOD->BSRR = 0x4000;
	else if (num == 4)
      GPIOD->BSRR = 0x8000;
}

void LED_off(int num) {
	if (num == 1)
	    GPIOD->BSRR = 0x20000000;
	else if (num == 2)
      GPIOD->BSRR = 0x10000000;
	else if (num == 3)
      GPIOD->BSRR = 0x40000000;
	else if (num == 4)
      GPIOD->BSRR = 0x80000000;
}

int main() {
	volatile double ADCconv = 12;
	volatile int count_value;
	char buffer[33];
  SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/2);
	initialiseLEDandButtonPorts();
	initialiseGPIO();
  PB_LCD_Init();
	
	
  while (1) {
		ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_SWSTART_Msk) | (0x01 << ADC_CR2_SWSTART_Pos);  

		 // while ((ADC->CSR & 0x02) != 0x02) { // Waits until end of conversion bit is set high

		
			ADCconv = ADC1->DR;
		  count_value = TIM1->CNT;
			DAC->DHR12R1 = (DAC->DHR12R1 & ~DAC_DHR12R1_DACC1DHR_Msk) | ADC1->DR;  // Sends the ADC1 output to the DAC data reg
		  snprintf(buffer, 10, "%d", count_value);
		  PB_LCD_Clear();
		  PB_LCD_WriteString(buffer, 10);
			
	    //if (LEDState) LED_on(4);
		  //else LED_off(4);
		}
	
}
