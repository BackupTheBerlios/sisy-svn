#ifndef _SISY_BANK_H_
#define _SISY_BANK_H_


typedef struct {
    char *name;
    int offset;
    int scope;
    int type;
} symbole_t;


typedef struct {
    unsigned int offset:18;//So, bank can be 256KB large
    unsigned int scope:2;//4 scopes available (in, out, local, global)
    unsigned int type:6;//64 types available (inc. NULL)
    unsigned int bank:6;//64 bank stackable
} siad_t;


typedef int (*siad_callback_t)(void *data);


typedef struct {
    siad_t siad;
    siad_callback_t callback;
    void *data;
} siad_watch_t;

typedef struct {
    void *data;
    int size;
    symbole_t *symtab;//no free
    const char *name;//
    int pos;
    siad_watch_t watches[6];
    int watches_size;
} bank_t;


typedef enum {
    SIAD_SCOPE_LOCAL=0,
    SIAD_SCOPE_IN=1,
    SIAD_SCOPE_OUT=2,
    SIAD_SCOPE_GLOBAL=3,
} siad_scope_t;


typedef enum {
    SIAD_TYPE_NULL=0,
    SIAD_TYPE_VALUE,//aka CONTROLER...
    SIAD_TYPE_BUFFER,
    SIAD_NB_TYPE,
    SIAD_TYPE_ENUM=SIAD_TYPE_VALUE,
} siad_type_t;


typedef struct {
    siad_t src, dst;
} affect_t ;


//////////////


int bank_set(siad_t, void*);
void *bank_get(siad_t);
int bank_push(bank_t*);
int bank_pop();
int bank_pop_check(bank_t*);
int bank_copy(bank_t *src, bank_t *dst);
int bank_clone(bank_t *src, bank_t *dst);

int siad_resolv(siad_t*, char*);
siad_t siad_null();
siad_t create_siad_from_symbole(symbole_t *sym);

int bank_affect(affect_t *affect);//, void *dest_callback_data);
int bank_add_watch(bank_t *bank, char *name, siad_callback_t callback, void *data);

int bank_stack_print();

siad_watch_t *bank_get_watch(siad_t siad);

//int bank_init(bank_t*, void*, int);
//#define BANK_INIT(bank, data) bank_init(bank, data, sizeof(data))
#define OFFSET(_type, field) ((int)(&(((_type*)0)->field)))
#define SIAD_NULL siad_null();

#endif
