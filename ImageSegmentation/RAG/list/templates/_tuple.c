
#include <stdlib.h>
#include "_tuple.h"

#define TUPLE_IS_TYPE

pair_t _pairNULL = {0,0};
triple_t _tripleNULL = {0,0,0};

pair_t * _pair(TUPLE_TYPE fst, TUPLE_TYPE snd)
{
  pair_t *p = (pair_t *) malloc(sizeof(pair_t));
  return _pair_set(p,fst,snd);
}

pair_t * _pair_set(pair_t *p, TUPLE_TYPE fst, TUPLE_TYPE snd)
{
  if (!p) return (_pair(fst,snd));

  p->first = fst;
  p->second = snd;

  return (p);
}

void _pair_destroy(pair_t *p)
{
  if (!p) return;
  free(p);
}

pair_t * _pair_cpy(pair_t *p_to, pair_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (_pair_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  return (p_to);
}

pair_t * _pair_Cpy(pair_t *p)
{
  if (!p) return (NULL);
  return (_pair(p->first,p->second));
}

pair_t * _pair_map(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2)
{
  if (!p) return (NULL);
  if (f1)
      (*f1)(p->first);
  if (f2)
      (*f2)(p->second);
  return (p);
}

pair_t * _pair_Map(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2)
{
  if (!p) return (NULL);
  return (_pair_mapSet(_pair_Cpy(p),f1,f2));
}

pair_t * _pair_mapSet(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2)
{
  if (!p) return (NULL);
  if (f1) 
      p->first = (*f1)(p->first);
  if (f2)
      p->second = (*f2)(p->second);
  return (p);
}

TUPLE_TYPE _pair_getFst(pair_t *p, TUPLE_TYPE undef)
{
  return ((p) ? p->first : undef);
}

TUPLE_TYPE _pair_getSnd(pair_t *p, TUPLE_TYPE undef)
{
  return ((p) ? p->second : undef);
}


TUPLE_TYPE _pair_setFst(pair_t *p, TUPLE_TYPE fst)
{
  TUPLE_TYPE tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
TUPLE_TYPE _pair_setSnd(pair_t *p, TUPLE_TYPE snd)
{
  TUPLE_TYPE tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void _pair_destroyFunc(pair_t *p, TUPLE_MAP_TYPE *free1,TUPLE_MAP_TYPE *free2)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  _pair_destroy(p);
}
#endif

triple_t * _triple(TUPLE_TYPE fst, TUPLE_TYPE snd, TUPLE_TYPE trd)
{
  triple_t *p = (triple_t *) malloc(sizeof(triple_t));
  return _triple_set(p,fst,snd,trd);
}

triple_t * _triple_set(triple_t *p, TUPLE_TYPE fst, TUPLE_TYPE snd, 
		     TUPLE_TYPE trd)
{
  if (!p) return (_triple(fst,snd,trd));

  p->first  = fst;
  p->second = snd;
  p->third  = trd;

  return (p);
}

void _triple_destroy(triple_t *p)
{
  if (!p) return;
  free(p);
}

triple_t * _triple_cpy(triple_t *p_to, triple_t *p_from)
{
  if (!p_from) return (p_to);
  if (!p_to) {
      return (_triple_Cpy(p_from));
  }
  p_to->first  = p_from->first;
  p_to->second = p_from->second;
  p_to->third  = p_from->third;
  return (p_to);
}

triple_t * _triple_Cpy(triple_t *p)
{
  if (!p) return (NULL);
  return (_triple(p->first,p->second,p->third));
}

triple_t * _triple_map(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
		       TUPLE_MAP_TYPE *f3)
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

triple_t * _triple_Map(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
		       TUPLE_MAP_TYPE *f3)
{
  if (!p) return (NULL);
  return (_triple_mapSet(_triple_Cpy(p),f1,f2,f3));
}

triple_t * _triple_mapSet(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
			  TUPLE_MAP_TYPE *f3)
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

TUPLE_TYPE _triple_getFst(triple_t *p, TUPLE_TYPE undef)
{
  return ((p) ? p->first : undef);
}

TUPLE_TYPE _triple_getSnd(triple_t *p, TUPLE_TYPE undef)
{
  return ((p) ? p->second : undef);
}

TUPLE_TYPE _triple_getTrd(triple_t *p, TUPLE_TYPE undef)
{
  return ((p) ? p->third : undef);
}


TUPLE_TYPE _triple_setFst(triple_t *p, TUPLE_TYPE fst)
{
  TUPLE_TYPE tmp;
  if (!p) return (fst);
  tmp = p->first;
  p->first = fst;
  return (tmp);
}
  
TUPLE_TYPE _triple_setSnd(triple_t *p, TUPLE_TYPE snd)
{
  TUPLE_TYPE tmp;
  if (!p) return (snd);
  tmp = p->second;
  p->first = snd;
  return (tmp);
}

TUPLE_TYPE _triple_setTrd(triple_t *p, TUPLE_TYPE trd)
{
  TUPLE_TYPE tmp;
  if (!p) return (trd);
  tmp = p->third;
  p->third = trd;
  return (tmp);
}

#if defined(TUPLE_IS_Pt)
void _triple_destroyFunc(triple_t *p, TUPLE_MAP_TYPE *free1,
			 TUPLE_MAP_TYPE *free2, TUPLE_MAP_TYPE *free3)
{
  if (!p) return;
  if (free1) (*free1)(p->first);
  if (free2) (*free2)(p->second);
  if (free3) (*free3)(p->third);
  _triple_destroy(p);
}
#endif




