#ifndef _SISY_MODULE_BIQUAD_H_
#define _SISY_MODULE_BIQUAD_H_


#include "../module.h"
#include "../obj.h"


obj_c *biquad_class();


#define MODULE_BIQUAD_CLASS(class) ((biquad_c*)(class))
#define IS_MODULE_BIQUAD(obj)      ((module_biquad_t*)check_ancestor(OBJ(obj), OBJ_CLASS(biquad_class()), 0, 0))
#ifdef DEBUG
#define MODULE_BIQUAD(obj)        ((module_biquad_t*)check_ancestor(OBJ(obj), OBJ_CLASS(biquad_class()), __FILE__, __LINE__))
#else
#define MODULE_BIQUAD(obj)        ((module_biquad_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_biquad_s module_biquad_t;
typedef struct module_biquad_class_s module_biquad_c;


module_t *module_biquad_create();

#endif
