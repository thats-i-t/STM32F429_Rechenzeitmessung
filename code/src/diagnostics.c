#include "string.h"

char* getCompileDate(){
	static char SW_date[100];
	strcat(SW_date, __DATE__);
	return SW_date;
}

char* getCompileTime(){
	static char SW_time[100];
	strcat(SW_time, __TIME__);
	return SW_time;
}

