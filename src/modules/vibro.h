#ifndef _SISY_MODULE_VIBRO_H_
#define _SISY_MODULE_VIBRO_H_


#include "../module.h"
#include "../obj.h"


obj_c *vibro_class();


#define MODULE_VIBRO_CLASS(class) ((vibro_c*)(class))
#define IS_MODULE_VIBRO(obj)      ((module_VIBRO_t*)check_ancestor(OBJ(obj), OBJ_CLASS(vibro_class()), 0, 0))
#ifdef DEBUG
#define MODULE_VIBRO(obj)        ((module_VIBRO_t*)check_ancestor(OBJ(obj), OBJ_CLASS(vibro_class()), __FILE__, __LINE__))
#else
#define MODULE_VIBRO(obj)        ((module_VIBRO_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_vibro_s module_vibro_t;
typedef struct module_vibro_class_s module_vibro_c;


module_t *module_vibro_create();


#endif
