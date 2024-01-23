#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "misc.h"

#include "string.h"

#include "usart3.h"

// #include "typedef.h"
// #include "gpio.h"
// #include "usart3.h"
// #include "communication.h"
// #include "tasks.h"

#include "usb.h"

// GPIO B
#define GPIO_PORT_USART3_TX		GPIOB
#define GPIO_PIN_USART3_TX		GPIO_Pin_10
#define GPIO_AFPIN_USART3_TX    GPIO_PinSource10

#define GPIO_PORT_USART3_RX		GPIOB
#define GPIO_PIN_USART3_RX		GPIO_Pin_11
#define GPIO_AFPIN_USART3_RX    GPIO_PinSource11


/* TX Buffer */
volatile uint8_t txBuff_USART3_aui8[N_DATA_VALS_TX_USART3];
#define N_DATA_VALS_TX_DMA_USART3_MEM TX_BYTES_PER_MSG
static uint8_t txDmaBuff_USART3_aui8[N_DATA_VALS_TX_DMA_USART3_MEM];
static uint8_t flag_TX_busy_USART3 = 0;

/* RX Buffer */
#define N_DATA_VALS_DMA_USART3_MEM LEN_RX_FRAME
static uint8_t rxDmaBuff_USART3_aui8[N_DATA_VALS_DMA_USART3_MEM];

static uint8_t rxBuff0_USART3_aui8[N_DATA_VALS_DMA_USART3_MEM];
static uint8_t rxBuff1_USART3_aui8[N_DATA_VALS_DMA_USART3_MEM];
static uint8_t * activeRxBuff_USART3_pui8 = NULL;
static uint8_t * inactiveRxBuff_USART3_pui8 = NULL;

volatile uint32_t nrBytesRcvTotal = 0;
volatile uint32_t nrBytesSentTotal = 0;

/* RX DMA operations */
#define PAUSE_USART3_RX_DMA while(DISABLE != DMA_GetCmdStatus(DMA1_Stream1)){ DMA_Cmd(DMA1_Stream1, DISABLE); }
#define ENABLE_USART3_RX_DMA while(ENABLE != DMA_GetCmdStatus(DMA1_Stream1)){ DMA_Cmd(DMA1_Stream1, ENABLE); }
#define resetCounter_USART3_RX_DMA DMA_SetCurrDataCounter(DMA1_Stream1, N_DATA_VALS_DMA_USART3_MEM)

#define PAUSE_USART3_TX_DMA while(DISABLE != DMA_GetCmdStatus(DMA1_Stream3)){ DMA_Cmd(DMA1_Stream3, DISABLE); }
#define ENABLE_USART3_TX_DMA while(ENABLE != DMA_GetCmdStatus(DMA1_Stream3)){ DMA_Cmd(DMA1_Stream3, ENABLE); }



void init_GPIO_USART3()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USART3_TX;
	GPIO_Init(GPIO_PORT_USART3_TX, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIO_PORT_USART3_TX, GPIO_AFPIN_USART3_TX, GPIO_AF_USART3);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USART3_RX;
	GPIO_Init(GPIO_PORT_USART3_RX, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIO_PORT_USART3_RX, GPIO_AFPIN_USART3_RX, GPIO_AF_USART3);

}

static void clear_RX_DMA_flags()
{
	// FlagStatus flag_TCIF1 = DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_TCIF1);
	// FlagStatus flag_HTIF1 = DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_HTIF1);
	// FlagStatus flag_TEIF1 = DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_TEIF1);
	// FlagStatus flag_DMEIF1 = DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_DMEIF1);
	// FlagStatus flag_FEIF1 = DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_FEIF1);
    DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1);
    DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_HTIF1);
    DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TEIF1);
    DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_DMEIF1);
    DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_FEIF1);
}

static void clear_TX_DMA_flags()
{
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF4);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_HTIF4);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TEIF4);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_DMEIF4);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_FEIF4);
}


void init_USART3()
{
    activeRxBuff_USART3_pui8 = rxBuff0_USART3_aui8;

    // 1) enable clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    USART_Cmd(USART3, DISABLE);

    // optional step for oversampling
    USART_OverSampling8Cmd(USART3, ENABLE);

    // 2) GPIO clocks are enabled in gpio.c

    // 3) configure GPIOs
    init_GPIO_USART3();
    
    // 4) program USART
    USART_InitTypeDef usartInit;
    usartInit.USART_BaudRate = 256000;
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usartInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartInit.USART_StopBits = USART_StopBits_1;
    usartInit.USART_WordLength = USART_WordLength_8b;
    usartInit.USART_Parity = USART_Parity_No;
    USART_Init(USART3, &usartInit);

    // 5.2) NVIC: dma interrupt   RX
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //(DMA1_Stream1, DMA_IT_TC, ENABLE); // transfer complete

    // 7) Enable USART
    USART_Cmd(USART3, ENABLE);

}

void init_USART3_w_DMA()
{
    // 1) enable clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    // disable everything
    PAUSE_USART3_RX_DMA;
    PAUSE_USART3_TX_DMA;

    USART_Cmd(USART3, DISABLE);

	USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);
    
    DMA_DeInit(DMA1_Stream1);
    DMA_DeInit(DMA1_Stream3);

    // optional step for oversampling
    USART_OverSampling8Cmd(USART3, ENABLE);

    // 2) GPIO clocks are enabled in gpio.c

    // 3) configure GPIOs
    init_GPIO_USART3();
    
    // 6.1) DMA init RX
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_Channel = DMA_Channel_4;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)rxDmaBuff_USART3_aui8;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_BufferSize = N_DATA_VALS_DMA_USART3_MEM;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = 	DMA_MemoryInc_Enable; // DMA_MemoryInc_Disable
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte; //DMA_MemoryDataSize_HalfWord; // DMA_MemoryDataSize_Byte DMA_MemoryDataSize_Word
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal; // DMA_Mode_Circular DMA_Mode_Normal
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_Init(DMA1_Stream1, &DMA_InitStruct);
    
    // 6.1) DMA init TX
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_Channel = DMA_Channel_4;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)txDmaBuff_USART3_aui8;
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize = N_DATA_VALS_TX_DMA_USART3_MEM;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = 	DMA_MemoryInc_Enable; // DMA_MemoryInc_Disable
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte; //DMA_MemoryDataSize_HalfWord; // DMA_MemoryDataSize_Byte DMA_MemoryDataSize_Word
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal; // DMA_Mode_Circular DMA_Mode_Normal
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_Init(DMA1_Stream3, &DMA_InitStruct);

    // 4) program USART
    USART_InitTypeDef usartInit;
    usartInit.USART_BaudRate = 256000;
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usartInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartInit.USART_StopBits = USART_StopBits_1;
    usartInit.USART_WordLength = USART_WordLength_8b;
    usartInit.USART_Parity = USART_Parity_No;
    USART_Init(USART3, &usartInit);

    // 5.1) no synchronous mode --> no clock needed
    
    // 5.2) NVIC: dma interrupt   RX
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_USART3_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA_USART3_SUBPRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    // 5.2) NVIC: dma interrupt TX
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA_USART3_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = DMA_USART3_SUBPRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    // 6.2) Run USART_DMA Cmd
	USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
    
    clear_RX_DMA_flags();
    clear_TX_DMA_flags();

    // 8) Enable DMA Rx
    ENABLE_USART3_RX_DMA;
    // ENABLE_USART3_TX_DMA;

    // DMA Interrupts
    DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE); // transfer complete
    DMA_ITConfig(DMA1_Stream1, DMA_IT_HT, ENABLE); // half transfer

    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE); // transfer complete
 

    // 7) Enable USART
    USART_Cmd(USART3, ENABLE);

}

uint8_t try_send_data_USART3(uint8_t * data, uint32_t NBytes)
{
    /* without DMA */
    uint32_t i;
    for(i = 0; i < NBytes; i++)
    {
        while(SET != USART_GetFlagStatus(USART3, USART_FLAG_TXE))
        {
            // waitUs(100);
        }
        USART_SendData(USART3, (uint16_t)data[i]);        
    }
    
    return 1;
}

uint8_t try_send_data_USART3_DMA(uint8_t * data, uint32_t NBytes)
{
    /* without DMA */
    // uint32_t i;
    // for(i = 0; i < NBytes; i++)
    // {
    //     while(SET != USART_GetFlagStatus(USART3, USART_FLAG_TXE));
    //     USART_SendData(USART3, (tUI16)data[i]);        
    // }


    /* with DMA */
    // uint32_t i;
    // for(i = 0; i < NBytes; i++)
    // {
    //     txDmaBuff_USART3_aui8[idx_txDmaBuff_USART3_ui32] = data[i];
    //     if(idx_txDmaBuff_USART3_ui32 >= N_DATA_VALS_TX_DMA_USART3_MEM)
    //     {
    //         idx_txDmaBuff_USART3_ui32 = 0;
    //     }
    // }
    // idx_txDmaBuff_USART3_ui32 = 0;
    uint8_t success_b = 0;
    if(0 == flag_TX_busy_USART3)
    {
        memcpy(txDmaBuff_USART3_aui8, data, NBytes);

        clear_TX_DMA_flags();
        // DMA_SetCurrDataCounter(DMA1_Stream3, N_DATA_VALS_TX_DMA_USART3_MEM);
        flag_TX_busy_USART3 = 1;
        ENABLE_USART3_TX_DMA;

        success_b = 1;
    }
    return success_b;
}

// void send_USART3_wrapper(uint8_t data)
// {
//     while(SET != USART_GetFlagStatus(USART3, USART_FLAG_TXE));
//     USART_SendData(USART3, data);
// }

// void send_byte_USART3(uint8_t * data, uint16_t NDataBytes)
// {
//     send_USART3_wrapper('H');
//     send_USART3_wrapper('E');
//     send_USART3_wrapper('A');
//     send_USART3_wrapper('D');

//     uint16_t i;
//     uint16_t dataTmp;
//     for(i = 0; i < NDataBytes; i++)
// 	{
//         dataTmp = (uint16_t)data[i];
//         send_USART3_wrapper(dataTmp);
//     }
    
//     send_USART3_wrapper('F');
//     send_USART3_wrapper('O');
//     send_USART3_wrapper('O');
//     send_USART3_wrapper('T');
// }

// void send_USART3()
// {
//     send_USART3_wrapper('H');
//     send_USART3_wrapper('E');
//     send_USART3_wrapper('A');
//     send_USART3_wrapper('D');

//     uint16_t i, j;
//     uint16_t dataTmp;
//     uint8_t * pt;
//     for(i = 0; i < N_DATA_VALS_TX_USART3; i++)
// 	{
//         pt = (uint8_t*)(&txBuff_USART3_aui8[i]);
//         for(j = 0; j < 4; j++)
//         {
//             dataTmp = (uint16_t)*pt;
//             send_USART3_wrapper(dataTmp);
//             pt++;
//         }
//     }
    
//     send_USART3_wrapper('F');
//     send_USART3_wrapper('O');
//     send_USART3_wrapper('O');
//     send_USART3_wrapper('T');
// }

uint32_t usart3_NbytesRcv = 0;

void read_USART3()
{
    // while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE))
    // {
    //     USART_SendData(USART3, USART_ReceiveData(USART3));
    // }
    // return;

    if(usart3_NbytesRcv >= N_DATA_VALS_DMA_USART3_MEM)
    {
        process_received_data((char*)&activeRxBuff_USART3_pui8[0], N_DATA_VALS_DMA_USART3_MEM);
        usart3_NbytesRcv = 0;
    }
}


// void read_USART3()
// {
//     if(activeRxBuff_USART3_pui8 == NULL) /* initialization; TODO: move at better place */
//     {
//         activeRxBuff_USART3_pui8 = rxBuff0_USART3_aui8;
//         inactiveRxBuff_USART3_pui8 = rxBuff1_USART3_aui8;
//     }

//     PAUSE_USART3_RX_DMA;
//     const uint32_t nrValsRcv = N_DATA_VALS_DMA_USART3_MEM - DMA_GetCurrDataCounter(DMA1_Stream1); /* Note: Calculation this way because DMA data counter counts backwards */
//     nrBytesRcvTotal += nrValsRcv;
//     // if(0 < nrValsRcv)
//     // {
//     //     memcpy(activeRxBuff_USART3_pui8, rxDmaBuff_USART3_aui8, nrValsRcv);
//     // }
    
//     if(nrValsRcv >= N_DATA_VALS_DMA_USART3_MEM)
//     {
//         process_received_data(rxDmaBuff_USART3_aui8, N_DATA_VALS_DMA_USART3_MEM);        
//         resetCounter_USART3_RX_DMA;
//     }

//     clear_RX_DMA_flags();
//     ENABLE_USART3_RX_DMA;
        
//     resetCounter_USART3_RX_DMA;

//     if(nrValsRcv > 0)
//     {
//         resetCounter_USART3_RX_DMA;
//         clear_RX_DMA_flags();
//     }
    
//     return;
    
// 	if(0 < nrValsRcv) /* not really needed as long as only called in e.g. RX DMA ISR */
//     {

//         // Loopback:
//         // try_send_data_USART3((uint8_t*)&activeRxBuff_USART3_pui8[0], nrValsRcv);
//         // return;

//         /* Loop through received data */
//         uint32_t sidx = 0;
//         uint32_t eidx = 0;
//         for(eidx = 0; eidx < nrValsRcv; )
//         {
//             if(activeRxBuff_USART3_pui8[eidx] == '\0')
//             {
//                 process_received_data((char*)&activeRxBuff_USART3_pui8[sidx], eidx - sidx + 1);
//                 sidx = eidx+1;
//                 eidx = sidx;
//             }
//             else
//             {
//                 eidx++;
//             }
//         }
//         const uint32_t nrValsRemaining = eidx - sidx;

//         /* Copy remaining values from active buffer to inactive buffer before switching buffers */
//         memcpy(inactiveRxBuff_USART3_pui8, &activeRxBuff_USART3_pui8[eidx], nrValsRemaining);

//         /* Switch buffer */
//         uint8_t * tmpRxBuff_pui8 = activeRxBuff_USART3_pui8;
//         activeRxBuff_USART3_pui8 = inactiveRxBuff_USART3_pui8;
//         inactiveRxBuff_USART3_pui8 = tmpRxBuff_pui8;
//     }
// }


void USART3_IRQHandler()
{
    // waitUs(100);
    // while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE))
    // {
        activeRxBuff_USART3_pui8[usart3_NbytesRcv] = (uint8_t)USART_ReceiveData(USART3);
        usart3_NbytesRcv++;
    // }
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}

void DMA1_Stream1_IRQHandler()
{
    DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_HTIF1); // ISR when half of the dma buffer is full
	DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1); // ISR when the transfer is complete
}

void DMA1_Stream3_IRQHandler()
{
    PAUSE_USART3_TX_DMA;
    nrBytesSentTotal += N_DATA_VALS_TX_DMA_USART3_MEM;
	// DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3);
	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3); // ISR when the transfer is complete
    clear_TX_DMA_flags();
    flag_TX_busy_USART3 = 0;
}