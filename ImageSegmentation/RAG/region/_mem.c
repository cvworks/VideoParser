/*
 * region/_mem.c
 *
 * Sven Wachsmuth, 08.04.2003
 *
 * maintenance of memory arrays
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "_mem.h"


void *
_rag_realloc_memory_space(void *mem, int *max_mem,
			  int _n_mem, size_t memsize,
			  rag_copyFunc *copyFunc,
			  rag_initFunc *initFunc, 
			  rag_freeFunc *freeFunc)
{
  int __max_mem = (*max_mem);

  if (_n_mem * memsize <= 0) return (mem);

  /** if already enough memory allocated ... */
  if ((*max_mem) >= _n_mem) {
    /* ... if previous items should be freed (inner structure), free them */
    if (freeFunc) {
      int i, n=__max_mem;
      char *item;
      for (i=0, item=(char *)mem; i < n; i++, item+=memsize) {
	(*freeFunc)((void *) item);
	if (initFunc) (*initFunc)((void *) item);
      }
    }
    /* ... return memory */
    return (mem);
  }
  
  /** if mem was previously allocated (no reference) ... */
  if ((*max_mem) > 0) {
    /* ... realloc memory ... */
    mem = (void *) realloc(mem, _n_mem * memsize);
    if (!mem) {
      fprintf(stderr,"_realloc_memory_space: not enough memory (%d x %d bytes)\n", _n_mem, memsize);
      exit(1);
    }
    /* ... if previous items should be freed (inner structure), free them */
    if (freeFunc) {
      int i, n=__max_mem;
      char *item;
      for (i=0, item=(char *)mem; i < n; i++, item+=memsize)
	(*freeFunc)((void *) item);
      __max_mem = 0;
    }
    
  } 
  /** else (mem was only referenced) ... */
  else {
    void *_mem = mem;

    /* ... allocate new memory ... */
    mem = (void *) malloc(_n_mem * memsize);
    if (!mem) {
      fprintf(stderr,"_alloc_memory_space: not enough memory (%d x %d bytes)\n", _n_mem, memsize);
      exit(1);
    }

    /* ... if previous items should be freed, nothing is to do ... */
    if (freeFunc)
      __max_mem = 0;

    else if (__max_mem > 0) { 
      /* ... else previous items have to copied to the new memory ... */
      if (copyFunc) {
	int i;
	char *item, *_item;
	for (i=0, item=(char *)mem, _item=(char *)_mem; i < _n_mem; 
	     i++, item+=memsize,    _item+=memsize)
	    (*copyFunc)((void *)item, (void *)_item);
      } else
	memcpy(mem, _mem, _n_mem * memsize);
    }
  }

  /** set number of (re)allocated items */
  (*max_mem) = _n_mem;

  /** initialize newly allocated or freed items ... */
  if (initFunc) {
    int i = __max_mem;
    char *item = (char *)mem;
    for (item=item+i*memsize; i < _n_mem; i++,item+=memsize) 
      (*initFunc)((void *) item);
  } else {
    char *new_mem = (char *)mem;
    memset(new_mem+__max_mem*memsize,0,(_n_mem - __max_mem) * memsize);
  }

  /** return new memory */
  return (mem);
}
      
void *
_rag_set_memory_space(void *mem, int *max_mem, int *n_mem,
		      void *_mem, int _max_mem,
		      int _n_mem, size_t memsize,
		      rag_copyFunc *copyFunc, 
		      rag_initFunc *initFunc,
		      rag_freeFunc *freeFunc)
{
  /** set new number of items */
  if (n_mem) (*n_mem) = _n_mem;

  if (mem == _mem) return (mem);

  /** if _mem shall not be copied, ... */ 
  if (_max_mem >= -1) {

    /** take control on memory space, if _max_mem >= 0  */
    /** keep external control,        if _max_mem == -1 */

    /** if mem was previously allocated (no reference) free mem */
    _rag_free_memory_space(mem, max_mem, memsize, freeFunc);

    /** set new memory */
    mem = _mem;
    (*max_mem) = _max_mem;
  }
  /** else, _mem shall be copied ... */
  else {
    /** copy memory space,            if _max_mem <= -2 */
    
    mem = _rag_realloc_memory_space(mem, max_mem, _n_mem, memsize,
				    copyFunc, initFunc, freeFunc);
    if (copyFunc) {
      int i;
      char *item, *_item;
      for (i=0, item=(char *)mem, _item=(char *)_mem; i < _n_mem; 
	   i++, item+=memsize,    _item+=memsize)
	(*copyFunc)((void *)item, (void *)_item);
    } else
      memcpy(mem, _mem, _n_mem * memsize);
  }

  /** return new memory */
  return (mem);
}

void *
_rag_cpy_memory_space(void *mem, int *max_mem, int *n_mem,
		      void *_mem, int _max_mem,
		      int _n_mem, size_t memsize,
		      rag_copyFunc *copyFunc, 
		      rag_initFunc *initFunc,
		      rag_freeFunc *freeFunc)
{
  /** if memory to copy is no reference, _mem must be copied
      else, the reference on _mem is copied */

  return (_rag_set_memory_space(mem, max_mem, n_mem,
				_mem, (_max_mem > 0) ? -2 : -1, _n_mem, 
				memsize, copyFunc, initFunc, freeFunc));
}

void
_rag_free_memory_space(void *mem,              /* memory space to free */
		       int *max_mem,           /* if (>0) mem is freed */
		       size_t memsize,         /* sizeof items */
		       rag_freeFunc *freeFunc  /* use special free func */
		       )
{
  if (!mem || (max_mem && *max_mem <= 0)) return;

  if (freeFunc && max_mem ) {
    int i;
    char *_item = (char *) mem;
    for (i=0; i < (*max_mem); i++, _item+=memsize)
      (*freeFunc)(_item);
  }
  free(mem);
  if (max_mem) (*max_mem) = 0;
}

