#ifndef CALCULATIONS_H
#define CALCULATIONS_H

#include <stdint.h>

uint16_t expo (uint16_t pulse_time_ms, uint8_t percent);
void mix(int proportional, int differential, int* a, int* b);
void slowdown (int input, int* output, int increment);

#endif // CALCULATIONS_H
