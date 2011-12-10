
#include <stdlib.h>
#include "tupleInt.h"

#define TUPLE_IS_Int

pairInt_t pairIntNULL = {0,0};
tripleInt_t tripleIntNULL = {0,0,0};

pairInt_t * pairInt(int fst, int snd)
{
  pairInt_t *p = (pairInt_t *) malloc(sizeof(pairInt_t));
  return pairInt_set(p,fst,snd);
}

pairInt_t * pairInt_set(pairInt_t *p, int fst, int snd)
{
  if (!p) return (pairInt(fst,snd));

  p->first = fst;
  p->second = snd;

  return (p);
}

void pairInt_destroy(pairInt_t *p)
{
  if (!p) return;
  free(p);
}

pairInt_t * pairInt_cpy(pairInt_t *p_to, pairInt_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (pairInt_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  return (p_to);
}

pairInt_t * pairInt_Cpy(pairInt_t *p)
{
  if (!p) return (NULL);
  return (pairInt(p->first,p->second));
}

pairInt_t * pairInt_map(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  return (p);
}

pairInt_t * pairInt_Map(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2)
{
  if (!p) return (NULL);
  return (pairInt_mapSet(pairInt_Cpy(p),f1,f2));
}

pairInt_t * pairInt_mapSet(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  return (p);
}

int pairInt_getFst(pairInt_t *p, int undef)
{
  return ((p) ? p->first : undef);
}

int pairInt_getSnd(pairInt_t *p, int undef)
{
  return ((p) ? p->second : undef);
}


int pairInt_setFst(pairInt_t *p, int fst)
{
  int tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
int pairInt_setSnd(pairInt_t *p, int snd)
{
  int tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void pairInt_destroyFunc(pairInt_t *p, tupleInt_map_t *free1,tupleInt_map_t *free2)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  pairInt_destroy(p);
}
#endif

tripleInt_t * tripleInt(int fst, int snd, int trd)
{
  tripleInt_t *p = (tripleInt_t *) malloc(sizeof(tripleInt_t));
  return tripleInt_set(p,fst,snd,trd);
}

tripleInt_t * tripleInt_set(tripleInt_t *p, int fst, int snd, 
		     int trd)
{
  if (!p) return (tripleInt(fst,snd,trd));

  p->first  = fst;
  p->second = snd;
  p->third  = trd;

  return (p);
}

void tripleInt_destroy(tripleInt_t *p)
{
  if (!p) return;
  free(p);
}

tripleInt_t * tripleInt_cpy(tripleInt_t *p_to, tripleInt_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (tripleInt_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  p_to->third  = p_from->third;
  return (p_to);
}

tripleInt_t * tripleInt_Cpy(tripleInt_t *p)
{
  if (!p) return (NULL);
  return (tripleInt(p->first,p->second,p->third));
}

tripleInt_t * tripleInt_map(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
		       tupleInt_map_t *f3)
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

tripleInt_t * tripleInt_Map(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
		       tupleInt_map_t *f3)
{
  if (!p) return (NULL);
  return (tripleInt_mapSet(tripleInt_Cpy(p),f1,f2,f3));
}

tripleInt_t * tripleInt_mapSet(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
			  tupleInt_map_t *f3)
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

int tripleInt_getFst(tripleInt_t *p, int undef)
{
  return ((p) ? p->first : undef);
}

int tripleInt_getSnd(tripleInt_t *p, int undef)
{
  return ((p) ? p->second : undef);
}

int tripleInt_getTrd(tripleInt_t *p, int undef)
{
  return ((p) ? p->third : undef);
}


int tripleInt_setFst(tripleInt_t *p, int fst)
{
  int tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
int tripleInt_setSnd(tripleInt_t *p, int snd)
{
  int tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

int tripleInt_setTrd(tripleInt_t *p, int trd)
{
  int tmp;
  if (!p) return (trd);
  tmp = p->third;
  p->third = trd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void tripleInt_destroyFunc(tripleInt_t *p, tupleInt_map_t *free1,
			 tupleInt_map_t *free2, tupleInt_map_t *free3)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  if (free3) (*free3)(p->third);
  tripleInt_destroy(p);
}
#endif




