#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "impulse.h"


static int destroy (module_impulse_t*);
static int process (module_impulse_t*, int);
static module_t *clone (module_impulse_t*);


struct module_impulse_class_s {
  module_c module;
};

typedef struct {
  buffer_t *out;
  int level;
} impulse_IO_t;

static symbole_t IO_symtab[]={
    {"level",	OFFSET(impulse_IO_t, level),	SIAD_SCOPE_IN,  SIAD_TYPE_VALUE},
    {"AO",	OFFSET(impulse_IO_t, out),	SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {0, 0, 0}
};

obj_c*
module_impulse_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(module_impulse, module);
      class->destroy = (obj_destroy_t*)destroy;
      MODULE_CLASS(class)->process = (module_process_t*)process;
      MODULE_CLASS(class)->clone = (module_clone_t*)clone;
    }

  return class;
}


struct module_impulse_s {
    module_t module;
    impulse_IO_t IO;
    int level;
};


static int
level_callback(module_impulse_t * const impulse)
{
    impulse->level = impulse->IO.level;
    return 0;
}


module_t*
module_impulse_create()
{
  module_impulse_t * impulse=0;

  impulse = obj_create(module_impulse);
  MODULE(impulse)->IO.name = "impulse IO";
  MODULE(impulse)->IO.size = sizeof(impulse_IO_t);
  MODULE(impulse)->IO.data = &impulse->IO;
  MODULE(impulse)->IO.symtab = IO_symtab;

  ck_err(bank_add_watch(&MODULE(impulse)->IO, "level",	(siad_callback_t)level_callback, impulse) < 0);
 
  return MODULE(impulse);
  error:
  return 0;
}


module_t*
clone(module_impulse_t *impulse)
{
  return module_impulse_create();
}


int
process(module_impulse_t * const impulse, int size)
{
    ck_err(!impulse->IO.out);//Check all buffers
    ck_err(size != impulse->IO.out->size || !size);

    impulse->IO.out->smpl[0] = impulse->level * 256;
    impulse->level=0;

    return 0;
  error:
    return -1;
}

int
destroy(module_impulse_t *impulse)
{
   printf("destroy\n");
   return 0;
}
