
#include <stdlib.h>
#include "tupleFloat.h"

#define TUPLE_IS_Float

pairFloat_t pairFloatNULL = {0,0};
tripleFloat_t tripleFloatNULL = {0,0,0};

pairFloat_t * pairFloat(float fst, float snd)
{
  pairFloat_t *p = (pairFloat_t *) malloc(sizeof(pairFloat_t));
  return pairFloat_set(p,fst,snd);
}

pairFloat_t * pairFloat_set(pairFloat_t *p, float fst, float snd)
{
  if (!p) return (pairFloat(fst,snd));

  p->first = fst;
  p->second = snd;

  return (p);
}

void pairFloat_destroy(pairFloat_t *p)
{
  if (!p) return;
  free(p);
}

pairFloat_t * pairFloat_cpy(pairFloat_t *p_to, pairFloat_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (pairFloat_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  return (p_to);
}

pairFloat_t * pairFloat_Cpy(pairFloat_t *p)
{
  if (!p) return (NULL);
  return (pairFloat(p->first,p->second));
}

pairFloat_t * pairFloat_map(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  return (p);
}

pairFloat_t * pairFloat_Map(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2)
{
  if (!p) return (NULL);
  return (pairFloat_mapSet(pairFloat_Cpy(p),f1,f2));
}

pairFloat_t * pairFloat_mapSet(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  return (p);
}

float pairFloat_getFst(pairFloat_t *p, float undef)
{
  return ((p) ? p->first : undef);
}

float pairFloat_getSnd(pairFloat_t *p, float undef)
{
  return ((p) ? p->second : undef);
}


float pairFloat_setFst(pairFloat_t *p, float fst)
{
  float tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
float pairFloat_setSnd(pairFloat_t *p, float snd)
{
  float tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void pairFloat_destroyFunc(pairFloat_t *p, tupleFloat_map_t *free1,tupleFloat_map_t *free2)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  pairFloat_destroy(p);
}
#endif

tripleFloat_t * tripleFloat(float fst, float snd, float trd)
{
  tripleFloat_t *p = (tripleFloat_t *) malloc(sizeof(tripleFloat_t));
  return tripleFloat_set(p,fst,snd,trd);
}

tripleFloat_t * tripleFloat_set(tripleFloat_t *p, float fst, float snd, 
		     float trd)
{
  if (!p) return (tripleFloat(fst,snd,trd));

  p->first  = fst;
  p->second = snd;
  p->third  = trd;

  return (p);
}

void tripleFloat_destroy(tripleFloat_t *p)
{
  if (!p) return;
  free(p);
}

tripleFloat_t * tripleFloat_cpy(tripleFloat_t *p_to, tripleFloat_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (tripleFloat_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  p_to->third  = p_from->third;
  return (p_to);
}

tripleFloat_t * tripleFloat_Cpy(tripleFloat_t *p)
{
  if (!p) return (NULL);
  return (tripleFloat(p->first,p->second,p->third));
}

tripleFloat_t * tripleFloat_map(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
		       tupleFloat_map_t *f3)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  if (f3)
      (*f3)(p->third);
  return (p);
}

tripleFloat_t * tripleFloat_Map(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
		       tupleFloat_map_t *f3)
{
  if (!p) return (NULL);
  return (tripleFloat_mapSet(tripleFloat_Cpy(p),f1,f2,f3));
}

tripleFloat_t * tripleFloat_mapSet(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
			  tupleFloat_map_t *f3)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  if (f3)
      p->third = (*f3)(p->third);
  return (p);
}

float tripleFloat_getFst(tripleFloat_t *p, float undef)
{
  return ((p) ? p->first : undef);
}

float tripleFloat_getSnd(tripleFloat_t *p, float undef)
{
  return ((p) ? p->second : undef);
}

float tripleFloat_getTrd(tripleFloat_t *p, float undef)
{
  return ((p) ? p->third : undef);
}


float tripleFloat_setFst(tripleFloat_t *p, float fst)
{
  float tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
float tripleFloat_setSnd(tripleFloat_t *p, float snd)
{
  float tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

float tripleFloat_setTrd(tripleFloat_t *p, float trd)
{
  float tmp;
  if (!p) return (trd);
  tmp = p->third;
  p->third = trd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void tripleFloat_destroyFunc(tripleFloat_t *p, tupleFloat_map_t *free1,
			 tupleFloat_map_t *free2, tupleFloat_map_t *free3)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  if (free3) (*free3)(p->third);
  tripleFloat_destroy(p);
}
#endif




