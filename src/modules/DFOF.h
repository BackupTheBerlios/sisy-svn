#ifndef _SISY_MODULE_DFOF_H_
#define _SISY_MODULE_DFOF_H_


#include "../module.h"
#include "../obj.h"


obj_c *DFOF_class();


#define MODULE_DFOF_CLASS(class) ((DFOF_c*)(class))
#define IS_MODULE_DFOF(obj)      ((module_DFOF_t*)check_ancestor(OBJ(obj), OBJ_CLASS(DFOF_class()), 0, 0))
#ifdef DEBUG
#define MODULE_DFOF(obj)        ((module_DFOF_t*)check_ancestor(OBJ(obj), OBJ_CLASS(DFOF_class()), __FILE__, __LINE__))
#else
#define MODULE_DFOF(obj)        ((module_DFOF_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_DFOF_s module_DFOF_t;
typedef struct module_DFOF_class_s module_DFOF_c;


module_t *module_DFOF_create();

#endif
