#ifndef _SISY_MODULE_DELAY_H_
#define _SISY_MODULE_DELAY_H_


#include "../module.h"
#include "../obj.h"


obj_c *adsr_class();


#define MODULE_delay_CLASS(class) ((adsr_c*)(class))
#define IS_MODULE_delay(obj)      ((module_delay_t*)check_ancestor(OBJ(obj), OBJ_CLASS(adsr_class()), 0, 0))
#ifdef DEBUG
#define MODULE_delay(obj)        ((module_delay_t*)check_ancestor(OBJ(obj), OBJ_CLASS(adsr_class()), __FILE__, __LINE__))
#else
#define MODULE_delay(obj)        ((module_delay_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_delay_s module_delay_t;
typedef struct module_delay_class_s module_delay_c;


module_t *module_delay_create();


#endif
