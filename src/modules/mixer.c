#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "../math.h"
#include "mixer.h"


static int destroy (module_mixer_t*);
static int process (module_mixer_t*, int);
static module_t *clone (module_mixer_t*);


struct module_mixer_class_s {
    module_c module;
};

typedef struct {
    //  trigger_t *trig;
    buffer_t *out, *in0, *in1;
    int level0, level1;
} mixer_IO_t;

static symbole_t IO_symtab[]={
    {"AO",	OFFSET(mixer_IO_t, out),	SIAD_SCOPE_IN, SIAD_TYPE_BUFFER},
    {"AI0",	OFFSET(mixer_IO_t, in0),	SIAD_SCOPE_IN, SIAD_TYPE_BUFFER},
    {"AI1",	OFFSET(mixer_IO_t, in1),	SIAD_SCOPE_IN, SIAD_TYPE_BUFFER},
    {"level0",	OFFSET(mixer_IO_t, level0),	SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"level1",	OFFSET(mixer_IO_t, level1),	SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};

obj_c*
module_mixer_class()
{
    static obj_c *class=0;

    if(!class)
	{
	    create_class(module_mixer, module);
	    class->destroy = (obj_destroy_t*)destroy;
	    MODULE_CLASS(class)->process = (module_process_t*)process;
	    MODULE_CLASS(class)->clone = (module_clone_t*)clone;
	}

    return class;
}


struct module_mixer_s {
    module_t module;
    mixer_IO_t IO;
    int level0, level1;
};


static int
level_callback(module_mixer_t * const mixer)
{
//    printf("mixer: %p, levels: %f, %f\n", mixer, fixed2float(mixer->IO.level0, 15), fixed2float(mixer->IO.level1, 15));
    mixer->level0=mixer->IO.level0;
    mixer->level1=mixer->IO.level1;
    return 0;
}


module_t*
module_mixer_create()
{
    module_mixer_t * mixer=0;

    mixer = obj_create(module_mixer);
    MODULE(mixer)->IO.name = "mixer IO";
    MODULE(mixer)->IO.size = sizeof(mixer_IO_t);
    MODULE(mixer)->IO.data = &mixer->IO;
    MODULE(mixer)->IO.symtab = IO_symtab;
    //  BANK_INIT(&MODULE(mixer)->IO, &mixer->IO, mixer_IO_t);

    ck_err(bank_add_watch(&MODULE(mixer)->IO, "level0",	(siad_callback_t)level_callback, mixer) < 0);
    ck_err(bank_add_watch(&MODULE(mixer)->IO, "level1",	(siad_callback_t)level_callback, mixer) < 0);

    mixer->level0 = 0;
    mixer->level1 = 0;
 
    return MODULE(mixer);
  error:
    return 0;
}


module_t*
clone(module_mixer_t *mixer)
{
    return module_mixer_create();
}


int
process(module_mixer_t * const mixer, int size)
{
    int i, level0, level1;
    smpl_t *out, *in0, *in1;

    ck_err(!mixer->IO.out);
    ck_err(!mixer->IO.in0);
    ck_err(!mixer->IO.in1);
    out = mixer->IO.out->smpl;
    in0 = mixer->IO.in0->smpl;
    in1 = mixer->IO.in1->smpl;
    level0 = mixer->level0;
    level1 = mixer->level1;
    ck_err(size != mixer->IO.out->size);

    for (i = 0; i < size; i++)
	out[i] = ((in0[i] * level0) >> 15) + ((in1[i] * level1) >> 15);

    return 0;
  error:
    return -1;
}


int
destroy(module_mixer_t *mixer)
{
    printf("destroy\n");
    return 0;
}
