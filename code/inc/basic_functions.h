#ifndef __BASIC_FUNCTIONS_H__
#define __BASIC_FUNCTIONS_H__

#include <stdint.h>

// Note: "#pragma xyz" can also be written as "_Pragma("xyz")"
#define DISABLE_OPTIMIZATIONS \
_Pragma("GCC push_options") \
_Pragma("GCC optimize (\"O0\")")

#define REENABLE_OPTIMIZATIONS _Pragma("GCC pop_options")


#define FONT_SIZE 1

#if FONT_SIZE==1
	#define Nrows_disp 27
	#define Ncols_disp 34
#else
	#define Nrows_disp 16
	#define Ncols_disp 22
#endif

extern volatile uint8_t displayMode;

void LCD_UsrLog(char *str);
void LCD_UsrLogChar(char *str);
void clear_line(uint8_t line);
void clearAllLines();
void disp_char(char *str);
void disp_var_nxtLn(char *name, uint32_t var, uint8_t line);
void disp_str_nxtLn(char *str);
void disp_fvar_nxtLn(char *name, float var);
void disp_str(char *name, uint8_t line);
void disp_var(char *name, uint32_t var, uint8_t line);
void disp_2var(char *name, uint32_t var1, uint32_t var2, uint8_t line);
void disp_var_str(char *name, char *var, uint8_t line);
void disp_fvar(char *name, float var, uint8_t line);
void goToNextLine(void);
void update_display(void);
void my_itoa( uint32_t value, char *buf, int size);
void itohexa(uint32_t dec, char *hex, uint16_t size);
void ftoa( float value, char *buf, int size);
uint32_t dec2hex(uint32_t dec);
void waitUs(uint32_t us);
void waitMs(uint32_t ms);
// inline void disable_interrupts(void);
// inline void enable_interrupts(void);

#endif
