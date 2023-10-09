#include "stdint.h"
#include "tm_stm32f4_ili9341.h"

#include "basic_functions.h"


volatile uint8_t displayMode = 2;
static char disp_array[Nrows_disp][Ncols_disp];
static uint8_t newData[Nrows_disp];

#define startLine 3
#define DISP_SCROLLING
//#define DISP_START_FROM_TOP

uint8_t nextLine = startLine;

extern volatile uint8_t interrDis;


void clear_line(uint8_t line)
{
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		disp_array[line-1][i] = ' ';
	}
	disp_array[line-1][Ncols_disp-1] = '\0';
	newData[line-1] = 1;
}

void clearAllLines()
{
	uint8_t i;
	for(i = 1; i <= Nrows_disp; i++)
	{
		clear_line(i);
	}
}

void disp_char(char *name){
	static uint8_t col = 0;
	disp_array[nextLine][col] = *name;
	newData[nextLine] = 1;
	col++;
	if(col > Ncols_disp)
	{
		col = 0;
	}
}

void disp_str_nxtLn(char *name){
	clear_line(nextLine);
	disp_str(name, nextLine);
	goToNextLine();
}

void disp_fvar_nxtLn(char *name, float var){
	clear_line(nextLine);
	disp_fvar(name, var, nextLine);
	goToNextLine();
}

void disp_str(char *name, uint8_t line){

	// line = nextLine;
	clear_line(line);

	char str[Ncols_disp];
	strcpy(str, name);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;
}

void disp_var(char *name, uint32_t var, uint8_t line){

	clear_line(line);

	char str[Ncols_disp];
	strcpy(str, name);
    char var_str[Ncols_disp];
    my_itoa(var,var_str,Ncols_disp);
	strcat(str, var_str);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;
}

void disp_var_nxtLn(char *name, uint32_t var, uint8_t line){

	line = nextLine;
	clear_line(line);

	char str[Ncols_disp];
	strcpy(str, name);
    char var_str[Ncols_disp];
    my_itoa(var,var_str,Ncols_disp);
	strcat(str, var_str);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;

	goToNextLine();
}

void disp_2var(char *name, uint32_t var1, uint32_t var2, uint8_t line){

	line = nextLine;
	clear_line(line);

	char str[Ncols_disp];
	strcpy(str, name);
    char var_str1[Ncols_disp];
    my_itoa(var1,var_str1,Ncols_disp);
    char var_str2[Ncols_disp];
    itohexa(var2,var_str2,Ncols_disp);
	strcat(str, var_str1);
	strcat(str, ": ");
	strcat(str, var_str2);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;

	goToNextLine();
}

void disp_var_str(char *name, char *var, uint8_t line){

	line = nextLine;
	clear_line(line);

	char str[Ncols_disp];
	strcpy(str, name);
	strcat(str, var);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;

	goToNextLine();
}


void disp_fvar(char *name, float var, uint8_t line){

	char str[Ncols_disp];
	strcpy(str, name);
    char var_str[Ncols_disp];
	ftoa(var,var_str,Ncols_disp);
	strcat(str, var_str);
	str[Ncols_disp-1] = ' ';
	uint8_t i;
	for(i = 0; i < Ncols_disp; i++){
		if(str[i] == '\0')
		{
			break;
		}
		disp_array[line-1][i] = str[i];
	}
	newData[line-1] = 1;
}

void update_display(){

	uint16_t i;
	if(displayMode == 1){
		TM_ILI9341_Puts(20, 50, &disp_array[0][0], &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
	}
	else if(displayMode == 2){
		for(i = 0; i < Nrows_disp; i++){
			if(newData[i] != 0)
			{
				if(FONT_SIZE == 1){
					TM_ILI9341_Puts(0, 12*i, &disp_array[i][0], &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
				}
				else if(FONT_SIZE == 2){
					TM_ILI9341_Puts(0, 20*i, &disp_array[i][0], &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
				}
				newData[i] = 0;
			}
		}
	}
}

void my_itoa( uint32_t value, char *buf, int size){

		int i=size-1, offset, bytes;
		buf[i--]='\0';
		do{
			buf[i--]=(value % 10)+'0';
			value = value/10;
		}while((value > 0) && (i>=0));
		offset=i+1;
		if(offset > 0){
			bytes=size-i-1;
			for(i=0;i<bytes;i++)
				buf[i]=buf[i+offset];
		}
}

void ftoa( float value, char *buf, int size){
	uint32_t ipart = (uint16_t)value;
	my_itoa(ipart, buf, size);
	char fpart_str[size];
	float fpart = (value-(float)ipart)*1000;
	uint32_t fpart_as_int = (uint32_t)fpart;
	my_itoa(fpart_as_int, fpart_str, size);
	strcat(buf,",");
	strcat(buf,fpart_str);
}

void itohexa(uint32_t dec, char *hex, uint16_t size){
	uint32_t temp;
	uint32_t i = 0;
	do{
		temp = dec % 16;
		//To convert integer into character
		if(temp < 10){
			 temp = temp + 48;
		}
		else{
			 temp = temp + 55;
		}
		hex[i] = temp;
		dec = dec / 16;
		i++;
		if(i > size-2){
			break;
		}
	}while(dec != 0);
	//hex[i] = '\0';
	char hex_tmp[size];
	uint16_t j = 0;
	int16_t k = (int16_t)i-1;
	while(k >= 0){
		hex_tmp[j] = hex[k];
		k--;
		j++;
	}
	hex_tmp[j] = '\0';
	strcpy(hex,hex_tmp);
}


void goToNextLine(void)
{
	nextLine++;
	uint8_t i;
	uint8_t k;

#ifdef DISP_START_FROM_TOP
	if(nextLine > Nrows_disp)
	{
		nextLine = 1;
	}
#elif defined(DISP_SCROLLING)
	if(nextLine > Nrows_disp)
	{
		nextLine--;
		for(i=startLine-1; i < Nrows_disp-1; i++)
		{
			for(k=0; k < Ncols_disp; k++)
			{
				disp_array[i][k] = disp_array[i+1][k];
			}
			newData[i] = 1;
		}
	}
#endif
}

DISABLE_OPTIMIZATIONS
inline void waitUs(uint32_t us){
	uint32_t i;
	uint32_t imax = (uint32_t)((float)us * 13.979f);
	for(i = 0; i < imax; i++)
	{
		asm("nop");
	}
}

inline void waitMs(uint32_t ms){
	int i;
	for(i = 0; i < ms; i++)
	{
		waitUs(1000.f);
	}
}
REENABLE_OPTIMIZATIONS

// // disable all interrupts
// inline __attribute__((always_inline)) void disable_interrupts(void){
// 	//__asm volatile("CPSID i");
// 	interrDis = 1;
// }

// // enable all interrupts
// inline __attribute__((always_inline)) void enable_interrupts(void){
// 	//__asm volatile("CPSIE i");
// 	interrDis = 0;
// }
