#include <stdint.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_adc.h"
#include "misc.h"

volatile uint8_t UsrBtn_Trigger = 0;
volatile uint32_t timStart;
volatile float timPassedMaxInUsGlobal = 0.f;

void initClocks()
{	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
}

void initGPIO()
{
#ifdef STM32F429_439xx
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed= GPIO_Speed_2MHz;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOE, GPIO_Pin_2); // Data
	GPIO_ResetBits(GPIOE, GPIO_Pin_6); // Clock


	// LEDs
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStruct);
	GPIO_ResetBits(GPIOG, GPIO_Pin_13);
	GPIO_SetBits(GPIOG, GPIO_Pin_14);

#elif (defined STM32F40_41xxx) 
	// LEDs
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_12;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_SetBits(GPIOD, GPIO_Pin_12);

#endif

}

// see usb_bsp.c !!!
//
// void init_GPIO_USB()
// {
// 	GPIO_InitTypeDef GPIO_InitStructure;

// 	/* Configure Data-Pin DP */
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_DP;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(GPIO_PORT_USB_DP, &GPIO_InitStructure);
// 	GPIO_PinAFConfig(GPIO_PORT_USB_DP, GPIO_AFPIN_USB_DP, GPIO_AF_OTG_FS);

// 	/* Configure Data-Pin DM */
// 	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_DM;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(GPIO_PORT_USB_DM, &GPIO_InitStructure);
// 	GPIO_PinAFConfig(GPIO_PORT_USB_DM, GPIO_AFPIN_USB_DM, GPIO_AF_OTG_FS);

// 	/* Configure OTG FS ID */
// 	// GPIO_InitStructure.GPIO_Pin =  GPIO_PIN_USB_ID;
// 	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
// 	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // GPIO_Mode_AF  // GPIO_Mode_IN
// 	// GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // GPIO_OType_OD // GPIO_OType_PP
// 	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // GPIO_PuPd_NOPULL // GPIO_PuPd_UP
// 	// GPIO_Init(GPIO_PORT_USB_ID, &GPIO_InitStructure);
// 	// GPIO_PinAFConfig(GPIO_PORT_USB_ID, GPIO_PIN_USB_ID, GPIO_AF_OTG_FS); // GPIO_AF_OTG2_HS // GPIO_AF_OTG1_FS

// 	/* Configure  VBUS Pin (Input) */
// 	// GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_VBUS;
// 	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
// 	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
// 	// GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
// 	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	// GPIO_Init(GPIO_PORT_USB_VBUS, &GPIO_InitStructure);
// 	// GPIO_PinAFConfig(GPIO_PORT_USB_VBUS, GPIO_PIN_USB_VBUS, GPIO_AF_OTG_FS);

// 	/* Configure OTG FS PSO (VBUS Enable) */
// 	/*
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
// 	GPIO_SetBits(GPIOC, GPIO_Pin_4);
// 	*/
// }

void initUserButton()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef myNVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure Button pin as input */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Connect Button EXTI Line to Button GPIO Pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	/* Configure Button EXTI line */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	myNVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	myNVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	myNVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	myNVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&myNVIC_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		UsrBtn_Trigger = 1;
	}
    EXTI_ClearITPendingBit(EXTI_Line0);
}

void initTIM1(void){

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	// TIM1 clock enable

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_Period = UINT16_MAX - 1;
	// TIM_TimeBaseInitStruct.TIM_Prescaler = 168 - 1;	// ergibt Zaehlfrequenz von 1/(1 us)
	TIM_TimeBaseInitStruct.TIM_Prescaler =  24 - 1;	// ergibt Zaehlfrequenz von 7 MHz
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;	// 0x00...0xFF; nur TIM1 und TIM8
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 0...15 (lower value --> higher prio)
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 0...15 (lower value --> higher prio)
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM1, ENABLE);
}

void TIM1_UP_TIM10_IRQHandler(void){

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

void startTimeMeas()
{
	timStart = TIM1->CNT;
}

void stopTimeMeas(float * timPassedInUs, float * timPassedMaxInUs)
{
	// get timestamp directly after end of function
	uint32_t timEnd = TIM1->CNT;

	// calculate delta time
	uint32_t timDelta;
	if(timStart <= timEnd)
	{
		timDelta = timEnd - timStart;
	}
	else // if cnt overflow occured
	{
		timDelta = (UINT16_MAX - timStart) + timEnd; // timer counts up to UINT16_MAX-1
	}
	
	*timPassedInUs = (float)(((double)timDelta) / 7.0); // TIM1 counts with 7 MHz
	if(*timPassedInUs > timPassedMaxInUsGlobal)
	{
		timPassedMaxInUsGlobal = *timPassedInUs;
	}

	*timPassedMaxInUs = timPassedMaxInUsGlobal;
}
