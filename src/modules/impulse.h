#ifndef _SISY_MODULE_IMPULSE_H_
#define _SISY_MODULE_IMPULSE_H_


#include "../module.h"
#include "../obj.h"


obj_c *impulse_class();


#define MODULE_IMPULSE_CLASS(class) ((impulse_c*)(class))
#define IS_MODULE_IMPULSE(obj)      ((module_impulse_t*)check_ancestor(OBJ(obj), OBJ_CLASS(impulse_class()), 0, 0))
#ifdef DEBUG
#define MODULE_IMPULSE(obj)        ((module_impulse_t*)check_ancestor(OBJ(obj), OBJ_CLASS(impulse_class()), __FILE__, __LINE__))
#else
#define MODULE_IMPULSE(obj)        ((module_impulse_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_impulse_s module_impulse_t;
typedef struct module_impulse_class_s module_impulse_c;


module_t *module_impulse_create();

#endif
