#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "ADSR.h"

static int destroy (module_ADSR_t*);
static int process (module_ADSR_t*, int);
static module_t *clone (module_ADSR_t*);

static int ADSR_null(module_ADSR_t *ADSR);
static int ADSR_attack(module_ADSR_t *ADSR);
static int ADSR_decay(module_ADSR_t *ADSR);
static int ADSR_sustain(module_ADSR_t *ADSR);
static int ADSR_release(module_ADSR_t *ADSR);
static int ADSR_attack(module_ADSR_t *ADSR);


struct module_ADSR_class_s {
    module_c module;
};

typedef struct {
    int A, D, S, R, ref, trigger, out;
} ADSR_IO_t;

static symbole_t IO_symtab[]={
    {"env_out",	OFFSET(ADSR_IO_t, out),		SIAD_SCOPE_OUT, SIAD_TYPE_VALUE},
    {"trigger",	OFFSET(ADSR_IO_t, trigger),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"ref",	OFFSET(ADSR_IO_t, ref),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"A",	OFFSET(ADSR_IO_t, A),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"D",	OFFSET(ADSR_IO_t, D),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"S",	OFFSET(ADSR_IO_t, S),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"R",	OFFSET(ADSR_IO_t, R),		SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};

obj_c*
module_ADSR_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(module_ADSR, module);
	    class->destroy = (obj_destroy_t*)destroy;
	    MODULE_CLASS(class)->process = (module_process_t*)process;
	    MODULE_CLASS(class)->clone = (module_clone_t*)clone;
	}

    return class;
}


struct module_ADSR_s {
    module_t module;
    ADSR_IO_t IO;
    int pos, dest, ttl, state;
};


typedef enum {
    ADSR_STATE_NULL=0,
    ADSR_STATE_ATTACK,
    ADSR_STATE_DECAY,
    ADSR_STATE_SUSTAIN,
    ADSR_STATE_RELEASE,
} ADSR_state_t;


//A: time to reach the note level
//D: time to reach the sustain level
//S: Sustain level per the note level (1024=note level, 0=0)
//R: Time to reach the level 0 per the note off ref


static int
ADSR_null(module_ADSR_t *ADSR)
{
    ADSR->dest = 0;//Just in case we dont come from decay.
    ADSR->ttl = 1000000000;
    ADSR->state = ADSR_STATE_NULL;
    return 0;
}


static int
ADSR_attack(module_ADSR_t *ADSR)
{
    ADSR->dest = ADSR->IO.ref;
    ADSR->ttl  = ADSR->IO.A;
    ADSR->state = ADSR_STATE_ATTACK;
    return 0;
}


static int
ADSR_decay(module_ADSR_t *ADSR)
{
    ADSR->dest  = (ADSR->IO.S * ADSR->IO.ref) >> 15;
    ADSR->ttl   = ADSR->IO.D;
    ADSR->state = ADSR_STATE_DECAY;
    return 0;
}


static int
ADSR_sustain(module_ADSR_t *ADSR)
{
    ADSR->dest  = (ADSR->IO.S * ADSR->IO.ref) >> 15;//Just in case we dont come from decay.
    ADSR->ttl   = 1000000000;
    ADSR->state = ADSR_STATE_SUSTAIN;
    return 0;
}


static int
ADSR_release(module_ADSR_t *ADSR)
{
    ADSR->dest  = 0;
    ADSR->ttl   = ADSR->IO.R;
    ADSR->state = ADSR_STATE_RELEASE;
    return 0;
}


static int
ADSR_next_state(module_ADSR_t *ADSR)
{
    switch(ADSR->state)
	{
	case ADSR_STATE_ATTACK:
	    ADSR_decay(ADSR);
	    break;
	case ADSR_STATE_DECAY:
	    ADSR_sustain(ADSR);
	    break;
	case ADSR_STATE_RELEASE:
	    ADSR_null(ADSR);
	    break;
	case ADSR_STATE_NULL:
	case ADSR_STATE_SUSTAIN:
	    break;
	}
    return 0;
}


static int
ref_callback(module_ADSR_t *ADSR)
{
     if(ADSR->IO.trigger)
 	return ADSR_attack(ADSR);
     else
 	return ADSR_release(ADSR);
}


static int
trigger_callback(module_ADSR_t *ADSR)
{
     if(ADSR->IO.trigger)
 	return ADSR_attack(ADSR);
     else
 	return ADSR_release(ADSR);
}


module_t*
module_ADSR_create()
{
    module_ADSR_t * ADSR=0;

    ADSR = obj_create(module_ADSR);
    MODULE(ADSR)->IO.name = "ADSR IO";
    MODULE(ADSR)->IO.size = sizeof(ADSR_IO_t);
    MODULE(ADSR)->IO.data = &ADSR->IO;
    MODULE(ADSR)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(ADSR)->IO, "ref", (siad_callback_t)ref_callback,  ADSR) < 0);
    ck_err(bank_add_watch(&MODULE(ADSR)->IO, "trigger", (siad_callback_t)trigger_callback,  ADSR) < 0);

    return MODULE(ADSR);
  error:
    return 0;
}


module_t*
clone(module_ADSR_t *ADSR)
{
    return module_ADSR_create();
}


int
process(module_ADSR_t *ADSR, int size)
{
    for(;;)
	{
	    if(size >= ADSR->ttl)
		{
		    size -= ADSR->ttl;
		    ADSR->pos = ADSR->dest;
		    ADSR_next_state(ADSR);
		}
	    else
		{
		    ADSR->pos += (ADSR->dest - ADSR->pos) * size / ADSR->ttl;
		    ADSR->ttl -= size;
		    break;
		}
	}

    ADSR->IO.out = ADSR->pos;
    return 0;
}


int
destroy(module_ADSR_t *ADSR)
{
 printf("destroy\n");
    return 0;
}
