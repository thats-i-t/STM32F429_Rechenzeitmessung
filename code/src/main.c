#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "tm_stm32f4_ili9341.h"
#include "tm_stm32f4_fonts.h"

#include "basic_functions.h"
#include "periph_functions.h"
#include "diagnostics.h"
#include "usb.h"
#include "usb_defines.h"
#include "usbd_desc.h"
#include "stm32_ub_usb_cdc.h"


int main(void)
{
	SystemInit();
	
	// Init peripherals
	initClocks();
#ifdef STM32F429_439xx
	initGPIO();
	initUserButton();
#endif
	initTIM1();
	UB_USB_CDC_Init();
	// char rx_buf[APP_TX_BUF_SIZE];
  	// USB_CDC_RXSTATUS_t check = RX_EMPTY;

#ifdef STM32F429_439xx
	// Init display
	TM_ILI9341_Init();
	TM_ILI9341_Rotate(TM_ILI9341_Orientation_Portrait_2);
	clearAllLines();

	// Display compile date
	char SWdate[20];
	strcpy(SWdate, "SW date: ");
	strcat(SWdate, getCompileDate());
	disp_str(SWdate, 1);

	// Display compile time
	char SWtime[20];
	strcpy(SWtime, "SW time: ");
	strcat(SWtime, getCompileTime());
	disp_str(SWtime, 2);

	// Display float variable
	float testVar_f32 = 1.23456789f;
	disp_fvar("Beispiel float: ", testVar_f32, 26);
	
	// Display int variable
	uint32_t testVar = 123;
	disp_var("Beispiel int: ", testVar, 27);
	
	// Red LED off
	GPIO_ResetBits(GPIOG, GPIO_Pin_14);
#endif

	while(1)
	{		
#ifdef STM32F429_439xx
		// User button pressed?
		if(UsrBtn_Trigger == 1)
		{
			UsrBtn_Trigger = 0;
		}
		
		// Display the time passed (values calculated in usb.c)
		disp_fvar("Time passed in us: ", (float)timPassedInUs, 5);
		disp_fvar("Time passed max in us: ", (float)timPassedMaxInUs, 7);

		// Lines which shall be printed with disp_var(), disp_fvar(), etc. are only shown when
		// the function update_display() is called subsequently.
		update_display();
#endif

		/* ############################################################################################### */
		/* This was used for initially testing the time measurement */
		/* ############################################################################################### */
		// startTimeMeas(); // log timestamp at start

		// // Toggle green LED
		// GPIO_SetBits(GPIOG, GPIO_Pin_13);
		// // waitMs(1);
		// waitUs(10);
		// GPIO_ResetBits(GPIOG, GPIO_Pin_13);
		
		// // get timestamp directly after end of function
		// float timPassedInUs, timPassedMaxInUs;
		// stopTimeMeas(&timPassedInUs, &timPassedMaxInUs);

		// // Display the time passed
		// disp_fvar("Time passed in us: ", (float)timPassedInUs, 5);
		// disp_fvar("Time passed max in us: ", (float)timPassedMaxInUs, 7);
		
		/* ############################################################################################### */

		/* ############################################################################################### */
		// USB communication (this loopback was only used for testing)
		/* ############################################################################################### */
		// if(UB_USB_CDC_GetStatus() == USB_CDC_CONNECTED)
		// {
		// 	check = UB_USB_CDC_ReceiveString(rx_buf);
		// 	if(check == RX_READY)
		// 	{
		// 		UB_USB_CDC_SendString(rx_buf,LFCR);
		// 	}
		// }
		/* ############################################################################################### */
	}
}



