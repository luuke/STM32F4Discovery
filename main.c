#include "stm32f4xx.h"

int i = 0;
int j = 0;

void LEDInit(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER |= GPIO_MODER_MODER15_0
							 |  GPIO_MODER_MODER14_0;
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15_1
								 |	GPIO_OSPEEDER_OSPEEDR14_1;
}

void SysTickInit(void){
	SysTick->LOAD = 20999;
	SysTick->VAL = 0;
	SysTick->CTRL &= 0xFFFFFFF8;
	SysTick->CTRL |= 0x00000003;
}

void TIM4Init(void){
	uint32_t const APB1Freq = 84000000;
	uint32_t const TIM4Freq = 100000;
	uint32_t const PWMFreq = 50;
	
	NVIC_EnableIRQ(TIM4_IRQn); //Enabling TIM4 interrupt in NVIC
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; //Enabling clock for TIM4
	TIM4->PSC = APB1Freq / TIM4Freq - 1;
	TIM4->ARR = TIM4Freq / PWMFreq - 1;
	TIM4->DIER |= TIM_DIER_UIE;	//Enabling update interrupt
	TIM4->CR1 |= TIM_CR1_ARPE //TIMx_ARR register is buffered
						|	 TIM_CR1_CEN;	//Enabling counter
}

void TIM4_IRQHandler(void){
	if(TIM4->SR & TIM_SR_UIF)
		TIM4->SR &= ~TIM_SR_UIF; //interrupt flag must be cleared after entering interrupt handler
	
	j++; //each ++ every 20ms
	if(j == 25){ //after 25*20ms
		GPIOD->BSRRL = GPIO_BSRR_BS_14;
	}
	if(j == 50){ //after 50*20ms
		GPIOD->BSRRH = GPIO_BSRR_BS_14;
		j = 0;
	}
		
	TIM4->CCR2++;
	if(TIM4->CCR2 == 1999) TIM4->CCR2 = 0;
}

void SysTick_Handler(void){
	i++; //each ++ every 1ms
	if(i == 500){ //after 500*1ms
		GPIOD->BSRRL = GPIO_BSRR_BS_15;
	}
	if(i == 1000){ //after 1000*1ms
		GPIOD->BSRRH = GPIO_BSRR_BS_15;
		i = 0;
	}
}

int main(void){
	SysTickInit();
	TIM4Init();
	LEDInit();
	
	GPIOD->MODER |= GPIO_MODER_MODER13_1;
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13_1;
	GPIOD->AFR[1] |= 0x00200000;
	TIM4->CCER |= TIM_CCER_CC2E;
	TIM4->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM4->CCR2 = 0;
	
	//if(RCC->CR & RCC_CR_HSIRDY) GPIOD->BSRRL = GPIO_BSRR_BS_13;
	//if(RCC->CR & RCC_CR_HSERDY) GPIOD->BSRRL = GPIO_BSRR_BS_12;
	while(1){
		
	}
	return 0;
}
