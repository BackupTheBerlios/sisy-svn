/* #include <stdio.h> */

int table_sin[16] = {0, 3196, 6270, 9102, 11585, 13623, 15137, 16069, 16384};

/* int */
/* osc_form(unsigned int phase)//phase is normalized at (1<<14) */
/* { */
/*     int index = (phase & 0xfff) >> 14; */
/*     switch((phase & 0x4000) >> 12) */
/* 	{ */
/* 	case 0: return table_form[index]; */
/* 	case 1: return table_form[8-index]; */
/* 	case 2: return -table_form[index]; */
/* 	case 3: return -table_form[8-index]; */
/* 	} */
/*     fprintf("osc_form: On a rien a foutre ici !!!"); */
/*     return 0; */
/* } */

int
osc_sin(unsigned int phase)//phase is normalized at (1<<14)
{
    int index = (phase & (0x7<<9)) >> 9;
    int case_index = (phase & (0x3<<12)) >> 12;
    //printf("phase: %x, index: %x, case: %x, ", phase, index, case_index);
    switch(case_index)
	{
	case 0: return table_sin[index];
	case 1: return table_sin[8-index];
	case 2: return -table_sin[index];
	case 3: return -table_sin[8-index];
	}
    //fprintf(stderr, "osc_sin: On a rien a foutre ici !!!");
    return 0;
}

/* int main(int ac, char **av) */
/* { */
/*     int i, n=32; */
/*     for(i=-n; i<n; i++) */
/* 	printf("sin(%f): %f\n", 2*3.141592*i/n, (float) osc_sin(i*(1<<14)/n) / (1<<14)); */
/* } */
