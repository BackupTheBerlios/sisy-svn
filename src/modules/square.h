#ifndef _SISY_MODULE_SQUARE_H_
#define _SISY_MODULE_SQUARE_H_


#include "../module.h"
#include "../obj.h"


obj_c *square_class();


#define MODULE_SQUARE_CLASS(class) ((square_c*)(class))
#define IS_MODULE_SQUARE(obj)      ((module_square_t*)check_ancestor(OBJ(obj), OBJ_CLASS(square_class()), 0, 0))
#ifdef DEBUG
#define MODULE_SQUARE(obj)        ((module_square_t*)check_ancestor(OBJ(obj), OBJ_CLASS(square_class()), __FILE__, __LINE__))
#else
#define MODULE_SQUARE(obj)        ((module_square_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_square_s module_square_t;
typedef struct module_square_class_s module_square_c;


module_t *module_square_create();

#endif
