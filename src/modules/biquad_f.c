#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "../math.h"
#include "biquad_f.h"
#include <math.h>

#define PI (3.141592)

static int destroy (module_biquad_f_t*);
static int process (module_biquad_f_t*, int);
static module_t *clone (module_biquad_f_t*);

struct module_biquad_f_class_s {
    module_c module;
};

typedef struct {
    int Q, f0;
    buffer_t *in, *out;
} biquad_f_IO_t;

static symbole_t IO_symtab[]={
    {"AI",	OFFSET(biquad_f_IO_t, in),	SIAD_SCOPE_IN,  SIAD_TYPE_BUFFER},
    {"AO",	OFFSET(biquad_f_IO_t, out),	SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {"f0",	OFFSET(biquad_f_IO_t, f0),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"Q",	OFFSET(biquad_f_IO_t, Q),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {0, 0, 0}
};

static obj_c*
module_biquad_f_class()
{
    static obj_c *class=0;
    if(!class) {
	create_class(module_biquad_f, module);
	class->destroy = (obj_destroy_t*)destroy;
	MODULE_CLASS(class)->process = (module_process_t*)process;
	MODULE_CLASS(class)->clone = (module_clone_t*)clone;
    }
    return class;
}


struct module_biquad_f_s {
    module_t module;
    biquad_f_IO_t IO;
    float Q, f0;
    float sin_f0, cos_f0, alpha;
    float A1, A2, B0, B1, B2;
    float w0, QQ;
    float o1, o2,     i1, i2;
};

#define LPF

static void
coeficients(module_biquad_f_t *bq)
{
/*     int a0; */
    float alpha, A0, w0;
//    s32_t a0;

    w0 = 2.0 * PI * bq->f0;
//    printf("w0: %f\t", w0);
//    printf("Q: %f\t", bq->Q);
    alpha  = sin(w0) / (2.0 * bq->Q);
//    printf("alpha: %f\t", alpha);

    A0     =  (1.0 + alpha);
    bq->A1 = -(2.0 * cos(w0)) / (A0);
    bq->A2 =  (1.0 - alpha)   / (A0);

#ifdef LPF
    bq->B0 =  (1.0 - cos(w0)) / (2.0 * A0);
    bq->B1 =  (1.0 - cos(w0)) / (A0);
    bq->B2 =  (1.0 - cos(w0)) / (2.0 * A0);
#else
#ifdef HPF
    bq->B0 =  (1 + cos(w0)) / (2 * A0);// [0:1]
    bq->B1 = -(1 + cos(w0)) / (    A0);// [-2:0]
    bq->B2 =  (1 + cos(w0)) / (2 * A0);// [0:1]
#else
#ifdef BPF
    bq->B0 =   sin(w0) / (2 * A0);//  =   Q*alpha [0:0.5]
    bq->B1 =   0;
    bq->B2 =  -sin(w0) / (2 * A0);// =  -Q*alpha [-0.5:0]
#endif
#endif
#endif

//    bq->alpha = bq->sin_f0/(2*bq->IO.Q);//.20

/*     printf("freq: %x, ", bq->IO.f0); */
/*     print_fixed(bq->IO.f0, 30); */
/*     printf(",\tQ: %x, ", bq->IO.Q); */
/*     print_fixed(bq->IO.Q, 10); */
/*     printf(",\tsin: "); */
/*     print_fixed(bq->sin_f0, 30); */
/*     printf(",\tcos: "); */
/*     print_fixed(bq->cos_f0, 30); */
/*     printf(",\talpha: "); */
/*     print_fixed(bq->alpha, 20); */
/*     printf("\n"); */


/*     a0 = ((1<<20) + (bq->alpha));//20 */

/*     bq->a1 = ((-2 * bq->cos_f0)) / (a0>>10);//20 */
/*     bq->a2 = (((1<<20) - bq->alpha)<<8) / (a0>>12); */

/*     bq->b0 = ((1<<30) - ((bq->cos_f0))) / (2 * (a0>>10)); */
/*     bq->b1 = ((1<<30) - ((bq->cos_f0))) / (a0>>10); */
/*     bq->b2 = ((1<<30) - ((bq->cos_f0))) / (2 * (a0>>10)); */

/*     bq->A1 = fixed2float(bq->a1, 20); */
/*     bq->A2 = fixed2float(bq->a2, 20); */
/*     bq->B0 = fixed2float(bq->b0, 20); */
/*     bq->B1 = fixed2float(bq->b1, 20); */
/*     bq->B2 = fixed2float(bq->b2, 20); */

/*     printf("A0: %f\t", A0); */
/*     printf("A1: %f\t", bq->A1); */
/*     printf("A2: %f\t", bq->A2); */
/*     printf("B0: %f\t", bq->B0); */
/*     printf("B1: %f\t", bq->B1); */
/*     printf("B2: %f\n", bq->B2); */
}

static int
Q_callback(module_biquad_f_t *bq)
{
    bq->Q = fixed2float(bq->IO.Q, 15);
    coeficients(bq);
    return 0;
}

static int
f0_callback(module_biquad_f_t *bq)
{
    bq->f0 = fixed2float(bq->IO.f0, 15);
    if(bq->f0 > 0.5)
	bq->f0 = 0.5;

/*     printf("F0: %f\t", bq->f0); */
/*     printf("cos_F0: %f\t", cos(2.0 * PI * bq->f0)); */
/*     printf("sin_F0: %f\n", sin(2.0 * PI * bq->f0)); */

    coeficients(bq);
    return 0;
}

module_t*
module_biquad_f_create()
{
    module_biquad_f_t * biquad_f=0;

    biquad_f = obj_create(module_biquad_f);
    MODULE(biquad_f)->IO.name = "biquad_f IO";
    MODULE(biquad_f)->IO.size = sizeof(biquad_f_IO_t);
    MODULE(biquad_f)->IO.data = &biquad_f->IO;
    MODULE(biquad_f)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(biquad_f)->IO, "Q",  (siad_callback_t)Q_callback,  biquad_f) < 0);
    ck_err(bank_add_watch(&MODULE(biquad_f)->IO, "f0", (siad_callback_t)f0_callback,  biquad_f) < 0);

    return MODULE(biquad_f);
  error:
    return 0;
}

static module_t*
clone(module_biquad_f_t *biquad_f)
{
    return module_biquad_f_create();
}


static int
process(module_biquad_f_t *biquad_f, const int size)
{
    module_biquad_f_t bq;
    smpl_t *in, *out;
    int i;

    in  = biquad_f->IO.in->smpl;
    out = biquad_f->IO.out->smpl;

    bq=*biquad_f;

    for(i=0; i<size; i++)
	{
	    float i0, o0;
	    i0 = in[i];
	    o0 = (bq.B0 * i0 + bq.B1 * bq.i1 + bq.B2 * bq.i2 - bq.A1 * bq.o1 - bq.A2 * bq.o2);
	    bq.i2=bq.i1;
	    bq.i1=i0;
	    bq.o2=bq.o1;
	    bq.o1=o0;
	    out[i] = o0;
	}

    *biquad_f=bq;

    return 0;
}


static int
destroy(module_biquad_f_t *biquad_f)
{
    printf("destroy\n");
    return 0;
}
