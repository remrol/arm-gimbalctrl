#ifndef CALCULATIONS_H
#define CALCULATIONS_H

int expo (int input, unsigned char percent);
void mix(int proportional, int differential, int* a, int* b);
void slowdown (int input, int* output, int increment);
int scale (int input, unsigned int percent);
void limit (int *output, int positive, int negative);

#endif // CALCULATIONS_H
