#include "stdio.h"

#include "stm32f407xx.h"
#include "Board_LED.h"
#include "PB_LCD_Drivers.h"

#define RESISTOR_CURRENT 0.3e-6


void initialiseGPIO(void);
void EXTI9_5_IRQHandler (void);
void LED_on (int num);
void LED_off (int num);
void initialiseLEDandButtonPorts(void);
double calculate_resistance(double voltage);


void initialiseLEDandButtonPorts() {
	// Enables the GPIOA, GPIOD and GPIOE clocks / Note that this will force the GPIOB clock low but it isn't needed
	RCC->AHB1ENR = (RCC->AHB1ENR & ~(RCC_AHB1ENR_GPIOAEN_Msk | RCC_AHB1ENR_GPIOCEN_Msk | RCC_AHB1ENR_GPIODEN_Msk | RCC_AHB1ENR_GPIOEEN_Msk))  | 0x1D;
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER0_Msk) | (0x00 << GPIO_MODER_MODER0_Pos);
	GPIOD->MODER = (GPIOD->MODER & ~0xFF000000) | (0x55 << 24);  // who knows
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
	
	
	// Interrupts
	RCC->APB2ENR  = (RCC->APB2ENR  & ~RCC_APB2ENR_SYSCFGEN_Msk)  | (0x01 << RCC_APB2ENR_SYSCFGEN_Pos);
	
  GPIOE->MODER  = (GPIOA->MODER  & ~GPIO_MODER_MODER6_Msk)    | (0x00 << GPIO_MODER_MODER6_Pos);    // Sets PE6 as an input
	GPIOE->PUPDR  = (GPIOA->PUPDR  & ~GPIO_PUPDR_PUPD6_Msk)     | (0x00 << GPIO_PUPDR_PUPD6_Pos);    // Enables pull-up resistor
	
	SYSCFG->EXTICR[1] = (SYSCFG->EXTICR[1] & ~SYSCFG_EXTICR2_EXTI6_Msk) | (0x04 << SYSCFG_EXTICR2_EXTI6_Pos); // Configure inturrupts on PE6
	EXTI->IMR         = (EXTI->IMR         & ~EXTI_IMR_MR6_Msk)         | (0x01 << EXTI_IMR_MR6_Pos);         // Enables inturrupts on line 6
	EXTI->FTSR        = (EXTI->FTSR        & ~EXTI_FTSR_TR6_Msk)        | (0x01 << EXTI_FTSR_TR6_Pos);        // Triggers interrupts on falling edge
}

void LED_on(int num) {  // num indicates the LED number
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

void EXTI9_5_IRQHandler (void) {
    EXTI->PR = EXTI_PR_PR6;
	  GPIOD->BSRR = 0x1000;
}

double calculate_resistance(double voltage) {
	double resistance = voltage / RESISTOR_CURRENT;
	return resistance;
}

int main() {
	double ADC_measurement = 12;
	double measured_voltage;
	double resistance;
	char buffer[8];
	SystemCoreClockUpdate();
	//SysTick_Config(SystemCoreClock/2);
	initialiseLEDandButtonPorts();
	initialiseGPIO();
	NVIC_EnableIRQ(EXTI9_5_IRQn);
  PB_LCD_Init();
  
	
  while (1) {
		ADC1->CR2 = (ADC1->CR2 & ~ADC_CR2_SWSTART_Msk) | (0x01 << ADC_CR2_SWSTART_Pos); // Starts ADC conversion
		
		while ((ADC->CSR & 0x02) != 0x02) {			// Waits until ADC end-of-conversion bit is set high
		}
		
		ADC_measurement = ADC1->DR;  // Stores value of ADC
		measured_voltage = ADC_measurement / 1365;
		resistance = calculate_resistance(measured_voltage);
		snprintf(buffer, 8, "%f", measured_voltage);
		PB_LCD_Clear();
		PB_LCD_WriteString(buffer, 10);
		
		// Code for reading ADC goes here
	}
}
