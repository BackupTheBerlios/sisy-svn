#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "../math.h"
#include "DFOF.h"


static int destroy (module_DFOF_t*);
static int process (module_DFOF_t*, int);
static module_t *clone (module_DFOF_t*);


struct module_DFOF_class_s {
    module_c module;
};


typedef struct {
    s32_t Q, f0;
    buffer_t *in, *out;
} DFOF_IO_t;


static symbole_t IO_symtab[]={
    {"AI",	OFFSET(DFOF_IO_t, in),	SIAD_SCOPE_IN,  SIAD_TYPE_BUFFER},
    {"AO",	OFFSET(DFOF_IO_t, out),	SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {"f0",	OFFSET(DFOF_IO_t, f0),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"Q",	OFFSET(DFOF_IO_t, Q),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};


static obj_c*
module_DFOF_class()
{
    static obj_c *class=0;
    if(!class) {
	create_class(module_DFOF, module);
	class->destroy = (obj_destroy_t*)destroy;
	MODULE_CLASS(class)->process = (module_process_t*)process;
	MODULE_CLASS(class)->clone = (module_clone_t*)clone;
    }
    return class;
}


struct module_DFOF_s {
    module_t module;
    DFOF_IO_t IO;
    float Q, f0, fb, buf0, buf1;
};

static int
coeficients(module_DFOF_t *DFOF)
{
    DFOF->fb = DFOF->Q + DFOF->Q/(1.0 - DFOF->f0);
    return 0;
}


static int
Q_callback(module_DFOF_t *DFOF)
{
    DFOF->Q = fixed2float(DFOF->IO.Q, 15);
    printf("Q_callback: %f", DFOF->Q);
    coeficients(DFOF);
    return 0;
}


static int
f0_callback(module_DFOF_t *DFOF)
{
    DFOF->Q = fixed2float(DFOF->IO.Q, 15);
    coeficients(DFOF);
//  error:
    return 0;
}


module_t*
module_DFOF_create()
{
    module_DFOF_t * DFOF=0;

    DFOF = obj_create(module_DFOF);
    MODULE(DFOF)->IO.name = "DFOF IO";
    MODULE(DFOF)->IO.size = sizeof(DFOF_IO_t);
    MODULE(DFOF)->IO.data = &DFOF->IO;
    MODULE(DFOF)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(DFOF)->IO, "Q",  (siad_callback_t)Q_callback,   DFOF) < 0);
    ck_err(bank_add_watch(&MODULE(DFOF)->IO, "f0", (siad_callback_t)f0_callback,  DFOF) < 0);

    return MODULE(DFOF);
  error:
    return 0;
}


static module_t*
clone(module_DFOF_t *DFOF)
{
    return module_DFOF_create();
}


static int
process(module_DFOF_t *module, const int size)
{
    smpl_t *in, *out;
    int i;

    in  = module->IO.in->smpl;
    out = module->IO.out->smpl;

    for(i=0; i<size; i++)
	{
	    module->buf0 += module->f0 * (in[i] - module->buf0 + module->fb * (module->buf0 - module->buf1));
	    out[i] = (module->buf1 += module->f0 * (module->buf0 - module->buf1));
	}

    return 0;
}


static int
destroy(module_DFOF_t *DFOF)
{
 printf("destroy\n");
    return 0;
}
