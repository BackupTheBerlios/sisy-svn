#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "../oscillators.h"
#include "vibro.h"

static int destroy (module_vibro_t*);
static int process (module_vibro_t*, int);
static module_t *clone (module_vibro_t*);


struct module_vibro_class_s {
    module_c module;
};

typedef struct {
    int freq, ampl, offset, trigger, out;
} vibro_IO_t;

static symbole_t IO_symtab[]={
    {"env_out",	OFFSET(vibro_IO_t, out),	SIAD_SCOPE_OUT, SIAD_TYPE_VALUE},
    {"trigger",	OFFSET(vibro_IO_t, trigger),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"freq",	OFFSET(vibro_IO_t, freq),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"ampl",	OFFSET(vibro_IO_t, ampl),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"offset",	OFFSET(vibro_IO_t, offset),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};

obj_c*
module_vibro_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(module_vibro, module);
	    class->destroy = (obj_destroy_t*)destroy;
	    MODULE_CLASS(class)->process = (module_process_t*)process;
	    MODULE_CLASS(class)->clone = (module_clone_t*)clone;
	}

    return class;
}


struct module_vibro_s {
    module_t module;
    vibro_IO_t IO;
    int ampl, freq, offset, phase;
};


//A: time to reach the note level
//D: time to reach the sustain level
//S: Sustain level per the note level (1024=note level, 0=0)
//R: Time to reach the level 0 per the note off ref


/* static int */
/* vibro_null(module_vibro_t *vibro) */
/* { */
/*     vibro->dest = 0;//Just in case we dont come from decay. */
/*     vibro->ttl = 1000000000; */
/*     vibro->state = vibro_STATE_NULL; */
/*     return 0; */
/* } */


static int
trigger_callback(module_vibro_t *vibro)
{
    vibro->phase = 0;
    return 0;
}


static int
freq_callback(module_vibro_t *vibro)
{
    printf("vibro: freq_callback: %d\n", vibro->IO.freq);
    vibro->freq = vibro->IO.freq;
    return 0;
}


static int
ampl_callback(module_vibro_t *vibro)
{
    vibro->ampl = vibro->IO.ampl;
    return 0;
}

static int
offset_callback(module_vibro_t *vibro)
{
    vibro->offset = vibro->IO.offset;
    return 0;
}


module_t*
module_vibro_create()
{
    module_vibro_t * vibro=0;

    vibro = obj_create(module_vibro);
    MODULE(vibro)->IO.name = "vibro IO";
    MODULE(vibro)->IO.size = sizeof(vibro_IO_t);
    MODULE(vibro)->IO.data = &vibro->IO;
    MODULE(vibro)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(vibro)->IO, "freq", (siad_callback_t)freq_callback,  vibro) < 0);
    ck_err(bank_add_watch(&MODULE(vibro)->IO, "ampl", (siad_callback_t)ampl_callback,  vibro) < 0);
    ck_err(bank_add_watch(&MODULE(vibro)->IO, "offset", (siad_callback_t)offset_callback,  vibro) < 0);
    ck_err(bank_add_watch(&MODULE(vibro)->IO, "trigger", (siad_callback_t)trigger_callback,  vibro) < 0);

    return MODULE(vibro);
  error:
    return 0;
}


module_t*
clone(module_vibro_t *vibro)
{
    return module_vibro_create();
}


int
process(module_vibro_t *vibro, int size)
{
    vibro->phase += size;

    if(vibro->freq)
	{
	    vibro->IO.out = vibro->offset + (vibro->ampl >> 7) * (osc_sin((vibro->phase>>7)*(vibro->freq>>7)) >> 7);
	    if(vibro->IO.out<0)
		vibro->IO.out = -vibro->IO.out;
	}
    return 0;
}


int
destroy(module_vibro_t *vibro)
{
    printf("destroy\n");
    return 0;
}
