#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "../math.h"
#include "biquad.h"
#include <math.h>

#define PI (3.141592)
#define FIXED_PI ((s32_t)(3.141592 * (1<<15)))

static int destroy (module_biquad_t*);
static int process (module_biquad_t*, int);
static module_t *clone (module_biquad_t*);

struct module_biquad_class_s {
    module_c module;
};

typedef struct {
    int Q, f0;
    buffer_t *in, *out;
} biquad_IO_t;

static symbole_t IO_symtab[]={
    {"AI",	OFFSET(biquad_IO_t, in),	SIAD_SCOPE_IN,  SIAD_TYPE_BUFFER},
    {"AO",	OFFSET(biquad_IO_t, out),	SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {"f0",	OFFSET(biquad_IO_t, f0),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"Q",	OFFSET(biquad_IO_t, Q),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {0, 0, 0}
};

static obj_c*
module_biquad_class()
{
    static obj_c *class=0;
    if(!class) {
	create_class(module_biquad, module);
	class->destroy = (obj_destroy_t*)destroy;
	MODULE_CLASS(class)->process = (module_process_t*)process;
	MODULE_CLASS(class)->clone = (module_clone_t*)clone;
    }
    return class;
}


struct module_biquad_s {
    module_t module;
    biquad_IO_t IO;
    s32_t Q, f0;
    s32_t sin_f0, cos_f0, alpha;
    s32_t a1, a2, b0, b1, b2;
    s32_t o1, o2,     i1, i2;
};

/* Q: [0.15:6] */

/* b0: [0:1] */
/* b1: [-2:2] */
/* b2: [-0.5:1] */

/* a0: [1:8]   |  [1:1+1/Q] */
/* a1: [-2:2] */
/* a2: [-6:1]  |  [1-1/Q:1] */

static int
coeficients(module_biquad_t *bq)
{
    s32_t a0, alpha, tmp;

    printf("Q: %f\t", fixed2float(bq->Q, 15));

    if(!(tmp=((2*bq->Q)>>5))) return -1;//10
    alpha = (bq->sin_f0<<15)/(tmp);//30/10:20
    printf("alpha: %f\t", fixed2float(alpha, 20));

    a0 = ((1<<20) + (alpha));//20
    printf("a0: %f\t", fixed2float(a0, 20));

    bq->a1 = ((-2 * bq->cos_f0)<<15) / (a0>>10);//30/10:20
    printf("a1: %f\t", fixed2float(bq->a1, 20));
    bq->a2 = (((1<<20) - alpha)<<8) / (a0>>12);//30/10:20
    printf("a2: %f\t", fixed2float(bq->a2, 20));

    bq->b0 = ((1<<30) - (bq->cos_f0<<15)) / (2 * (a0>>10));//30/10:20
    printf("b0: %f\t", fixed2float(bq->b0, 20));
    bq->b1 = ((1<<30) - (bq->cos_f0<<15)) / (a0>>10);
    printf("b1: %f\t", fixed2float(bq->b0, 20));
    bq->b2 = ((1<<30) - (bq->cos_f0<<15)) / (2 * (a0>>10));
    printf("b2: %f\n", fixed2float(bq->b0, 20));
    return 0;
}


static int
Q_callback(module_biquad_t *bq)
{
    bq->Q = bq->IO.Q;
    printf("Q_callback: %f", fixed2float(bq->IO.Q, 15));
/*     if(bq->Q < 5000) */
/* 	bq->Q = 5000; */
    coeficients(bq);
    return 0;
}


static int
f0_callback(module_biquad_t *bq)
{
//    ck_err(bq->IO.f0>(1<<10));
    bq->f0 = bq->IO.f0;
    if(bq->f0>(1<<14))
	bq->f0 = (1<<14);
    printf("f0: %f\t", fixed2float(bq->f0, 15));
    bq->cos_f0 = fixed_cos((bq->f0 * 2 * FIXED_PI) >> 15);//[-1:1]
    printf("cos_f0: %f\t", fixed2float(bq->cos_f0, 15));
    bq->sin_f0 = fixed_sin((bq->f0 * 2 * FIXED_PI) >> 15);//[0:1]
    printf("sin_f0: %f\n", fixed2float(bq->sin_f0, 15));

    coeficients(bq);
//  error:
    return 0;
}


module_t*
module_biquad_create()
{
    module_biquad_t * biquad=0;

    biquad = obj_create(module_biquad);
    MODULE(biquad)->IO.name = "biquad IO";
    MODULE(biquad)->IO.size = sizeof(biquad_IO_t);
    MODULE(biquad)->IO.data = &biquad->IO;
    MODULE(biquad)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(biquad)->IO, "Q",  (siad_callback_t)Q_callback,   biquad) < 0);
    ck_err(bank_add_watch(&MODULE(biquad)->IO, "f0", (siad_callback_t)f0_callback,  biquad) < 0);

    return MODULE(biquad);
  error:
    return 0;
}


static module_t*
clone(module_biquad_t *biquad)
{
    return module_biquad_create();
}


static int
process(module_biquad_t *biquad, const int size)
{
    module_biquad_t bq;
    smpl_t *in, *out;
    int i;

    in  = biquad->IO.in->smpl;
    out = biquad->IO.out->smpl;

    bq=*biquad;

    for(i=0; i<size; i++)
	{
	    out[i] = ((bq.b0*(in[i]>>6)) + bq.b1*bq.i1 + bq.b2*bq.i2 - bq.a1*bq.o1 - bq.a2*bq.o2)>>14;
	    bq.i2=bq.i1;//10
	    bq.i1=in[i]>>6;//10
	    bq.o2=bq.o1;//10
	    bq.o1=out[i]>>6;//10
	}

    *biquad=bq;

    return 0;
}


static int
destroy(module_biquad_t *biquad)
{
 printf("destroy\n");
    return 0;
}
