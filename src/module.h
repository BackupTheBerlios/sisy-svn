#ifndef _SISY_MODULE_H_
#define _SISY_MODULE_H_


#include "obj.h"
#include "bank.h"
#include "list.h"


obj_c *module_class();


#define MODULE_CLASS(class) ((module_c*)(class))
#define IS_MODULE(obj)      ((module_t*)check_ancestor(OBJ(obj), OBJ_CLASS(module_class()), 0, 0))
#ifdef DEBUG
#define MODULE(obj)        ((module_t*)check_ancestor(OBJ(obj), OBJ_CLASS(module_class()), __FILE__, __LINE__))
#else
#define MODULE(obj)        ((module_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_s module_t;
typedef struct module_class_s module_c;

typedef int module_process_t (module_t*, int);
typedef module_t *module_clone_t (module_t*);
typedef module_t *module_create_t ();

int module_affect(module_t*);
siad_t module_siad_resolv(module_t*, char*);
int module_process(module_t*, int);
module_t *module_clone(module_t *module);

struct module_s {
  module_c *obj;
  bank_t IO;
  list_t affects;
};


struct module_class_s {
  obj_c obj;
  module_process_t *process;
  module_clone_t *clone;
};


//#define module_process(module, size) (MODULE(module)->obj->process(MODULE(module), (size)))
module_t *module_create(char*);


#endif
