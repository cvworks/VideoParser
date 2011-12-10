/*
 * region/_mem.h
 *
 * Sven Wachsmuth, 08.04.2003
 *
 * maintenance of memory arrays
 */
#ifndef _REGION_MEM_H
#define _REGION_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(rag_initFunc)(void *);
typedef void *(rag_copyFunc)(void *, void *);
typedef void  (rag_freeFunc)(void *);

void *                                 /* returns new allocated memory space */
_rag_realloc_memory_space(void *mem,    /* memory space to realloc */
			  int *max_mem, /* if (>0) mem is reallocated
					 * if (<=0) mem is newly allocated 
					 * is set to _n_mem afterwards */
			  int _n_mem,   /* new number of items to alloc */
			  size_t memsize, /* size of item */
			  rag_copyFunc *copyFunc, /* use special copy func */
			  rag_initFunc *initFunc, /* use special init func */
			  rag_freeFunc *freeFunc  /* free previous items */
			  );

void *                              /* returns newly set memory space */
_rag_set_memory_space(void *mem,    /* memory space to set */ 
		      int *max_mem, /* if (>0) mem is reallocated or freed 
				     * if (<0) mem is newly allocated or set 
				     * is set to number of allocated items
				     *   afterwards (or -1, if _max_mem==-1)*/
		      int *n_mem,   /* previous number of items 
				     * is set to _n_mem afterwards */
		      void *_mem,   /* new memory space */
		      int _max_mem, /* if (>=0) keep _mem, max_mem=_max_mem
				     * if (-1) reference _mem, max_mem=-1
				     * if (-2) copy _mem, max_mem>=_n_mem */
		      int _n_mem,     /* new number of items */
		      size_t memsize, /* size of items */
		      rag_copyFunc *copyFunc, /* use special copy func */
		      rag_initFunc *initFunc, /* use special init func */
		      rag_freeFunc *freeFunc  /* free previous items */
		      );

void *                              /* returns newly set memory space */
_rag_cpy_memory_space(void *mem,    /* memory space to set */
		      int *max_mem, /* if (>0) mem is reallocated or freed 
				     * if (<0) mem is newly allocated or set
				     * is set to number of allocated items
				     *   afterwards (or -1, if _max_mem==-1)*/
		      int *n_mem,   /* previous number of items 
				     * is set to _n_mem afterwards */
		      void *_mem,   /* memory space to copy */
		      int _max_mem, /* if (<=0) reference _mem, max_mem=-1
				     * if (>0) copy _mem, max_mem>=_n_mem */
		      int _n_mem,   /* number of items to copy */
		      size_t memsize, /* sizeof items */
		      rag_copyFunc *copyFunc, /* use special copy func */
		      rag_initFunc *initFunc, /* use special init func */
		      rag_freeFunc *freeFunc  /* free previous items */
		      );

void
_rag_free_memory_space(void *mem,              /* memory space to free */
		       int *max_mem,           /* if (>0) mem is freed */
		       size_t memsize,         /* sizeof items */
		       rag_freeFunc *freeFunc  /* use special free func */
		       );

#ifdef __cplusplus
}
#endif

#endif
