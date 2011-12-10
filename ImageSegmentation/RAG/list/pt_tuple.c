
#include <stdlib.h>
#include "tuplePt.h"

#define TUPLE_IS_Pt

pairPt_t pairPtNULL = {0,0};
triplePt_t triplePtNULL = {0,0,0};

pairPt_t * pairPt(void * fst, void * snd)
{
  pairPt_t *p = (pairPt_t *) malloc(sizeof(pairPt_t));
  return pairPt_set(p,fst,snd);
}

pairPt_t * pairPt_set(pairPt_t *p, void * fst, void * snd)
{
  if (!p) return (pairPt(fst,snd));

  p->first = fst;
  p->second = snd;

  return (p);
}

void pairPt_destroy(pairPt_t *p)
{
  if (!p) return;
  free(p);
}

pairPt_t * pairPt_cpy(pairPt_t *p_to, pairPt_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (pairPt_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  return (p_to);
}

pairPt_t * pairPt_Cpy(pairPt_t *p)
{
  if (!p) return (NULL);
  return (pairPt(p->first,p->second));
}

pairPt_t * pairPt_map(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  return (p);
}

pairPt_t * pairPt_Map(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2)
{
  if (!p) return (NULL);
  return (pairPt_mapSet(pairPt_Cpy(p),f1,f2));
}

pairPt_t * pairPt_mapSet(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  return (p);
}

void * pairPt_getFst(pairPt_t *p, void * undef)
{
  return ((p) ? p->first : undef);
}

void * pairPt_getSnd(pairPt_t *p, void * undef)
{
  return ((p) ? p->second : undef);
}


void * pairPt_setFst(pairPt_t *p, void * fst)
{
  void * tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
void * pairPt_setSnd(pairPt_t *p, void * snd)
{
  void * tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void pairPt_destroyFunc(pairPt_t *p, tuplePt_map_t *free1,tuplePt_map_t *free2)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  pairPt_destroy(p);
}
#endif

triplePt_t * triplePt(void * fst, void * snd, void * trd)
{
  triplePt_t *p = (triplePt_t *) malloc(sizeof(triplePt_t));
  return triplePt_set(p,fst,snd,trd);
}

triplePt_t * triplePt_set(triplePt_t *p, void * fst, void * snd, 
		     void * trd)
{
  if (!p) return (triplePt(fst,snd,trd));

  p->first  = fst;
  p->second = snd;
  p->third  = trd;

  return (p);
}

void triplePt_destroy(triplePt_t *p)
{
  if (!p) return;
  free(p);
}

triplePt_t * triplePt_cpy(triplePt_t *p_to, triplePt_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (triplePt_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  p_to->third  = p_from->third;
  return (p_to);
}

triplePt_t * triplePt_Cpy(triplePt_t *p)
{
  if (!p) return (NULL);
  return (triplePt(p->first,p->second,p->third));
}

triplePt_t * triplePt_map(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
		       tuplePt_map_t *f3)
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

triplePt_t * triplePt_Map(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
		       tuplePt_map_t *f3)
{
  if (!p) return (NULL);
  return (triplePt_mapSet(triplePt_Cpy(p),f1,f2,f3));
}

triplePt_t * triplePt_mapSet(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
			  tuplePt_map_t *f3)
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

void * triplePt_getFst(triplePt_t *p, void * undef)
{
  return ((p) ? p->first : undef);
}

void * triplePt_getSnd(triplePt_t *p, void * undef)
{
  return ((p) ? p->second : undef);
}

void * triplePt_getTrd(triplePt_t *p, void * undef)
{
  return ((p) ? p->third : undef);
}


void * triplePt_setFst(triplePt_t *p, void * fst)
{
  void * tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
void * triplePt_setSnd(triplePt_t *p, void * snd)
{
  void * tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

void * triplePt_setTrd(triplePt_t *p, void * trd)
{
  void * tmp;
  if (!p) return (trd);
  tmp = p->third;
  p->third = trd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void triplePt_destroyFunc(triplePt_t *p, tuplePt_map_t *free1,
			 tuplePt_map_t *free2, tuplePt_map_t *free3)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  if (free3) (*free3)(p->third);
  triplePt_destroy(p);
}
#endif




