#ifndef _SISY_MATH_H_
#define _SISY_MATH_H_

#include "misc.h"

void init_tab ();

u32_t note2freq(u8_t note);//normalised frequence << 31
u32_t note2smpl(u8_t note);//period in samples

s32_t fixed_cos(u32_t a);//a is an angle between 0 and 2 PI contained in a a value beetween 0 and 0xffffffff
s32_t fixed_sin(u32_t a);//a is an angle between 0 and 2 PI contained in a a value beetween 0 and 0xffffffff

void print_fixed(s32_t fixed, u8_t point);
float fixed2float(s32_t fixed, u8_t point);

#endif
