#include <math.h>
#include "math.h"
#include "audio.h"
/* w = 2 pi f */
/* wn = w / ws */

/* 0 <= w <= 2 PI */

/* int sin_table[]={0, 0x7fffffff, 0, -0x800000}; */
/* int cos_table[]={0x7fffffff, 0, -0x800000, 0}; */

//NOTE2FREQ
u32_t midi_s[128];//periode en sample
u32_t midi_f[128];//frequence / SAMPLE_RATE << 30

#define PI (3.141592)

float _midi_T[] = { 
    122.3022,
    115.4473,
    108.9378,
    102.8519,
    97.0793,
    91.6306,
    86.4878,
    81.6336,
    77.0519,
    72.7273,
    68.6454,
    64.7926
};

void init_tab ();

/* s^i */
s32_t
Powi(s32_t s, u32_t i){
    s32_t n;
    s32_t t=1;

    if(i<0){
	i=-i;
	s=1/s;
    }
    for(n=1; n<=i; n++)
	t*=s;
    return t;
}

void
init_tab ()
{
    u32_t i;
    float tmp;
    for (i = 0; i < 128; i++)
	{
	    tmp = (_midi_T[(i % 12)] / Powi (2, (i / 12)));
	    midi_s[i] = (u32_t)(tmp * (float)SAMPLE_RATE);
	    midi_f[i] = (s32_t)(((float)1000*(1<<15) / ((float)SAMPLE_RATE*tmp)));
/* 	    printf("note: %x, normalised frequency: 0x%x, ", i, midi_f[i]); */
/* 	    print_fixed(midi_f[i], 30); */
/* 	    printf(", sin:0x%x, ", fixed_sin(midi_f[i])); */
/* 	    print_fixed(fixed_sin(midi_f[i]), 30); */
/* 	    printf(", cos:0x%x, ", fixed_cos(midi_f[i])); */
/* 	    print_fixed(fixed_cos(midi_f[i]), 30); */
/* 	    printf("\n"); */
	} // tmp * (float)SAMPLE_RATE;
}

u32_t
note2freq(u8_t note)
{
    ck_err(note>127);
    return midi_f[note];
  error:
    return -1;
}

u32_t
note2smpl(u8_t note)
{
    ck_err(note>127);
    return midi_s[note];
  error:
    return -1;
}

//15 bits shifted result, 0=0, (1<<15)-1 == 2*PI
s32_t
fixed_cos(u32_t a)
{
    return (s32_t)(cos((float)a/(float)(1<<15)) * (float)((1<<15)));
}

//15 bits shifted result, 15 bits shifted input
s32_t
fixed_sin(u32_t a)
{
    return (s32_t)(sin((float)a/(float)(1<<15)) * (float)((1<<15)));
/*     return (s32_t)(sin(f) * (float)((1<<30)-1)); */
}

void
print_fixed(s32_t fixed, u8_t point)
{
    printf("%f", (float)fixed/(float)(1<<point));
}

float
fixed2float(s32_t fixed, u8_t point)
{
    return (float)fixed/(float)(1<<point);
}
