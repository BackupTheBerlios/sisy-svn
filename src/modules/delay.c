#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "delay.h"


#define DELAY_BUFFER_SIZE 4000


static int destroy (module_delay_t*);
static int process (module_delay_t*, int);
static module_t *clone (module_delay_t*);

struct module_delay_class_s {
    module_c module;
};

typedef struct {
    int T, TTC, feedback;
    buffer_t *in, *out;
} delay_IO_t;

static symbole_t IO_symtab[]={
    {"T",	OFFSET(delay_IO_t, T),	SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"TTC",	OFFSET(delay_IO_t, TTC),SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"feedback",OFFSET(delay_IO_t, feedback),SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"AI",	OFFSET(delay_IO_t, in),	SIAD_SCOPE_IN, SIAD_TYPE_BUFFER},
    {"AO",	OFFSET(delay_IO_t, out),SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {0, 0, 0, 0}
};

obj_c*
module_delay_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(module_delay, module);
	    class->destroy = (obj_destroy_t*)destroy;
	    MODULE_CLASS(class)->process = (module_process_t*)process;
	    MODULE_CLASS(class)->clone = (module_clone_t*)clone;
	}

    return class;
}


struct module_delay_s {
    module_t module;
    delay_IO_t IO;
    smpl_t *buffer;
    int size;
    int tail, head;
    int T, TTC, feedback;//Time To Change
};


int
T_callback(module_delay_t *delay)
{
    ck_err(delay->IO.T > delay->size);
    delay->T = delay->IO.T;
    delay->tail = delay->head + delay->T;
    if(delay->tail < 0)
	delay->tail += delay->size;
    return 0;
  error:
    return -1;
}


int
TTC_callback(module_delay_t *delay)
{
    delay->TTC = delay->IO.TTC;
    return 0;
}


int
feedback_callback(module_delay_t *delay)
{
    delay->feedback = delay->IO.feedback;
    return 0;
}


module_t*
module_delay_create()
{
    module_delay_t * delay=0;

    delay = obj_create(module_delay);
    MODULE(delay)->IO.name = "delay IO";
    MODULE(delay)->IO.size = sizeof(delay_IO_t);
    MODULE(delay)->IO.data = &delay->IO;
    MODULE(delay)->IO.symtab = IO_symtab;

    ck_err(bank_add_watch(&MODULE(delay)->IO, "T", (siad_callback_t)T_callback,  delay) < 0);
    ck_err(bank_add_watch(&MODULE(delay)->IO, "TTC", (siad_callback_t)TTC_callback,  delay) < 0);
    ck_err(bank_add_watch(&MODULE(delay)->IO, "feedback", (siad_callback_t)feedback_callback,  delay) < 0);

    delay->size=DELAY_BUFFER_SIZE;
    ck_err(!(delay->buffer=Xalloc(smpl_t, delay->size)));
    delay->head=0;
    delay->tail=0;

    return MODULE(delay);
  error:
    return 0;
}


module_t*
clone(module_delay_t *delay)
{
    return module_delay_create();
}


int
process(module_delay_t *delay, int size)
{
    int i;
    smpl_t *in, *out;

    ck_err(!delay->IO.out || !delay->IO.in);//Check all buffers
    in = delay->IO.in->smpl;
    out = delay->IO.out->smpl;
    ck_err(size != delay->IO.out->size);

    for(i=0;i<size;i++)
	out[i]=in[i];
    return 0;

    if(!delay->T)
	{
	    for(i=0;i<size;i++)
		out[i]=in[i];
	    return 0;
	}
    for(i=0;i<size;i++)
	{
	    out[i]=in[i]+((delay->buffer[delay->tail] * delay->feedback) >>16);
	    delay->buffer[delay->head]=out[i];
	    delay->buffer[delay->tail] = 0;

	    if(++delay->head >= delay->size)
		delay->head=0;
	    if(++delay->tail >= delay->size)
		delay->tail=0;
	}

    return 0;
  error:
    return -1;
}


int
destroy(module_delay_t *delay)
{
    printf("destroy\n");
    return 0;
}
