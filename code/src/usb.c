#include <stdint.h>
#include <string.h>

#include "usbd_cdc_vcp.h"

#include "basic_functions.h"
#include "periph_functions.h"


typedef float tFxx;


extern uint8_t  APP_Rx_Buffer [];
extern uint32_t APP_Rx_ptr_in;

volatile float timPassedInUs, timPassedMaxInUs;

// Dummy
void RefCurrentCalculation(tFxx* IdqRef, tFxx* MrefLim, tFxx Mref, tFxx omEl, const tFxx* p)
{
	(void)omEl;
	(void)p[0];
	IdqRef[0] = -15.f;
	IdqRef[1] = 20.f;
	*MrefLim = Mref;
}

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

uint8_t check_header(char *buf){

	char header[5];
	memcpy(header, buf, 4);
	header[4] = '\0';

	if(!strcmp(header, "HEAD")){
		return 1;
	}
	return 0;
}

#define LEN_TX_FRAME (16+1)*4 
#define LEN_RX_FRAME (12+1)*4
#define buflen 200
char BufArr[buflen];
char * Buf;
uint32_t bufend = 0;

void process_received_data(char* BufIn, uint32_t Len){

	Buf = BufArr;
	uint32_t i = 0;
	for(i = 0; i < Len; i++)
	{
		Buf[bufend] = BufIn[i];
		bufend++;
	}

	if(bufend >= LEN_RX_FRAME)
	{
		uint32_t idx = 0;

		if(1) // check_header(&Buf[idx])){
		{	
			tFxx IdqRef[2], MrefLim[2], Mref, omEl, p[7], Imax, Uzk, Rs, Ld, Lq, PsiP, Np, reluk, Ureserve;

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
			p[0] = (tFxx)Np;
			p[1] = (tFxx)Rs;
			p[2] = (tFxx)Ld;
			p[3] = (tFxx)Lq;
			p[4] = (tFxx)PsiP;
			p[5] = (tFxx)Imax;
			p[6] = (tFxx)Uzk;

			startTimeMeas(); // log timestamp at start
			RefCurrentCalculation(IdqRef, MrefLim, Mref, omEl, (const tFxx*)&p); // call function-under-test
			stopTimeMeas((float*)&timPassedInUs, (float*)&timPassedMaxInUs); // log time directly after end of function

			float testVar = 123.4;
			uint8_t TxBuff8[LEN_TX_FRAME];
			strcpy((char*)TxBuff8, "HEAD");
			uint32_t * TxBuff32;
			TxBuff32 = (uint32_t*)TxBuff8;
			memcpy((char*)&TxBuff32[1], (char*)&IdqRef[0], 4);
			memcpy((char*)&TxBuff32[2], (char*)&IdqRef[1], 4);
			memcpy((char*)&TxBuff32[3], (char*)&MrefLim, 4);
			memcpy((char*)&TxBuff32[4], (char*)&pt32[11], 4); // Timestamp is sent back (loopback)
			memcpy((char*)&TxBuff32[5], (char*)&timPassedInUs, 4);
			memcpy((char*)&TxBuff32[6], (char*)&testVar, 4);
			send_data_USB(TxBuff8, LEN_TX_FRAME);
		}
		bufend = 0;
	}
}

