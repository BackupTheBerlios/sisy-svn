#include "misc.h"
#include "module.h"
#include "bank.h"
#include "debug.h"
#include "modules/square.h"
#include "modules/biquad.h"
#include "modules/biquad_f.h"
#include "modules/impulse.h"
#include "modules/mixer.h"
#include "modules/DFOF.h"
#include "modules/ADSR.h"
#include "modules/delay.h"
#include "modules/vibro.h"


obj_c*
module_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(module, obj);
	    MODULE_CLASS(class)->process = (module_process_t*)dummy_int;
	}

    return class;
}


module_t*
module_clone(module_t *module)
{
    module_t *clone;
    int i;

    ck_err(!(clone=module->obj->clone(module)));
    ck_err(bank_copy(&module->IO, &clone->IO) < 0);

    clone->affects=module->affects;
    clone->IO.watches_size=module->IO.watches_size;
    for(i=0; i<module->IO.watches_size; i++)
	{
	    clone->IO.watches[i].data=clone;
	    clone->IO.watches[i].callback(clone);
	}

    return clone;
  error:
    return 0;
}


int
module_process(module_t *module, int size)
{
    node_t *node;
    dbg(DBG_PROC, "module_affect");

//    bank_stack_print();

    for(node=module->affects.first; node; node=node->next)
	{
	    affect_t *affect=node->data;
	    if(affect->src.bank<affect->dst.bank)
		ck_err(bank_affect(affect) < 0);
	}

    MODULE(module)->obj->process(MODULE(module), size);

    for(node=module->affects.first; node; node=node->next)
	{
	    affect_t *affect=node->data;
	    if(affect->src.bank>affect->dst.bank)
		ck_err(bank_affect(affect) < 0);
	}

    return 0;
  error:
    return -1;
}


typedef struct {
    char *name;
    module_create_t *create;
} module_symbole_t;

module_symbole_t module_symtab[]={
    {"square",   module_square_create},
    {"biquad",   module_biquad_create},
    {"biquad_f", module_biquad_f_create},
    {"impulse",  module_impulse_create},
    {"mixer",    module_mixer_create},
    {"DFOF",     module_DFOF_create},
    {"ADSR",     module_ADSR_create},
    {"delay",    module_delay_create},
    {"vibro",    module_vibro_create},
    {0, 0}
};


module_t*
module_create(char *name)
{
    module_symbole_t *modsym;

    for(modsym=module_symtab; modsym->name; modsym++)
	if(!strcmp(modsym->name, name))
	    return modsym->create();

// error:
    printf("module_create: %s not found\n", name);
    return 0;
}
//Sisy specifc stuffs:
