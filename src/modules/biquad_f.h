#ifndef _SISY_MODULE_BIQUAD_F_H_
#define _SISY_MODULE_BIQUAD_F_H_


#include "../module.h"
#include "../obj.h"


obj_c *biquad_f_class();


#define MODULE_BIQUAD_F_CLASS(class) ((biquad_f_c*)(class))
#define IS_MODULE_BIQUAD_F(obj)      ((module_biquad_f_t*)check_ancestor(OBJ(obj), OBJ_CLASS(biquad_f_class()), 0, 0))
#ifdef DEBUG
#define MODULE_BIQUAD_F(obj)        ((module_biquad_f_t*)check_ancestor(OBJ(obj), OBJ_CLASS(biquad_f_class()), __FILE__, __LINE__))
#else
#define MODULE_BIQUAD_F(obj)        ((module_biquad_f_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_biquad_f_s module_biquad_f_t;
typedef struct module_biquad_f_class_s module_biquad_f_c;

module_t *module_biquad_f_create();

#endif
