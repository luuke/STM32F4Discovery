#include "stm32f4xx.h"

volatile int i = 0;
volatile int j = 0;
volatile int adcData = 0;

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
		
	//TIM4->CCR2++;
	//if(TIM4->CCR2 == 1999) TIM4->CCR2 = 0;
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

void TIM4PWMInit(){
	GPIOD->MODER |= GPIO_MODER_MODER13_1;
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR13_1;
	GPIOD->AFR[1] |= 0x00200000;
	TIM4->CCER |= TIM_CCER_CC2E;
	TIM4->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	TIM4->CCR2 = 0;
}

void SPI1Init(){
//change NSS pin GPIO init if not working!!
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //enabling SPI1 clock
	SPI1->CR2 |= SPI_CR2_SSOE;
	SPI1->CR1 |= SPI_CR1_SSM
						|  SPI_CR1_SSI
						|  SPI_CR1_BR_0
						|  SPI_CR1_BR_1
						|  SPI_CR1_CPHA
						|  SPI_CR1_CPOL
						|  SPI_CR1_MSTR
						|  SPI_CR1_SPE;
}
void SPI1SendCommand(uint8_t cmd){
	GPIOA->BSRRH = GPIO_BSRR_BS_6; //A0=0 -> sending command
	GPIOA->BSRRH = GPIO_BSRR_BS_4; // nCS1B=0
	SPI1->DR = cmd;
	while(!(SPI1->SR & SPI_SR_TXE));
	while(!(SPI1->SR & SPI_SR_BSY));
//here add waiting for BSY = 0 if not working!!!
	GPIOA->BSRRL = GPIO_BSRR_BS_4; // nCS1B=1
}

void SPI1SendData(uint8_t data){
	GPIOA->BSRRL = GPIO_BSRR_BS_6; //A0=1 -> sending display data
	GPIOA->BSRRH = GPIO_BSRR_BS_4; // nCS1B=0
	SPI1->DR = data;
	while(!(SPI1->SR & SPI_SR_TXE));
//here add waiting for BSY = 0 if not working!!!
	GPIOA->BSRRL = GPIO_BSRR_BS_4; // nCS1B=1
}

void LCDInit(){
	/*
	 * 		 SI - PA7 - AF  - PP
	 * 		 A0 - PA6 - OUT - PP
	 * 	  SCL - PA5 - AF  -	PP
	 *  nCS1B - PA4 - OUT - PU
	 * nRESET - PA3 - OUT - PU
	 */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	//enabling PORT A clock
	GPIOA->MODER |= GPIO_MODER_MODER3_0		//PA3 as output
							 |  GPIO_MODER_MODER4_0		//PA4 as output
							 |	GPIO_MODER_MODER5_1		//PA5 as alternate function
							 |	GPIO_MODER_MODER6_0		//PA6 as output
							 |	GPIO_MODER_MODER7_1;	//PA7 as alternate function
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3_1	//all speed 50MHz
								 |  GPIO_OSPEEDER_OSPEEDR4_1
								 |	GPIO_OSPEEDER_OSPEEDR5_1
								 |	GPIO_OSPEEDER_OSPEEDR6_1
								 |	GPIO_OSPEEDER_OSPEEDR7_1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR3_0		//PA3 pull-up
							 |	GPIO_PUPDR_PUPDR4_0;	//PA4 pull-up			
	GPIOA->AFR[0] |= 0x50500000; //PA7 and PA5 alternate function AF5 (SPI1)
	GPIOA->BSRRL = GPIO_BSRR_BS_3
							 | GPIO_BSRR_BS_4;
	
	SPI1Init();
	
	SPI1SendCommand(0x40);
	SPI1SendCommand(0xA1);
	SPI1SendCommand(0xC0);
	SPI1SendCommand(0xA6);
	SPI1SendCommand(0xA2);
	SPI1SendCommand(0x2F);
	SPI1SendCommand(0xF8);
	SPI1SendCommand(0x00);
	SPI1SendCommand(0x27);
	SPI1SendCommand(0x81);
	SPI1SendCommand(0x16);
	SPI1SendCommand(0xAC);
	SPI1SendCommand(0x00);
	SPI1SendCommand(0xAF);
	
	
	//Displaying test
	SPI1SendCommand(0xB0);
	SPI1SendCommand(0x10);
	SPI1SendCommand(0x00);
	SPI1SendData(0xFF);
	SPI1SendCommand(0xA5);
}

void ADCInit(){
	//GPIOA init if ADC not working
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER1; 
	NVIC_EnableIRQ(ADC_IRQn);
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC->CCR |= ADC_CCR_ADCPRE_1;
	ADC1->SQR3 |= ADC_SQR3_SQ1_0;
	ADC1->CR2 |= ADC_CR2_CONT;
	ADC1->CR2 |= ADC_CR2_EOCS;
	ADC1->CR1 |= ADC_CR1_EOCIE;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;
}

void ADC_IRQHandler(void){
	adcData = ADC1->DR;
	adcData /= 20;
	
	TIM4->CCR2 = adcData ;
	
}

int main(void){
	LEDInit();
	SysTickInit();
	TIM4Init();
	TIM4PWMInit();
	//LCDInit(); //not working
	ADCInit();
	
	while(1){
		
	}
}
