#ifndef _SISY_MODULE_MIXER_H_
#define _SISY_MODULE_MIXER_H_


#include "../module.h"
#include "../obj.h"


obj_c *mixer_class();


#define MODULE_MIXER_CLASS(class) ((mixer_c*)(class))
#define IS_MODULE_MIXER(obj)      ((module_mixer_t*)check_ancestor(OBJ(obj), OBJ_CLASS(mixer_class()), 0, 0))
#ifdef DEBUG
#define MODULE_MIXER(obj)        ((module_mixer_t*)check_ancestor(OBJ(obj), OBJ_CLASS(mixer_class()), __FILE__, __LINE__))
#else
#define MODULE_MIXER(obj)        ((module_mixer_t*)OBJ(obj))
#endif


//DEFINITIONS
typedef struct module_mixer_s module_mixer_t;
typedef struct module_mixer_class_s module_mixer_c;


module_t *module_mixer_create();

#endif
