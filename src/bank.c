#include <stdlib.h>
#include "misc.h"
#include "audio.h"
#include "bank.h"
#include "debug.h"


int siad_size[SIAD_NB_TYPE]={0, 4, 4};


//BANK_STACK
#define BANK_STACK_SIZE 32
bank_t *bank_stack[BANK_STACK_SIZE];
int bank_stack_next=0;
static int siad_check(siad_t siad);
static void siad_print(siad_t siad);


int
bank_push(bank_t *bank)
{
    if(bank&&dbg_filter&DBG_BANK)
	printf("push bank \"%s\"\n", bank->name);

    ck_err(!bank->name);

    ck_err(bank_stack_next >= BANK_STACK_SIZE);
    if(!bank->pos && bank->pos != bank_stack_next)
	{
	    int i;
	    if(bank&&dbg_filter&DBG_BANK)
		printf("BANK[%d] %s: NEW POS: %d!!!\n", bank->pos, bank->name, bank_stack_next);
	    bank->pos = bank_stack_next;
	    for(i=0; i<bank->watches_size; i++)
		{
		    bank->watches[i].siad.bank = bank_stack_next;
		}
	}
 
    bank_stack[bank_stack_next++] = bank;
    return 0;
  error:
    return -1;
}


int
bank_pop()
{
    ck_err(bank_stack_next <= 0);
    if(bank_stack[bank_stack_next-1]&&dbg_filter&DBG_BANK)
	printf("pop bank \"%s\"\n", bank_stack[bank_stack_next-1]->name);
    bank_stack[--bank_stack_next] = 0;
    return 0;
  error:
    return -1;
}


void
symbols_print(symbole_t *symbols)
{
    if(!symbols)
	{
	    printf("no symbols\n");
	    return;
	}
    while(symbols->name)
	{
	    printf("symbol: %s\n", symbols->name);
	    symbols++;
	}
}

void
watches_print(siad_watch_t *watches, int size)
{
    while(size--)
	{
	    printf("watch: callback: %p, data: %p, siad: ", watches[size].callback, watches[size].data);
	    siad_print(watches[size].siad);
	}
}


int
bank_stack_print()
{
    int i;
    for(i=0; i<bank_stack_next; i++)
	{
	    if(bank_stack[i])
		{
		    int c;
		    for(c=0; c<i; c++)
			printf(" ");
		    printf("%s:\n", bank_stack[i]->name);
		    symbols_print(bank_stack[i]->symtab);
		    ck_err(bank_stack[i]->watches_size>6);
		    ck_err(bank_stack[i]->watches_size<0);
		    watches_print(bank_stack[i]->watches, bank_stack[i]->watches_size);
		}
	}
error:    return 0;
}

int
bank_pop_check(bank_t *bank)
{
    ck_err(bank_stack[bank_stack_next-1] != bank);
    bank_pop();
    return 0;
  error:
    if(bank&&dbg_filter&DBG_BANK)
	bank_stack_print();
    bank_pop();
    return -1;
}

int
bank_init(bank_t *bank, void *data, int size)
{
    ck_err(!bank);
    bank->size = size;
    bank->data = data;
    return 0;
  error:
    return -1;
}

int
siad_cmp(siad_t a, siad_t b)
{
    if(a.bank == b.bank && a.offset == b.offset && a.type == b.type)
	return 0;
    return 1;
}

siad_watch_t*
bank_get_watch(siad_t siad)
{
    bank_t *bank;
    int i;
    
    ck_err(siad_check(siad));

    ck_err(!(bank = bank_stack[siad.bank]));
    for(i=0; i<bank->watches_size; i++)
	if(!siad_cmp(siad, bank->watches[i].siad))
	    return bank->watches+i;

  error:
    return 0;
}


int
bank_affect(affect_t *affect)
{
    siad_watch_t *watch;
    void *ptr;

    if(dbg_filter&DBG_BANK)
	printf("bank_affect: %p\n", affect);

    ck_err(!affect);
    ck_err(siad_check(affect->src));
    ck_err(siad_check(affect->dst));
    ptr = bank_get(affect->src);

    switch(affect->src.type)
	{
	case SIAD_TYPE_VALUE:
	    if(*(int*)ptr != *(int*)bank_get(affect->dst))
		{
		    bank_set(affect->dst, ptr);
		    if((watch=bank_get_watch(affect->dst)) && watch->callback)
			{
//			    printf("callback of watch of bank %s with data: %p\n", bank_stack[watch->siad.bank]->name, watch->data);
			    watch->callback(watch->data);
			}
		}
	    break;
	case SIAD_TYPE_BUFFER:
	    bank_set(affect->dst, ptr);
	    break;
	default:
	    return -1;
	}

    return 0;
  error:
    return -1;
}


int
bank_set(siad_t siad, void *data)
{
    bank_t *bank;

    ck_err(siad_check(siad));
    ck_err(!(bank = bank_stack[siad.bank]));

    switch(siad.type)
	{
	case SIAD_TYPE_VALUE:
	    *(int*)(bank->data + siad.offset) = *(int*)data;
	    break;
	case SIAD_TYPE_BUFFER:
	    *(buffer_t**)(bank->data + siad.offset) = (buffer_t*)data;
	    break;
	default:
	    return -1;
	}

    return 0;
  error:
    return -1;
}


void*
bank_get(siad_t siad)
{
    bank_t *bank;

    ck_err(siad_check(siad));
    ck_err(!(bank = bank_stack[siad.bank]));

    switch(siad.type)
	{
	case SIAD_TYPE_VALUE:
	    return (bank->data + siad.offset);
	    break;
	case SIAD_TYPE_BUFFER:
	    return *(void**)(bank->data + siad.offset);
	    break;
	default:
	    break;
	}
  error:
    return 0;
}


//SIAD MISC
static int siad_check(siad_t siad)
{
    bank_t *bank;
    ck_err(siad.bank < 0 || siad.bank >= bank_stack_next);
    ck_err(!(bank=bank_stack[siad.bank]));
    ck_err(siad.type < 0 || siad.type >= SIAD_NB_TYPE);
    ck_err(siad.offset < 0);
    ck_err(siad.offset + siad_size[siad.type] > bank->size);
    return 0;
  error:
    return -1;
}


siad_t
siad_null()
{
    siad_t siad;
    memset(&siad, 0, sizeof(siad_t));
    return siad;
}


void
siad_print(siad_t siad)
{
    printf("siad: bank: %d, type: %d, scope: %d, offset: %d\n", siad.bank, siad.type, siad.scope, siad.offset);
}

int
siad_resolv(siad_t *siad, char *name)
{
    symbole_t *sym;
    int pos;

    siad->type=0;

    if(dbg_filter&DBG_BANK)
	printf("siad_resolv(%p, %s)\n", siad, name);

    for(pos=0; pos < bank_stack_next; pos++)
	{
	    //      printf("pos:%d\n", pos);
	    for(sym=bank_stack[pos]->symtab; sym && sym->name; sym++)
		{
		    if(!strcmp(name, sym->name))
			{
			    //	      ck_err(siad->type);
			    if(dbg_filter&DBG_BANK)
				printf("resolved siad: \"%s\" in bank: %s (%d)\n", name, bank_stack[pos]->name, pos);
			    siad->bank=pos;
			    siad->type = sym->type;
			    siad->scope = sym->scope;
			    siad->offset = sym->offset;
			    if(dbg_filter&DBG_BANK)
				siad_print(*siad);
			    ck_err(siad_check(*siad));
			    //	      return 0;
			}
		}
	}
    if(!siad->type)
	goto error;
    return 0;
  error:
    if(dbg_filter&DBG_BANK)
	printf("Error: \"%s\" symbole not found\n", name);
    return -1;
}


int
bank_copy(bank_t *src, bank_t *dst)
{
    ck_err(src->size!=dst->size);
//    dst->size = src->size;//already copied
    dst->symtab = src->symtab;
    dst->name = src->name;
    dst->pos = src->pos;
    memcpy(dst->data, src->data, src->size);
    dst->watches_size=src->watches_size;
    memcpy(dst->watches, src->watches, sizeof(siad_watch_t)*src->watches_size);
//    we dont touch to the watches
    return 0;
  error:
    return -1;
}

int
bank_clone(bank_t *src, bank_t *dst)
{
//    memcpy(dst, src, sizeof(bank_t));
    ck_err(dst->data);
    if(src->size)
	ck_err(!(dst->data=Malloc(src->size)));
    ck_err(bank_copy(src, dst)<0);
    
    return 0;
  error:
    return -1;
}

int
bank_add_watch(bank_t *bank, char *name, siad_callback_t callback, void *data)
{
    siad_t siad;
    symbole_t *sym;

    ck_err(bank->watches_size >= 6);

    for(sym=bank->symtab; sym; sym++)
	{
	    if(name && sym->name && !strcmp(name, sym->name))
		{
		    if(dbg_filter&DBG_BANK)
			printf("bank add watch resolved siad: \"%s\"\n", name);
		    siad.offset = sym->offset;
		    siad.type = sym->type;
		    break;
		}
	}

    bank->watches[bank->watches_size].callback = callback;
    bank->watches[bank->watches_size].data = data;
    bank->watches[bank->watches_size].siad = siad;
    bank->watches_size++;

    return 0;
  error:
    return -1;
}
