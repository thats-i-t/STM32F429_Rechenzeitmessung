
#define LEN_TX_FRAME ((16+1)*4) // = 68
#define LEN_RX_FRAME ((12+1)*4) // = 52

extern volatile float timPassedInUs, timPassedMaxInUs;

void process_received_data(char* BufIn, uint32_t Len);
