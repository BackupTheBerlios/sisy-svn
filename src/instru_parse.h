#ifndef _SISY_INSTRU_PARSE_H_
#define _SISY_INSTRU_PARSE_H_

#include "instru.h"

int instru_parse(instru_t *instru, char *name);

void instru_begin(char *name);
void instru_end();

void voice_begin();
void voice_end();

void global_begin();
void global_end();

void data_begin();
void data_end();

void module_begin(char*);
void module_end();

void declare(int scope, int type, char *name, char *value);

void key_peer(char *key, char *peer);

extern instru_t *instru;

#endif
