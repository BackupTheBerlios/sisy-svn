#include "../module.h"
#include "../audio.h"
#include "../bank.h"
#include "../misc.h"
#include "square.h"


static int destroy (module_square_t*);
static int process (module_square_t*, int);
static module_t *clone (module_square_t*);


struct module_square_class_s {
  module_c module;
};


typedef struct {
  //  trigger_t *trig;
  buffer_t *out;
  int T, level, RC;
} square_IO_t;


static symbole_t IO_symtab[]={
    {"AO",	OFFSET(square_IO_t, out),	SIAD_SCOPE_IN, SIAD_TYPE_BUFFER},
    {"T",	OFFSET(square_IO_t, T),		SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"level",	OFFSET(square_IO_t, level),	SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {0, 0, 0}
};


obj_c*
module_square_class()
{
  static obj_c *class=0;

  if(!class)
    {
      create_class(module_square, module);
      class->destroy = (obj_destroy_t*)destroy;
      MODULE_CLASS(class)->process = (module_process_t*)process;
      MODULE_CLASS(class)->clone = (module_clone_t*)clone;
    }

  return class;
}


struct module_square_s {
    module_t module;
    square_IO_t IO;
    int t, RC, offset;
    int level;
};


static int
T_callback(module_square_t * const square)
{
//    printf("square: %p, T: %d\n", square, square->IO.T);
    square->RC=square->IO.T/2;
    if(square->RC<1024)
	square->level = 0;
    return 0;
}


static int
level_callback(module_square_t * const square)
{
//    printf("square: %p, level: %d\n", square, square->IO.level);
    if(square->level>0)
	square->level = square->IO.level >> 4;
    else
	square->level = -square->IO.level >> 4;
    return 0;
}


module_t*
module_square_create()
{
  module_square_t * square=0;

  //  printf("module_square_create\n");

  square = obj_create(module_square);
  MODULE(square)->IO.name = "square IO";
  MODULE(square)->IO.size = sizeof(square_IO_t);
  MODULE(square)->IO.data = &square->IO;
  MODULE(square)->IO.symtab = IO_symtab;
  //  BANK_INIT(&MODULE(square)->IO, &square->IO, square_IO_t);

  ck_err(bank_add_watch(&MODULE(square)->IO, "T",	(siad_callback_t)T_callback, square) < 0);
  ck_err(bank_add_watch(&MODULE(square)->IO, "level",	(siad_callback_t)level_callback, square) < 0);
 
  return MODULE(square);
  error:
  return 0;
}


module_t*
clone(module_square_t *square)
{
  return module_square_create();
}


int
process(module_square_t * const square, int size)
{
    int i;
    smpl_t *buffer;

    ck_err(!square->IO.out);//Check all buffers
    buffer = square->IO.out->smpl;
    ck_err(size != square->IO.out->size);

    if(!square->level)
	return 0;

    for (i = 0; i < size; i++)
	{
	    buffer[i] = square->level;
	    if ((square->t+=1024) >= (square->RC))
		{
		    square->t -= square->RC;
//		    buffer[i] = (square->level * square->t) >> 10;
		    buffer[i] = square->level;
		    square->level *= -1;
//		    printf("square: %p, %d, Alternation: RC: %d, level: %d, t: %d\n", square, i, square->RC, square->level, square->t);
		}
	}

    return 0;
  error:
    return -1;
}

/* int */
/* process(module_square_t *square) */
/* { */
/*   int i, size; */
/*   smpl_t *buffer; */
/*   static short T=0; */
   
/*   ck_err(!square->IO.buff);//Check all buffers */
/*   buffer = square->IO.out->smpl; */
/*   size = square->IO.out->size; */

/* //  square->IO.level=square->t=square->IO.T=0; */
/*   square->RC=square->IO.T/2; */

/*   //  printf("square->IO.T: %x\t", square->IO.T); */
/*   //  printf("square->IO.level: %x\t", square->IO.level); */
/*   //  printf("square->RC: %x\n", square->RC); */

/*   for (i = 0; i < size; i++) */
/*     { */
/*       if ((square->t += 50) > (square->IO.T)) */
/* 	{ */
/* 	  square->t -= square->IO.T; */
/* 	  buffer[i] = 0; */
/* 	} */
/*       else */
/* 	{ */
/* 	  if (square->t > square->RC) */
/* 	    buffer[i] = square->IO.level * -32; */
/* 	  else */
/* 	    buffer[i] = square->IO.level * 32; */
/* 	} */
/*     } */

/*   return 0; */
/*  error: */
/*   return -1; */
/* } */

int
destroy(module_square_t *square)
{
   printf("destroy\n");
   return 0;
}
