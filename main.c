#include "stm32f4xx.h"

int i = 0;

void LEDInit(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	GPIOD->MODER |= GPIO_MODER_MODER15_0
								| GPIO_MODER_MODER14_0;
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15_1
									|	GPIO_OSPEEDER_OSPEEDR14_1;
}

void SysTickInit(void){
	SysTick->LOAD = 20999;
	SysTick->VAL = 0;
	SysTick->CTRL &= 0xFFFFFFF8;
	SysTick->CTRL |= 0x00000003;
}

void SysTick_Handler(void){
	i++;
	if(i == 500){ //po 5000ms
		GPIOD->BSRRL = GPIO_BSRR_BS_15;
	}
	if(i == 1000){ //po 1000ms
		GPIOD->BSRRH = GPIO_BSRR_BS_15;
		i = 0;
	}
}

int main(void){
	SysTickInit();
	LEDInit();
	
	//if(RCC->CR & 0x00000002) GPIOD->BSRRL = GPIO_BSRR_BS_15;
	//if(RCC->CR & 0x00020000) GPIOD->BSRRL = GPIO_BSRR_BS_14;
	while(1){
		
	}
	return 0;
}