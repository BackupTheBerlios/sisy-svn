#ifndef _SISY_MODULE_ADSR_H_
#define _SISY_MODULE_ADSR_H_


#include "../module.h"
#include "../obj.h"


obj_c *adsr_class();


#define MODULE_ADSR_CLASS(class) ((adsr_c*)(class))
#define IS_MODULE_ADSR(obj)      ((module_ADSR_t*)check_ancestor(OBJ(obj), OBJ_CLASS(adsr_class()), 0, 0))
#ifdef DEBUG
#define MODULE_ADSR(obj)        ((module_ADSR_t*)check_ancestor(OBJ(obj), OBJ_CLASS(adsr_class()), __FILE__, __LINE__))
#else
#define MODULE_ADSR(obj)        ((module_ADSR_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_ADSR_s module_ADSR_t;
typedef struct module_ADSR_class_s module_ADSR_c;


module_t *module_ADSR_create();


#endif
