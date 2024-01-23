#include <stdint.h>
#include <string.h>

// #include "usbd_cdc_vcp.h"
#include "usbd_conf.h"
#include "usb_user.h"

#ifdef STM32F40_41xxx
#include "usart3.h"
#include "stm32f4xx_gpio.h"
#endif

#include "basic_functions.h"
#include "periph_functions.h"


#define buflen 200
char BufArr[buflen];
char * Buf;
uint32_t bufend = 0;

typedef float tFxx;


extern uint8_t  APP_Rx_Buffer [];
extern uint32_t APP_Rx_ptr_in;

volatile float timPassedInUs, timPassedMaxInUs;


void calc_opt_op_cur(tFxx *i_0,tFxx m_ref,tFxx n,tFxx *Rs, tFxx *Ls, tFxx *Psi,tFxx Usmax,tFxx Ismax,tFxx n_nom,tFxx np,tFxx ibatmax,tFxx ibatmin,tFxx uzk,tFxx n_max,tFxx *output);
void RefCurrentCalculation(tFxx* IdRef, tFxx* IqRef, tFxx* Mlim, char * str, tFxx Mref, tFxx omEl, const tFxx* param) ;

// Dummy
// void RefCurrentCalculation(tFxx* IdqRef, tFxx* MrefLim, tFxx Mref, tFxx omEl, const tFxx* p)
// {
// 	(void)omEl;
// 	(void)p[0];
// 	IdqRef[0] = -15.f;
// 	IdqRef[1] = 20.f;
// 	*MrefLim = Mref;
// }

#ifdef STM32F429_439xx
void send_data_USB(uint8_t* Buf, uint32_t Len)
{
	uint32_t i;
	for(i=0; i<Len; i++)
	{
		APP_Rx_Buffer[APP_Rx_ptr_in] = Buf[i];

		APP_Rx_ptr_in++;
		if(APP_Rx_ptr_in == APP_RX_DATA_SIZE)
		{
			APP_Rx_ptr_in = 0;
		}
	}
}
#endif

uint8_t check_header(char *buf){

	char header[5];
	memcpy(header, buf, 4);
	header[4] = '\0';

	if(!strcmp(header, "HEAD")){
		return 1;
	}
	return 0;
}

void process_received_data(char* BufIn, uint32_t Len){

	// // Buf = BufArr;
	// // uint32_t i = 0;
	// // for(i = 0; i < Len; i++)
	// // {
	// // 	Buf[bufend] = BufIn[i];
	// // 	bufend++;

	// // 	if(bufend >= LEN_RX_FRAME)
	// // 	{
			uint32_t idx = 0;

	Buf = BufIn;

	// 		if(1) // check_header(&Buf[idx])){
	// 		{	
				static uint32_t callcnt = 0;
				callcnt++;
				float callcntf32 = (float)callcnt;

				static tFxx IdLast = 0.f, IqLast = 0.f;
				tFxx IdqRef[2], MrefLim[2], Mref, omEl, p[11], Imax, Uzk, Rs, Ld, Lq, PsiP, Np, reluk, Ureserve;

				// Read inputs for RefCurrCalc from received data
				Buf = &Buf[idx + 4];
				float *pt32 = (float*)Buf;
				memcpy((char*)&Mref, (char*)&pt32[0], 4);
				memcpy((char*)&omEl, (char*)&pt32[1], 4);
				memcpy((char*)&Rs, (char*)&pt32[2], 4);
				memcpy((char*)&Ld, (char*)&pt32[3], 4);
				memcpy((char*)&Lq, (char*)&pt32[4], 4);
				memcpy((char*)&PsiP, (char*)&pt32[5], 4);
				memcpy((char*)&Np, (char*)&pt32[6], 4);
				memcpy((char*)&Uzk, (char*)&pt32[7], 4);
				memcpy((char*)&Imax, (char*)&pt32[8], 4);
				memcpy((char*)&reluk, (char*)&pt32[9], 4);
				memcpy((char*)&Ureserve, (char*)&pt32[10], 4);
				
				//#####################################################################################################################
				//#####################################################################################################################
				tFxx Umax = Uzk / 1.732050807568f - Ureserve;
				tFxx n_nom = 1335.f;

	// Np 			= param[0]; 	// number of pole pairs
	// Rs 			= param[1];		// phase resistance	in Ohm
	// Ld 			= param[2];		// direct-inductance in Henry
	// Lq 			= param[3];		// quadrature inductance in Henry
	// PsiP 		= param[4];		// PM magnet flux in Volt*sec
	// Umax 		= param[5]; 	// maximum phase voltage amplitude in Volt
	// Imax 		= param[6]; 	// maximum phase current amplitude in Ampere
	// IbattMax 	= param[7]; 	// Max. battery current in Ampere (> 0)
	// IbattMin 	= param[8]; 	// Min. battery current in Ampere (< 0)
	// Uzk 		= param[9]; 	// DC Link voltage
	// omNenn 		= param[10]; 	// rated el. angular speed
				p[0] = (tFxx)Np;
				p[1] = (tFxx)Rs;
				p[2] = (tFxx)Ld;
				p[3] = (tFxx)Lq;
				p[4] = (tFxx)PsiP;
				p[5] = (tFxx)Umax;
				p[6] = (tFxx)Imax;
				p[7] = (tFxx)500;
				p[8] = (tFxx)-500;
				p[9] = (tFxx)Uzk;
				p[10] = (tFxx)n_nom; // omNenn
				
				static char str[2];

				startTimeMeas(); // log timestamp at start					
				GPIO_SetBits(GPIOD, GPIO_Pin_13);
				RefCurrentCalculation(&IdqRef[0], &IdqRef[1], MrefLim, str, Mref, omEl, (const tFxx*)&p); // call function-under-test			
				GPIO_ResetBits(GPIOD, GPIO_Pin_13);
				stopTimeMeas((float*)&timPassedInUs, (float*)&timPassedMaxInUs); // log time directly after end of function

				float isum = IdqRef[0]+IdqRef[1];

				float testVar = 123.4;
				uint8_t TxBuff8[LEN_TX_FRAME];
				strcpy((char*)TxBuff8, "HEAD");
				uint32_t * TxBuff32;
				TxBuff32 = (uint32_t*)TxBuff8;
				memcpy((char*)&TxBuff32[1], (char*)&IdqRef[0], 4);
				memcpy((char*)&TxBuff32[2], (char*)&IdqRef[1], 4);
				memcpy((char*)&TxBuff32[3], (char*)MrefLim, 4);
				memcpy((char*)&TxBuff32[4], (char*)&pt32[11], 4); // Timestamp is sent back (loopback)
				memcpy((char*)&TxBuff32[5], (char*)&timPassedInUs, 4);
				// memcpy((char*)&TxBuff32[6], (char*)&testVar, 4);
				memcpy((char*)&TxBuff32[6], (char*)&isum, 4);
				// memcpy((char*)&TxBuff32[7], (char*)&output[3], 4);
				memcpy((char*)&TxBuff32[8], (char*)&omEl, 4);				
				//#####################################################################################################################
				//#####################################################################################################################

				// // tFxx i_0[2] = { 0., 0. };
				// // tFxx m_ref = 250.f;
				// // tFxx n = 10500.f;
				// // tFxx Rsx[4] = { 0.057021246441950f, 0.f, 0.057021246441950f, 0.f };
				// // tFxx Ls[4] = {1.057812045982978f, 0.f, 0.f , 3.541165744747624f}; 
				// // tFxx Psi[2] = {0.596176046255743f, -6.284096542285975e-05f};
				// // tFxx Usmax = 350.f;
				// // tFxx Ismax = 520.f;
				// // tFxx n_nom = 1335.f;
				// // tFxx np = 4.f;
				// // tFxx ibatmax = 360.f;
				// // tFxx ibatmin = -350.f;
				// // tFxx uzk = 650.f;
				// // tFxx n_max = 18250.f;
				// // tFxx output[4];
				
				// tFxx Umax = Uzk / 1.732050807568f - Ureserve;
				// tFxx i_0[2] = { IdLast, IqLast };
				// tFxx m_ref = Mref;
				// tFxx n_nom = 2000.f;
				// tFxx n = omEl / Np / 3.141592653589793238f * 30.f;
				// tFxx Rsx[4] = { Rs * Imax/Umax , 0.f, 0.f, Rs * Imax/Umax };
				// tFxx Ls[4] = {Ld * n_nom*Imax/Umax, 0.f, 0.f , Lq * n_nom*Imax/Umax}; 
				// tFxx Psi[2] = {(PsiP + IdLast * Ld) * n_nom/Umax, (IqLast * Lq) * n_nom/Umax};
				// tFxx Usmax = Umax;
				// tFxx Ismax = Imax;
				// tFxx np = Np;
				// tFxx ibatmax = 500.f;
				// tFxx ibatmin = -500.f;
				// tFxx uzk = Uzk;
				// tFxx n_max = 20000.f;
				// tFxx output[4];
				

				// startTimeMeas(); // log timestamp at start			
				// GPIO_SetBits(GPIOD, GPIO_Pin_13);
				// calc_opt_op_cur(  i_0, // call function-under-test
				// 				m_ref,
				// 				n,
				// 				Rsx, 
				// 				Ls, 
				// 				Psi,
				// 				Usmax,
				// 				Ismax,
				// 				n_nom,
				// 				np,
				// 				ibatmax,
				// 				ibatmin,
				// 				uzk,
				// 				n_max,
				// 				output
				// 			);
				// GPIO_ResetBits(GPIOD, GPIO_Pin_13);
				// stopTimeMeas((float*)&timPassedInUs, (float*)&timPassedMaxInUs); // log time directly after end of function

				// tFxx Id = output[0] * Imax;
				// tFxx Iq = output[1] * Imax;

				// Id = Id > Imax ? Imax : Id;
				// Id = Id < -Imax ? -Imax : Id;
				// Iq = Iq > Imax ? Imax : Iq;
				// Iq = Iq < -Imax ? -Imax : Iq;

				// IdLast = Id;
				// IqLast = Iq;

				// output[2] = output[2]/n_nom*Imax*Usmax;

				// // Iq = Mref/(1.5f*Np*PsiP);
				// // Id = 0.f;
				// // MrefLim[0] = Mref;

				// float isum = Id+Iq;

				// float testVar = 123.4;
				// uint8_t TxBuff8[LEN_TX_FRAME];
				// strcpy((char*)TxBuff8, "HEAD");
				// uint32_t * TxBuff32;
				// TxBuff32 = (uint32_t*)TxBuff8;
				// // memcpy((char*)&TxBuff32[1], (char*)&IdqRef[0], 4);
				// // memcpy((char*)&TxBuff32[2], (char*)&IdqRef[1], 4);
				// memcpy((char*)&TxBuff32[1], (char*)&Id, 4);
				// memcpy((char*)&TxBuff32[2], (char*)&Iq, 4);
				// memcpy((char*)&TxBuff32[3], (char*)&output[2], 4);
				// memcpy((char*)&TxBuff32[4], (char*)&pt32[11], 4); // Timestamp is sent back (loopback)
				// memcpy((char*)&TxBuff32[5], (char*)&timPassedInUs, 4);
				// // memcpy((char*)&TxBuff32[6], (char*)&testVar, 4);
				// memcpy((char*)&TxBuff32[6], (char*)&isum, 4);
				// memcpy((char*)&TxBuff32[7], (char*)&output[3], 4);
				// memcpy((char*)&TxBuff32[8], (char*)&omEl, 4);
				
				//#####################################################################################################################
				//#####################################################################################################################

				// send_data_USB(TxBuff8, LEN_TX_FRAME);

	#ifdef STM32F40_41xxx
				try_send_data_USART3(TxBuff8, LEN_TX_FRAME);
	#endif

	// 		}
	// 		bufend -= LEN_RX_FRAME;
	// 	}
	// }
}

