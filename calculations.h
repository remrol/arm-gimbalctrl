#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include <stdio.h>

//uint16_t expo(uint16_t pulse_time_ms, uint8_t percent);
int16_t exponent(int16_t value, int16_t maxValue, uint8_t percent);

void mix(int proportional, int differential, int* a, int* b);
void slowdown (int input, int* output, int increment);

#endif // CALCULATIONS_H
