extern volatile uint8_t UsrBtn_Trigger;
extern volatile uint32_t timStart;
extern volatile float timPassedMaxInUsGlobal;

void initGPIO();
void initUserButton();
void initTIM1();
void initADC();
void init_GPIO_USB();

void startTimeMeas();
void stopTimeMeas(float * timPassedInUs, float * timPassedMaxInUs);
