#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

struct Debug
{
	int16_t data0;
	int16_t data1;
	int16_t data2;
	int16_t data3;
	int16_t data4;
	int16_t data5;
	int16_t data6;
	int16_t data7;
};

extern Debug g_debug;

void debug_init();

#endif // DEBUG_H


