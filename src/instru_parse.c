#include <ctype.h>
#include "instru_parse.h"
#include "parse.h"
#include "instru.h"
#include "bank.h"
#include "misc.h"
#include "audio.h"
#include "debug.h"

#define BANK_TMP_SIZE 128

//Parallel allocation

/* Si on copie la bank et la liste de modules apres la definition de la voie, */
/* il faut retrouver les buffers car il faut les realouer. */
/* Si on copie en meme temps toutes les voix, il faut plusieur bank et symtab */

/* Faire une methode fast_clone pour les modules. */

/* SISY */
/* MIDI_GLOB */
/* INSTRU */
/* MIDI_CHAN */
/* VOIX */
/* MIDI_VOIX */
/* MODULES_PARAM */

/* SISY */
/* MIDI_GLOB */
/* INSTRU */
/* MIDI_CHAN */
/* MODULES_PARAM */

int string_get_value(char *string);
int string_is_number(char *string);

//JOE HILL
int line=1;
instru_t *instru=0;

//DATA STACK
bank_t *data_bank=0;
#define DATA_STACK_SIZE 1024
char data_stack[DATA_STACK_SIZE];
int data_stack_offset=0;
#define DATA_STACK_SYMTAB_SIZE 256
symbole_t data_stack_symtab[DATA_STACK_SYMTAB_SIZE];
int data_stack_symtab_offset=0;


void (*data_hook)()=0;
list_t *data_affects=0;
list_t *module_affects=0;
module_t *module_process_current=0;
list_t *process_modules_list=0;


int
data_stack_bank_init(bank_t *bank)
{
    if(dbg_filter&DBG_PARSE)
	printf("*data_stack_bank_init\n");
    bank->name = "data stack";
    bank->size = DATA_STACK_SIZE;
    memset(data_stack, 0, DATA_STACK_SIZE);
    bank->data = data_stack;
    memset(data_stack_symtab, 0, DATA_STACK_SYMTAB_SIZE);
    bank->symtab = data_stack_symtab;
    data_stack_offset = 0;
    data_stack_symtab_offset = 0;
    return 0;
}


int
data_stack_bank_freeze(bank_t *bank)
{
    if(dbg_filter&DBG_PARSE)
	printf("*data_stack_bank_freeze\n");
    bank->size = data_stack_offset;
    bank->data = Malloc(bank->size);
    memcpy(bank->data, data_stack, bank->size);
    bank->symtab = Xalloc(symbole_t, data_stack_symtab_offset+1);
    memcpy(bank->symtab, data_stack_symtab, sizeof(symbole_t) * data_stack_symtab_offset);
    //printf("  data_affects = 0;\n");
    data_affects = 0;
    return 0;
}


int
data_stack_alloc(char *name, int type, int scope)
{
    symbole_t *sym;
    if(dbg_filter&DBG_PARSE)
	printf("*data_stack_alloc\n");
    ck_err(type < 0 || type >= SIAD_NB_TYPE);
    ck_err(data_stack_offset + siad_size[type] > DATA_STACK_SIZE);
    ck_err(data_stack_symtab_offset+1 >= DATA_STACK_SYMTAB_SIZE);

    sym=&data_stack_symtab[data_stack_symtab_offset++];
    sym->name = strdup(name);
    sym->type = type;
//    printf("data alloc: %s, %d, %d\n", name, type, scope);
    sym->scope = scope;
    sym->offset = data_stack_offset;
    data_stack_offset += siad_size[type];
    if(dbg_filter&DBG_PARSE)
	printf("data_stack_offset: %d\n", data_stack_offset);
    {
	siad_t siad;
	ck_err(siad_resolv(&siad, name) < 0);
	bank_get(siad);
    }
    return 0;
  error:
    return -1;
}


void
instru_data()
{
    if(dbg_filter&DBG_PARSE)
	printf("*instru_data\n");
}


void instru_begin(char *name)
{
    if(dbg_filter&DBG_PARSE)
	printf("*instru_begin: %s\n", name);
    instru->name = strdup(name);

    bank_push(&instru->midi.bank);
    bank_push(&instru->bank_IO);
    bank_push(&instru->bank_USER);//Init in data_begin

    data_affects = &instru->affects;
    //printf("  data_affects = &instru->affects;\n"); 
    data_bank = &instru->bank_USER;

    data_hook=instru_data;
}


void instru_end()
{
    if(dbg_filter&DBG_PARSE)
	printf("*instru_end\n");
    ck_err(bank_pop_check(&instru->bank_USER));
    ck_err(bank_pop_check(&instru->bank_IO));
    ck_err(bank_pop_check(&instru->midi.bank));
    if(!instru->voices)
	{
	    instru->IO.nb_voices=0;
	}
  error:
    return;
}


void voice_begin()
{
    instru->IO.nb_voices=6;
    if(dbg_filter&DBG_PARSE)
	printf("*voice_begin: %d\n", instru->IO.nb_voices);

    ck_err(instru_midi_mux_init(&instru->midi, instru->IO.nb_voices)<0);
    ck_err(!instru->IO.nb_voices);
    ck_err(!(instru->voices = Xalloc(instru_voice_t, instru->IO.nb_voices)));

    ck_err(instru_voice_init(instru->voices, &instru->midi, 0) < 0);

//    printf("bank: %s\n", instru->voices->bank_USER.name);//Init in data_begin

    bank_push(&instru->voices->midi.bank);
    bank_push(&instru->voices->bank_IO);
    bank_push(&instru->voices->bank_USER);//Init in data_begin

    data_bank = &instru->voices->bank_USER;
    data_affects = &instru->voices->affects;
    //printf("  data_affects = &instru->voices->affects;\n");
    process_modules_list = &instru->voices->modules;

  error:
    return;
}


void voice_end()
{
    int i;
    if(dbg_filter&DBG_PARSE)
	printf("*voice_end\n");
    for(i=1; i<instru->IO.nb_voices; i++)
	{
	    ck_err(instru_voice_init(instru->voices+i, &instru->midi, i)<0);
	    ck_err(instru_voice_copy(instru->voices, instru->voices+i)<0);
	}
    ck_err(bank_pop_check(&instru->voices->bank_USER));
    ck_err(bank_pop_check(&instru->voices->bank_IO));
    ck_err(bank_pop_check(&instru->voices->midi.bank));
  error:
    return;
}


void global_begin()
{
    if(dbg_filter&DBG_PARSE)
	printf("*global_begin\n");

    ck_err(instru_voice_init(&instru->global, 0, 0)<0);
    if(dbg_filter&DBG_PARSE)
	printf("global: buffer: %p\n", instru->global.IO.buffer);
    //   instru->global.bank_IO.name = "instru global IO";
    //   instru->global.bank_IO.size = sizeof(instru_voice_IO_t);
    //   instru->global.bank_IO.data = &instru->global.IO;
    //   instru->global.bank_IO.symtab = instru_voice_IO_symtab;//yep, voice for global, ok ok, i'll change it

    bank_push(&instru->global.midi.bank);
    bank_push(&instru->global.bank_IO);
    bank_push(&instru->global.bank_USER);//Init in data_begin

    //   data_stack_bank_init(&instru->global.bank_USER);
    data_affects = &instru->global.affects;
    //printf("  data_affects = &instru->global.affects;");
    process_modules_list = &instru->global.modules;
  error:
    return;
}


void global_end()
{
    if(dbg_filter&DBG_PARSE)
	printf("*global_end\n");
    ck_err(bank_pop_check(&instru->global.bank_USER));
    ck_err(bank_pop_check(&instru->global.bank_IO));
    ck_err(bank_pop_check(&instru->global.midi.bank));
  error:
    return;
}


void data_begin()
{
    if(dbg_filter&DBG_PARSE)
	printf("*data_begin\n");
    data_stack_bank_init(data_bank);
}


void data_end()
{
    if(dbg_filter&DBG_PARSE)
	printf("*data_end\n");
    data_stack_bank_freeze(data_bank);
    if(data_hook)
	{
	    data_hook();
	    data_hook=0;
	}
}


void module_begin(char *name)
{
    node_t *node;
    if(dbg_filter&DBG_PARSE)
	printf("*module_begin: \"%s\"\n", name);
    ck_err(!(module_process_current=module_create(name)));
    node = node_create(module_process_current);
    ck_err(list_add_end(process_modules_list, node) < 0);
    module_affects = &module_process_current->affects;
    bank_push(&module_process_current->IO);
  error:
    return;
}


void module_end()
{
    if(dbg_filter&DBG_PARSE)
	printf("*module_end\n");  
    ck_err(bank_pop_check(&module_process_current->IO));
  error:
    return;
}


void assign(int type, char *name, char *value)
{
    affect_t affect;
    if(dbg_filter&DBG_PARSE)
	printf("*assign: %d, %s, %s\n", type, name, value);
    ck_err(siad_resolv(&affect.dst, name) < 0);
    if(string_is_number(value))
	{
	    int ptr=string_get_value(value);
	    bank_set(affect.dst, &ptr);
	}
    else
	{
	    node_t *node;
	    affect_t *ptr;

	    node = node_create(ptr=Talloc(affect_t));
	    *ptr = affect;
	    if(!data_affects)
		{
		    printf("ARRGGG !!! no data_affects !!!\n");
		    abort();
		}
	    ck_err(list_add_beg(data_affects, node) < 0);   
	}
  error:
    return;
}

/* //IN/OUT: no assign, then default value */
/* //LOCAL: [immediate value] */
/* int */
/* assign(siad_t siad, char *value) */
/* { */
/*     if((!value) || (siad.type & (SIAD_SCOPE_IN | SIAD_SCOPE_OUT))) */
/* 	{ */
/* 	    ck_err(value); */
/* 	    return 0; */
/* 	} */

/*     if(siad.type == SIAD_TYPE_ */
/*  str_is_int(value)) */
/* 	{ */
/* 	    ck_err(!affect.dst.type==SIAD_TYPE_VALUE); */
/* 	    int ptr=atoi(value); */
/* 	    bank_set(affect.dst, &ptr); */
/* 	} */
/*     else if(value) */
/* 	{ */
/* 	    node_t *node; */
/* 	    affect_t *ptr; */
/* 	    node = node_create(ptr=Talloc(affect_t)); */
/* 	    *ptr = affect; */
/* 	    if(!data_affects) */
/* 		{ */
/* 		    printf("ARRGGG !!! no data_affects !!!\n"); */
/* 		    abort(); */
/* 		} */
/* 	    ck_err(list_add_beg(module_affects, node) < 0); */
/* 	} */
/* } */


void
declare(int scope, int type, char *name, char *value)
{
    switch(scope)
	{
	case SIAD_SCOPE_IN:
	case SIAD_SCOPE_OUT:
	    ck_err(data_stack_alloc(name, type, scope)<0);
	    break;
	case SIAD_SCOPE_LOCAL:
	    ck_err(data_stack_alloc(name, type, scope)<0);
	    if(type==SIAD_TYPE_BUFFER)
		{
		    siad_t siad;
		    buffer_t *buffer;
		    ck_err(!(buffer=buffer_create()));
		    ck_err(siad_resolv(&siad, name)<0);
		    ck_err(bank_set(siad, buffer) < 0);
		    //      printf("create: bank_get: %p, %p\n", bank_get(siad), buffer);
		}
	    break;
/* 	case SIAD_SCOPE_GLOBAL: */
	default:
	    ck_err("default");
	}
  error:;
}


void
key_peer(char *name, char *value)
{
    affect_t *affect;
    node_t *node;
    siad_t siad;
    siad_watch_t *watch;

    if(dbg_filter&DBG_PARSE)
	printf("*key_peer: \"%s\", \"%s\"\n", name, value);

    ck_err(siad_resolv(&siad, name) < 0);

    switch(siad.scope)
	{
	case SIAD_SCOPE_IN:
	    if(dbg_filter&DBG_PARSE)
		printf("SIAD_SCOPE_IN: %s\n", value);
	    if(string_is_number(value))
		{
		    int i=string_get_value(value);
		    if(dbg_filter&DBG_PARSE)
			printf("string_is_number(value)\n");
		    ck_err(siad.type!=SIAD_TYPE_VALUE);
		    bank_set(siad, &i);
		    if((watch=bank_get_watch(siad)) && watch->callback)
			watch->callback(watch->data);
		    break;
		}
	    node = node_create(affect=Talloc(affect_t));
	    ck_err(siad_resolv(&affect->src, value) < 0);
	    affect->dst=siad;
	    ck_err(list_add_beg(module_affects, node) < 0);
	    break;
	case SIAD_SCOPE_OUT:
	    if(dbg_filter&DBG_PARSE)
		printf("SIAD_SCOPE_OUT: %s\n", value);
	    node = node_create(affect=Talloc(affect_t));
	    ck_err(siad_resolv(&affect->dst, value) < 0);
	    affect->src=siad;
	    ck_err(list_add_beg(module_affects, node) < 0);
	    if(siad.type==SIAD_TYPE_BUFFER)
		ck_err(bank_set(siad, bank_get(affect->dst)) < 0);
	    break;
	default:
	    ck_err("default");
	}
    return;
  error:
//    if(node)free(node);
//    if(affect)free(affect);
//  if(dbg_filter&DBG_PARSE)
    bank_stack_print(); 
    printf("Unknow symbol \"%s\" or \"%s\"\n", name, value);
    abort();
    return;
}

int
string_is_number(char *string)
{
    if((*string >= '0' && *string <= '9') || *string == '.')
	return 1;
    return 0;
}

int
string_is_float(char *string)
{
    char *s=string;

    while(*s)
	{
	    if(*s=='.')
		break;
	    s++;
	}
    if(*s=='.')
	return 1;
    return 0;
}

int
string_get_value(char *string)
{
    if(string_is_float(string))
	return (int)((float)(1<<14) * atof(string));
    else return atoi(string);
}
