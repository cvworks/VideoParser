
#include <stdlib.h>
#include "tupleLong.h"

#define TUPLE_IS_Long

pairLong_t pairLongNULL = {0,0};
tripleLong_t tripleLongNULL = {0,0,0};

pairLong_t * pairLong(long int fst, long int snd)
{
  pairLong_t *p = (pairLong_t *) malloc(sizeof(pairLong_t));
  return pairLong_set(p,fst,snd);
}

pairLong_t * pairLong_set(pairLong_t *p, long int fst, long int snd)
{
  if (!p) return (pairLong(fst,snd));

  p->first = fst;
  p->second = snd;

  return (p);
}

void pairLong_destroy(pairLong_t *p)
{
  if (!p) return;
  free(p);
}

pairLong_t * pairLong_cpy(pairLong_t *p_to, pairLong_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (pairLong_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  return (p_to);
}

pairLong_t * pairLong_Cpy(pairLong_t *p)
{
  if (!p) return (NULL);
  return (pairLong(p->first,p->second));
}

pairLong_t * pairLong_map(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  return (p);
}

pairLong_t * pairLong_Map(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2)
{
  if (!p) return (NULL);
  return (pairLong_mapSet(pairLong_Cpy(p),f1,f2));
}

pairLong_t * pairLong_mapSet(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  return (p);
}

long int pairLong_getFst(pairLong_t *p, long int undef)
{
  return ((p) ? p->first : undef);
}

long int pairLong_getSnd(pairLong_t *p, long int undef)
{
  return ((p) ? p->second : undef);
}


long int pairLong_setFst(pairLong_t *p, long int fst)
{
  long int tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
long int pairLong_setSnd(pairLong_t *p, long int snd)
{
  long int tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void pairLong_destroyFunc(pairLong_t *p, tupleLong_map_t *free1,tupleLong_map_t *free2)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  pairLong_destroy(p);
}
#endif

tripleLong_t * tripleLong(long int fst, long int snd, long int trd)
{
  tripleLong_t *p = (tripleLong_t *) malloc(sizeof(tripleLong_t));
  return tripleLong_set(p,fst,snd,trd);
}

tripleLong_t * tripleLong_set(tripleLong_t *p, long int fst, long int snd, 
		     long int trd)
{
  if (!p) return (tripleLong(fst,snd,trd));

  p->first  = fst;
  p->second = snd;
  p->third  = trd;

  return (p);
}

void tripleLong_destroy(tripleLong_t *p)
{
  if (!p) return;
  free(p);
}

tripleLong_t * tripleLong_cpy(tripleLong_t *p_to, tripleLong_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (tripleLong_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  p_to->third  = p_from->third;
  return (p_to);
}

tripleLong_t * tripleLong_Cpy(tripleLong_t *p)
{
  if (!p) return (NULL);
  return (tripleLong(p->first,p->second,p->third));
}

tripleLong_t * tripleLong_map(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
		       tupleLong_map_t *f3)
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

tripleLong_t * tripleLong_Map(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
		       tupleLong_map_t *f3)
{
  if (!p) return (NULL);
  return (tripleLong_mapSet(tripleLong_Cpy(p),f1,f2,f3));
}

tripleLong_t * tripleLong_mapSet(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
			  tupleLong_map_t *f3)
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

long int tripleLong_getFst(tripleLong_t *p, long int undef)
{
  return ((p) ? p->first : undef);
}

long int tripleLong_getSnd(tripleLong_t *p, long int undef)
{
  return ((p) ? p->second : undef);
}

long int tripleLong_getTrd(tripleLong_t *p, long int undef)
{
  return ((p) ? p->third : undef);
}


long int tripleLong_setFst(tripleLong_t *p, long int fst)
{
  long int tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
long int tripleLong_setSnd(tripleLong_t *p, long int snd)
{
  long int tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

long int tripleLong_setTrd(tripleLong_t *p, long int trd)
{
  long int tmp;
  if (!p) return (trd);
  tmp = p->third;
  p->third = trd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void tripleLong_destroyFunc(tripleLong_t *p, tupleLong_map_t *free1,
			 tupleLong_map_t *free2, tupleLong_map_t *free3)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  if (free3) (*free3)(p->third);
  tripleLong_destroy(p);
}
#endif




