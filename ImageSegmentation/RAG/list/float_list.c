/*
 * Datei:        list.c
 * Autor:        Sven Wachsmuth
 * Datum:        5.2.99
 *
 * list of integers
 */

/*
 * Typ der Liste wird per (sed-cmd) gesetzt
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rs/memory.h>
#include <rs/messages.h>

#include "string+.h"
#include "tuplePt.h"
#include "tuple_macro.h"
#include "lsFloat.h"
#include "list_macro.h"

#if defined(LS_IS_lsInt) \
    || defined(LS_IS_lsULInt) \
    || defined(LS_IS_lsLInt) \
    || defined(LS_IS_lsChar) \
    || defined(LS_IS_lsFloat) \
    || defined(LS_IS_lsDouble) \
    || defined(LS_IS_lsPt)
#define LS_IS_lvalue
#endif

#if !defined(LS_IS_lsInt)
void _lsInt_qsortX_2(int *v, int left, int right, 
                     int mode, lsInt_cmp_2_t *func, void *arg);
#endif

#define LIST_BUF 32

lsFloat_t * lsFloat_Nil(void)
{
  return (lsFloat_nil(NULL));
}

lsFloat_t * lsFloat_realloc(lsFloat_t *il, int n)
{
  if (!il) il = lsFloat_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(float), 
			  "list items");
  }
  return (il);
}

lsFloat_t *  lsFloat_nil(lsFloat_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsFloat_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsFloat_t * lsFloat_ConsNil(float i)
{
  lsFloat_t * il = rs_malloc(sizeof(lsFloat_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(float), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

float lsFloat_setIndex(lsFloat_t *il, int index, float i, float i0)
{
  float ret;
  int j;

  if (!il)
    rs_error("lsFloat_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsFloat_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(float), 
			  "list items");
  }
  for (j=il->n_list; j <= index; j++) 
    il->list[j] = i0;

  ret = il->list[index];

  il->list[index] = i;
  if (il->n_list <= index)
    il->n_list = index+1;

  return (ret);
}


lsFloat_t * lsFloat_setIndices(lsFloat_t *il, lsInt_t *indices, float x,
		       float undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsFloat_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsFloat_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsFloat_t * lsFloat_setNil(lsFloat_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsFloat_t * lsFloat_nsetIndex(lsFloat_t *il, int index, int n, float x, 
		      float undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsFloat_Nil();

  lsFloat_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsFloat_t * lsFloat_setConsNil(lsFloat_t * il, float i)
{
  if (!il)
    return lsFloat_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(float), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsFloat_getNewItemIndex(lsFloat_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsFloat_getNewItem(il);
  return (index);
}

float *lsFloat_getNewItem(lsFloat_t *il)
{
  float *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(float), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsFloat_t * lsFloat_add(lsFloat_t * il, float i)
{
  if (!il)
    il = lsFloat_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(float), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsFloat_t * lsFloat_Add(lsFloat_t * il, float i)
{
  lsFloat_t *il_to = lsFloat_Nil();
  if (!il)
    return (lsFloat_setConsNil(il_to,i));

  lsFloat_realloc(il_to, il->n_list+LIST_BUF);
  lsFloat_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsFloat_t * lsFloat_Cons(lsFloat_t * il, float i)
{
  lsFloat_t *il_to = lsFloat_Nil();
  if (!il)
    return (lsFloat_setConsNil(il_to,i));

  lsFloat_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(float)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsFloat_t * lsFloat_cons(lsFloat_t *il, float i)
{
  if (!il)
    il = lsFloat_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(float), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(float));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

float lsFloat_last(lsFloat_t *il, float undef)
{
  return (LS_LAST_CHECK(il,undef));
}

float lsFloat_head(lsFloat_t *il, float undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

float lsFloat_popLast(lsFloat_t *il, float undef)
{
  float x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsFloat_init(il);
  return (x);
}

float lsFloat_popHead(lsFloat_t *il, float undef)
{
  float x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsFloat_tail(il);
  return (x);
}

lsFloat_t * lsFloat_init(lsFloat_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsFloat_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsFloat_t * lsFloat_Init(lsFloat_t * il)
{
  lsFloat_t *il_to = lsFloat_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsFloat_Init: got empty list");
  }
  lsFloat_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsFloat_t * lsFloat_tail(lsFloat_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsFloat_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(float));
  return (il);
}
 
lsFloat_t * lsFloat_Tail(lsFloat_t *il)
{
  lsFloat_t *il_to = lsFloat_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsFloat_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsFloat_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(float));
  return (il_to);
}


lsFloat_t * lsFloat_take(lsFloat_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsFloat_t * lsFloat_Take(lsFloat_t *il, int n)
{
  lsFloat_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsFloat_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(float));
  LS_N(il_to) = m;
  return (il_to);
}

float lsFloat_delSwap(lsFloat_t *il, int i)
{
  float x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

float lsFloat_delete(lsFloat_t *il, int index, float undef)
{
  float x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(float));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsFloat_Free(lsFloat_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsFloat_free(lsFloat_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;
}

float lsFloat_get(lsFloat_t * il,int i)
{
  if (!il)
    rs_error("lsFloat_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsFloat_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

float lsFloat_getCheck(lsFloat_t * il,int i,float undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsFloat_length(lsFloat_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsFloat_t *  lsFloat_getRowPt(lsFloat_t * row, lsFloat_t * il, int i, int cols)
{
  row = lsFloat_take(lsFloat_dropPt(lsFloat_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsFloat_t *  lsFloat_getRow(lsFloat_t *row, lsFloat_t *il, int i, int cols)
{
  lsFloat_t rowPt;
  lsFloat_getRowPt(lsFloat_init(&rowPt), il, i, cols);

  lsFloat_cpy(row, &rowPt);

  return (row);
}

lsFloat_t *  lsFloat_getCol(lsFloat_t *col, lsFloat_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  float *item;

  if (n <= j)
    return (lsFloat_setNil(col));

  col = lsFloat_realloc(col, n / cols + 1);
  lsFloat_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsFloat_add(col,*item);

  return (col);
}

lsFloat_t *  lsFloat_setRow(lsFloat_t *il, lsFloat_t *row, int i, int cols)
{
  lsFloat_t rowPt;
  int n;
  lsFloat_getRowPt(lsFloat_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsFloat_cpy(&rowPt,lsFloat_take(row,n));
  return (il);
}

lsFloat_t *  lsFloat_setCol(lsFloat_t *il, lsFloat_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  float *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsFloat_t *  lsFloat_SetPt(int n, float * items)
{
  return (lsFloat_setPt(NULL,n,items));
}

lsFloat_t *  lsFloat_setPt(lsFloat_t * il_to, int n, float * items)
{
  if (!il_to)
    il_to = lsFloat_Nil();
  lsFloat_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsFloat_t *  lsFloat_CpyPt(lsFloat_t * il_from)
{
  return (lsFloat_cpyPt(NULL, il_from));
}

lsFloat_t *  lsFloat_cpyPt(lsFloat_t * il_to, lsFloat_t * il_from)
{
  if (!il_to)
    il_to = lsFloat_Nil();
  lsFloat_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsFloat_t * lsFloat_Cpy(const lsFloat_t * il_from)
{
  return (lsFloat_cpy(NULL,il_from));
}

lsFloat_t * lsFloat_cpy(lsFloat_t * il_to, const lsFloat_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (!il_from) return (il_to);

  lsFloat_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(float));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsFloat_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsFloat_t * lsFloat_Cat(lsFloat_t * il_1, lsFloat_t * il_2)
{
  return (lsFloat_cat(lsFloat_Cpy(il_1), il_2));
}

lsFloat_t * lsFloat_cat(lsFloat_t * il_to, lsFloat_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsFloat_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsFloat_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(float));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsFloat_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsFloat_t * lsFloat_addCat(lsFloat_t *il_to, lsFloat_t *il)
{
  return lsFloat_add(il_to, lsFloat_Cat(il_to,il));
}

lsFloat_t * lsFloat_AddCat(lsFloat_t *il_to, lsFloat_t *il)
{
  return lsFloat_Add(il_to, lsFloat_Cat(il_to,il));
}
#endif

lsFloat_t * lsFloat_drop(lsFloat_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsFloat_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(float));
  return (il);
}

lsFloat_t * lsFloat_Drop(lsFloat_t *il, int i)
{
  lsFloat_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsFloat_setPt(lsFloat_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsFloat_Cpy(&tmp);
  return (il_split);
}

lsFloat_t * lsFloat_dropPt(lsFloat_t *il, int i)
{
  if (!il) return (NULL);
  if (LS_N(il) <= i) {
    LS_ITEMS(il) += LS_N(il);
    LS_N(il) = 0;
  } else {
    LS_ITEMS(il) += i;
    LS_N(il) -= i;
  }
  return (il);
}

lsFloat_t * lsFloat_split(lsFloat_t *il, int i)
{
  lsFloat_t *il_drop = lsFloat_Drop(il,i);
  lsFloat_take(il,i);
  return (il_drop);
}

lsPt_t * lsFloat_nsplit(lsPt_t *il_split, lsFloat_t *il, lsInt_t *is)
{
  int i;
  lsPt_setNil(il_split);
  if (LS_isNIL(il))
      return (il_split);
  if (!il_split)
      il_split = lsPt_Nil();

  if (LS_isNIL(is))
      return (lsPt_add(il_split,il));

  LS_FORALL_ITEMS_REV(is,i) {
      int index = LS_GET(is,i);
      lsFloat_t *split = lsFloat_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsFloat_t *lsFloat_range(lsFloat_t *il, int i0, int iN)
{
  lsFloat_take(il,iN);
  if (i0 > 0) {
    lsFloat_drop(il,i0-1);
  }
  return (il);
}

lsFloat_t *lsFloat_Range(lsFloat_t *il, int i0, int iN)
{
  int n;
  lsFloat_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsFloat_Drop(il,i0);
  return (lsFloat_take(il_to,iN));
}

lsFloat_t *lsFloat_rangePt(lsFloat_t *il, int i0, int iN)
{
  return (lsFloat_dropPt(lsFloat_take(il,iN),i0));
}

lsFloat_t *lsFloat_cpyRange(lsFloat_t *il_to, lsFloat_t *il, int i0, int iN)
{
  lsFloat_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsFloat_rangePt(lsFloat_cpyPt(lsFloat_nil(&tmp),il),i0,iN);
  return (lsFloat_cpy(il_to,&tmp));
}

lsFloat_t *lsFloat_catRange(lsFloat_t *il_to, lsFloat_t *il, int i0, int iN)
{
  lsFloat_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsFloat_rangePt(lsFloat_cpyPt(lsFloat_nil(&tmp),il),i0,iN);
  return (lsFloat_cat(il_to,&tmp));
}
  
lsFloat_t *lsFloat_reverse(lsFloat_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    float tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsFloat_makeIndex(lsInt_t *index, lsFloat_t *il)
{
  int i;
  if (LS_isNIL(il))
    return (index);
  if (index)
    lsInt_setNil(index);
  else
    index = lsInt_Nil();

  LS_FORALL_ITEMS(il,i)
    lsInt_setIndex(index,LS_GET(il,i),i,-1);
  return (index);
}
#endif

#if defined(LS_IS_lvalue)
lsFloat_t * lsFloat_join(lsFloat_t *il_to, lsFloat_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsFloat_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    float x = LS_GET(il_from,i);
    if (lsFloat_elem(il_to,x)) continue;

    lsFloat_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsFloat_t * lsFloat_filterByValue(lsFloat_t *il, float undef, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  
  if (indices) lsInt_setNil(indices);
  j=0; LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != undef) {
      LS_GET(il,j++) = LS_GET(il,i);
      if (indices) lsInt_add(indices,i);
    }
  }
  LS_N(il) = j;

  return (il);
}

lsFloat_t * lsFloat_CpyFilterByValue(lsFloat_t *il, float undef, lsInt_t *indices)
{
  return (lsFloat_cpyFilterByValue(NULL,il,undef,indices));
}

lsFloat_t * lsFloat_cpyFilterByValue(lsFloat_t *il_to, lsFloat_t *il_from, float undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsFloat_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsFloat_t * lsFloat_filterByIndex(lsFloat_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsFloat_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsFloat_t * lsFloat_CpyFilterByIndex(lsFloat_t *il, lsInt_t *indices)
{
  return (lsFloat_cpyFilterByIndex(NULL,il,indices));
}

lsFloat_t * lsFloat_cpyFilterByIndex(lsFloat_t *il_to, lsFloat_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsFloat_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsFloat_t * lsFloat_joinInts(lsFloat_t *il_to, lsFloat_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsFloat_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsFloat_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsFloat_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsFloat_elem(lsFloat_t * il, float item)
{
  return (lsFloat_getLastIndex(il,item) >= 0);
}

int lsFloat_getLastIndex(lsFloat_t *il, float item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsFloat_getFstIndex(lsFloat_t *il, float item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsFloat_getIndex(lsFloat_t * il, float item)
{
  return (lsFloat_getLastIndex(il,item));
}

int lsFloat_neqElem(lsFloat_t * il, float item)
{
  return (lsFloat_getLastNeqIndex(il,item) >= 0);
}

int lsFloat_getLastNeqIndex(lsFloat_t * il, float item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsFloat_getFstNeqIndex(lsFloat_t * il, float item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsFloat_disjoint(lsFloat_t *il1, lsFloat_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsFloat_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsFloat_t *lsFloat_subst(lsFloat_t *il, float i, float j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsFloat_subBag(lsFloat_t *il_sub, lsFloat_t *il_super, float undef)
{
  return lsFloat_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsFloat_subBagIndices(lsInt_t *indices,
		     lsFloat_t *il_sub, lsFloat_t *il_super, float undef)
{
  lsFloat_t _sub;
  lsFloat_t _super;
  int i;
  float last;
  float item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsFloat_qsortLt(lsFloat_cpy(lsFloat_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsFloat_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsFloat_take(&_super,last_index);
      if ((last_index = lsFloat_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsFloat_cpyPt(lsFloat_nil(&_super),il_super);

      if ((last_index = lsFloat_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsFloat_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsFloat_subBagLimitedIndices(lsInt_t *indices,
			    lsFloat_t *il_sub, lsFloat_t *il_super, 
			    lsInt_t *limit, float undef)
{
  lsFloat_t _sub;
  lsFloat_t _super;
  int i;
  float last;
  float item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsFloat_qsortLt(lsFloat_cpy(lsFloat_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsFloat_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsFloat_take(&_super,last_index);
      if ((last_index = lsFloat_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsFloat_cpyPt(lsFloat_nil(&_super),il_super);
      if (limit) lsFloat_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsFloat_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsFloat_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsFloat_elemFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func)
{
 if (!func) rs_error("lsFloat_elemFunc: cmp-function undefined.");
 return (lsFloat_getIndexFunc(il,item,func) >= 0);
}

int lsFloat_getIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsFloat_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsFloat_neqElemFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func)
{
  if (!func) rs_error("lsFloat_neqElemFunc: cmp-function undefined.");
  return (lsFloat_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsFloat_getLastNeqIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsFloat_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsFloat_getFstNeqIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsFloat_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsFloat_t * lsFloat_insert(lsFloat_t *il, int index, float item, float item0)
{
  int i;
  if (!il)
    il = lsFloat_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsFloat_insert: illegal index %d.",index);

    lsFloat_setIndex(il,index,item,item0);
    return (il);
  }
  lsFloat_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(float));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsFloat_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsFloat_t * lsFloat_insertN(lsFloat_t *il, int index, int n, float item, 
		    float item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsFloat_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsFloat_insert: illegal index %d.",index);

    lsFloat_setIndex(il,index,item,item0);
    lsFloat_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsFloat_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(float));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsFloat_t * lsFloat_insSwap(lsFloat_t *il, int index, float item, float item0)
{
  int i,n;
  float _item;

  if (!il)
    il = lsFloat_Nil();

  if (index < 0)
    rs_error("lsFloat_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsFloat_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsFloat_add(il,_item);
#else
  if (index < n)
    lsFloat_add(il,_item);
#endif

  return (il);
}

float lsFloat_getFlip(int i, lsFloat_t *il, float undef)
{
  float item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsFloat_t * lsFloat_map(lsFloat_t * il, lsFloat_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsFloat_t * lsFloat_map_2(lsFloat_t * il, lsFloat_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsFloat_t * lsFloat_map_3(lsFloat_t * il, lsFloat_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsFloat_t * lsFloat_mapSet(lsFloat_t * il, lsFloat_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsFloat_t * lsFloat_mapSet_2(lsFloat_t * il, lsFloat_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsFloat_t * lsFloat_mapSet_3(lsFloat_t * il, lsFloat_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsFloat_t * lsFloat_CpyMap(lsFloat_t * il_from, lsFloat_map_t *func)
{
  return (lsFloat_cpyMap(NULL,il_from,func));
}

lsFloat_t * lsFloat_cpyMap(lsFloat_t * il_to, lsFloat_t * il_from, lsFloat_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsFloat_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsFloat_t *  lsFloat_CpyMap_2(lsFloat_t * il_from, lsFloat_map_2_t *func, void *arg)
{
  return (lsFloat_cpyMap_2(NULL, il_from, func, arg));
}

lsFloat_t *  lsFloat_CpyMap_3(lsFloat_t * il_from, lsFloat_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsFloat_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsFloat_t *  lsFloat_cpyMap_2(lsFloat_t * il_to, lsFloat_t * il_from, 
		      lsFloat_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsFloat_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsFloat_t *  lsFloat_cpyMap_3(lsFloat_t * il_to, lsFloat_t * il_from, 
		      lsFloat_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsFloat_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsFloat_t * lsFloat_CartProd(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func)
{
  return (lsFloat_cpyCartProd(NULL,il1,il2,func));
}

lsFloat_t * lsFloat_cpyCartProd(lsFloat_t *il_to, 
			lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsFloat_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsFloat_cpy(il_to,il1);
  if (!il_to)
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsFloat_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsFloat_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsFloat_t * lsFloat_cartProd(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func)
{
  int i,j;
  lsFloat_t tmp;
  if (LS_isNIL(il1)) return lsFloat_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsFloat_cpy(lsFloat_nil(&tmp),il1);
  lsFloat_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsFloat_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsFloat_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsFloat_t * lsFloat_cartProd_2(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsFloat_t tmp;
  if (LS_isNIL(il1)) return lsFloat_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsFloat_cpy(lsFloat_nil(&tmp),il1);
  lsFloat_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsFloat_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsFloat_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsFloat_t * lsFloat_filter(lsFloat_t * il, lsFloat_filter_t *func)
{
  int i,j;
  if (!il) return (il);
  
  j = 0;
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i)))
      LS_GET(il,j++) = LS_GET(il,i);
  }
  LS_N(il) = j;

  return (il);
}

lsFloat_t * lsFloat_CpyFilter(lsFloat_t *il_from, lsFloat_filter_t *func)
{
  return (lsFloat_cpyFilter(NULL,il_from,func));
}

lsFloat_t * lsFloat_cpyFilter(lsFloat_t *il_to, lsFloat_t *il_from, lsFloat_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsFloat_Nil();
  else
    lsFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsFloat_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

float lsFloat_foldl(lsFloat_t *il, float item0, lsFloat_fold_t *func)
{
  int i;
  float result = item0;
  if (!func)
    rs_error("lsFloat_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

float lsFloat_foldr(lsFloat_t *il, float item0, lsFloat_fold_t *func)
{
  int i;
  float result = item0;
  if (!func)
    rs_error("lsFloat_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

float lsFloat_foldl_2(lsFloat_t *il, float item0, lsFloat_fold_2_t *func,
		     void *arg)
{
  int i;
  float result = item0;
  if (!func)
    rs_error("lsFloat_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

float lsFloat_foldr_2(lsFloat_t *il, float item0, lsFloat_fold_2_t *func,
		     void *arg)
{
  int i;
  float result = item0;
  if (!func)
    rs_error("lsFloat_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsFloat_t * lsFloat_sscan_chr(lsFloat_t *il, char t, char *s)
{
  char *p;
  float v;

  if (!il)
    il = lsFloat_Nil();
  else
    lsFloat_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsFloat_add(il,v);
    }
  }
  return (il);
}

char * lsFloat_sprint_chr(char *s, lsFloat_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    float x = LS_GET(il,i);
#if defined(LS_IS_lsInt)
    sprintf(tmp,"%d",x);
#elif defined(LS_IS_lsULInt)
    sprintf(tmp,"%ld",x);
#elif defined(LS_IS_lsFloat)
    sprintf(tmp,"%g",x);
#elif defined(LS_IS_lsDouble)
    sprintf(tmp,"%lg",x);
#elif defined(LS_IS_lsChar)
    sprintf(tmp,"%c",x);
#endif
    n_tmp = (i > 0) ? strlen(tmp)+1 : strlen(tmp);
    if (n+n_tmp > max) {
      if (s)
	s = realloc(s,((max=n+n_tmp)+1)*sizeof(char));
      else {
	s = malloc(((max=n+n_tmp)+1)*sizeof(char));
	*s = '\0';
      }
    }
    if (i > 0) { 
      s[n] = t; s[n+1] = '\0';
    }
    strcat(s,tmp);
    n += n_tmp;
  }
  return (s);
}

char * lsFloat_sprintf_chr(char *s, lsFloat_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    float x = LS_GET(il,i);
    sprintf(tmp,format,x);

    n_tmp = (i > 0) ? strlen(tmp)+1 : strlen(tmp);
    if (n+n_tmp > max) {
      if (s)
	s = realloc(s,((max=n+n_tmp)+1)*sizeof(char));
      else {
	s = malloc(((max=n+n_tmp)+1)*sizeof(char));
	*s = '\0';
      }
    }
    if (i > 0) { 
      s[n] = t; s[n+1] = '\0';
    }
    strcat(s,tmp);
    n += n_tmp;
  }
  return (s);
}

#define LS_FILE_BUFSIZE (256)
 
int lsFloat_fwrite(FILE *fp, lsFloat_t *il)
{
    float *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(float), m, fp)) != m) {
	    rs_error("lsFloat_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(float), l, fp)) != l) {
	    rs_error("lsFloat_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsFloat_fread(lsFloat_t *il, int k, FILE *fp)
{
    float *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsFloat_realloc(lsFloat_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(float), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(float), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsFloat_lsFloat_minmax(lsFloat_t *il, int mode, lsFloat_cmp_t *func)
{
  int i,iminmax;
  float minmax;
  if (LS_isNIL(il)) 
    rs_error("lsFloat_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    float tmp = LS_GET(il,i);
    int cmp;
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0: /* min */
      cmp = (tmp < minmax) ? -1 : 1;
      break;
    case 1: /* max */
      cmp = (minmax < tmp) ? -1 : 1;
      break;
#endif
    case 2: /* minFunc */
      cmp = (*func)(tmp,minmax);
      break;
    case 3: /* maxFunc */
      cmp = (*func)(minmax,tmp);
      break;
    default:
      rs_error("_lsFloat_lsFloat_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

float lsFloat_maxFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  int i = _lsFloat_lsFloat_minmax(il,3,func);
  return (LS_GET(il,i));
}

float lsFloat_minFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  int i = _lsFloat_lsFloat_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsFloat_maxIndexFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  return _lsFloat_lsFloat_minmax(il,3,func);
}

int lsFloat_minIndexFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  return _lsFloat_lsFloat_minmax(il,2,func);
}

lsFloat_t * lsFloat_sortByIndex(lsFloat_t *il, lsInt_t *index, float undef)
{
  lsFloat_t *tmp = NULL;
  tmp = lsFloat_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsFloat_cpy(il,tmp);
      lsFloat_Free(tmp);
  }
  return (il);
}

lsFloat_t * lsFloat_cpySortByIndex(lsFloat_t *il_to, lsFloat_t *il_from, 
			   lsInt_t *index, float undef)
{
  int i;
  lsFloat_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsFloat_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    float item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsFloat_setIndex(il_to,i,item,undef);
    }
#else
    lsFloat_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsFloat_qsortX(float *v, int left, int right, 
		int mode, lsFloat_cmp_t *func)
{
  float x = v[left]; /* bestimme Trennelement */
  int l = left;
  int r = right;
  /* Trenne Zahlenfolge v[left..right] in zwei Zahlenfolgen
   * v[left..r] <= x und v[l..right] >= x */
  do {
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0:
      while(v[l] < x) l++;
      while(x < v[r]) r--;
      break;
    case 1:
      while(v[l] > x) l++;
      while(x > v[r]) r--;
      break;
#endif
    case 2:
      while((*func)(v[l],x) < 0) l++;
      while((*func)(x,v[r]) < 0) r--;
      break;
    case 3:
      while((*func)(v[l],x) > 0) l++;
      while((*func)(x,v[r]) > 0) r--;
      break;
    default:
      rs_error("_ls_qsort_X: undefined mode %d.",mode);
    }
    if (l <= r) {
      float h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsFloat_qsortX(v,left,r,mode,func);
  if (l < right) _lsFloat_qsortX(v,l,right,mode,func);
}

lsFloat_t * lsFloat_qsortLtFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsFloat_t * lsFloat_qsortGtFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsFloat_qsortX_2(float *v, int left, int right, 
		  int mode, lsFloat_cmp_2_t *func, void *arg)
{
  float x = v[left]; /* bestimme Trennelement */
  int l = left;
  int r = right;
  /* Trenne Zahlenfolge v[left..right] in zwei Zahlenfolgen
   * v[left..r] <= x und v[l..right] >= x */
  do {
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0:
      while(v[l] < x) l++;
      while(x < v[r]) r--;
      break;
    case 1:
      while(v[l] > x) l++;
      while(x > v[r]) r--;
      break;
#endif
    case 2:
      while((*func)(v[l],x,arg) < 0) l++;
      while((*func)(x,v[r],arg) < 0) r--;
      break;
    case 3:
      while((*func)(v[l],x,arg) > 0) l++;
      while((*func)(x,v[r],arg) > 0) r--;
      break;
    default:
      rs_error("_ls_qsort_X: undefined mode %d.",mode);
    }
    if (l <= r) {
      float h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsFloat_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsFloat_qsortX_2(v,l,right,mode,func,arg);
}

lsFloat_t * lsFloat_qsortLtFunc_2(lsFloat_t *il, lsFloat_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsFloat_t * lsFloat_qsortGtFunc_2(lsFloat_t *il, lsFloat_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsFloat_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsFloat_t *il = tplFst(arg);
  lsFloat_cmp_t *func = (lsFloat_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      float x = LS_GET(il,i);
      float y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsFloat_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsFloat_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      float x = LS_GET(il,i);
      float y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsFloat_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsFloat_t *il = tplFst(arg);
  lsFloat_cmp_t *func = (lsFloat_cmp_t *) tplSnd(arg);
  float *value = (float *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      float x = *value;
      float y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsFloat_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsFloat_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  float *value = (float *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      float x = *value;
      float y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsFloat_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsFloat_qsortIndexLtFunc(lsInt_t *index, lsFloat_t *il, 
			      lsFloat_cmp_t *func)
{
  int n; 
  pairPt_t arg;

  if (il && (n=LS_N(il)) > 0) {
    int i;
    /* index initialisieren */
    index = lsInt_realloc(index,n);
    LS_FORALL_ITEMS(il,i) LS_GET(index,i) = i;
    LS_N(index) = n;
    /* index sortieren */
    _lsInt_qsortX_2(LS_ITEMS(index),0,LS_N(index)-1,2,
		    (lsInt_cmp_2_t *) lsFloat_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsFloat_qsortIndexLt(lsInt_t *index, lsFloat_t *il)
{
  return (lsFloat_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsFloat_qsortIndexGtFunc(lsInt_t *index, lsFloat_t *il, 
			      lsFloat_cmp_t *func)
{
  int n; 
  pairPt_t arg;

  if (il && (n=LS_N(il)) > 0) {
    int i;
    /* index initialisieren */
    index = lsInt_realloc(index,n);
    LS_FORALL_ITEMS(il,i) LS_GET(index,i) = i;
    LS_N(index) = n;
    /* index sortieren */
    _lsInt_qsortX_2(LS_ITEMS(index),0,LS_N(index)-1,3,
		    (lsInt_cmp_2_t *) lsFloat_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsFloat_qsortIndexGt(lsInt_t *index, lsFloat_t *il)
{
  return (lsFloat_qsortIndexGtFunc(index,il,NULL));
}

void _lsFloat_mergeX(float *v, float *w, int ll, int rl, int rr,
		int mode, lsFloat_cmp_t *func)
{
  int lr = rl;
  int i = ll;
  while (ll < lr && rl < rr) {
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0:
      if (w[ll] <= w[rl])
	v[i++] = w[ll++];
      else
	v[i++] = w[rl++];
      break;
    case 1:
      if (w[ll] >= w[rl])
	v[i++] = w[ll++];
      else
	v[i++] = w[rl++];
      break;
#endif
    case 2:
      if (func(w[ll],w[rl]) <= 0)
	v[i++] = w[ll++];
      else
	v[i++] = w[rl++];
      break;
    case 3:
      if (func(w[ll],w[rl]) >= 0)
	v[i++] = w[ll++];
      else
	v[i++] = w[rl++];
      break;
    default:
      rs_error("_lsFloat_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsFloat_msortX(float *v, float *w, int left, int right, 
		int mode, lsFloat_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsFloat_msortX(v,w,left,m,mode,func);
  _lsFloat_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsFloat_mergeX(w,v,left,m,right,mode,func);
}

lsFloat_t * lsFloat_msortLtFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  static lsFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsFloat_realloc(_il,LS_N(il));
    _lsFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsFloat_t * lsFloat_msortGtFunc(lsFloat_t *il, lsFloat_cmp_t *func)
{
  static lsFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsFloat_realloc(_il,LS_N(il));
    _lsFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

float lsFloat_sum(lsFloat_t *il)
{
  float sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

float lsFloat_prod(lsFloat_t *il)
{
  float sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsFloat_t * lsFloat_scale(lsFloat_t *il, float s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsFloat_t * lsFloat_delta(lsFloat_t *il_to, lsFloat_t *il_from, float base)
{
  int i;
  lsFloat_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsFloat_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

float lsFloat_max(lsFloat_t *il)
{
  int i = _lsFloat_lsFloat_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

float lsFloat_min(lsFloat_t *il)
{
  int i = _lsFloat_lsFloat_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsFloat_maxIndex(lsFloat_t *il)
{
  return _lsFloat_lsFloat_minmax(il,1,NULL);
}

int lsFloat_minIndex(lsFloat_t *il)
{
  return _lsFloat_lsFloat_minmax(il,0,NULL);
}

lsFloat_t *lsFloat_rmdup(lsFloat_t *il)
{
  int i,j;
  float item;
  if (LS_isNIL(il)) return (il);

  item = LS_FIRST(il);
  for (i=1,j=1; i < LS_N(il); i++)
    if (item != LS_GET(il,i)) {
      item = LS_GET(il,i);
      LS_GET(il,j++) = item;
    }
  LS_N(il) = j;
  return (il);
}
      
    
lsFloat_t * lsFloat_qsortLt(lsFloat_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsFloat_t * lsFloat_qsortGt(lsFloat_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsFloat_t * lsFloat_msortLt(lsFloat_t *il)
{
  static lsFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsFloat_realloc(_il,LS_N(il));
    _lsFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsFloat_t * lsFloat_msortGt(lsFloat_t *il)
{
  static lsFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsFloat_realloc(_il,LS_N(il));
    _lsFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsFloat_bsearchX(lsFloat_t *il, float i,
		 int mode, lsFloat_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    float x = LS_GET(il,m);
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0: /* aufsteigend sortiert */
      if (i == x) return (m);
      if (i <  x) r = m-1; else l = m+1;
      break;
    case 1: /* absteigend sortiert */
      if (i == x) return (m);
      if (i >  x) r = m-1; else l = m+1;
      break;
#endif
    case 2: { /* aufsteigend sortiert, Vergleichsfunktion */
      int cmp = (*func)(i,x);
      if (cmp == 0) return (m);
      if (cmp <  0) r = m-1; else l = m+1;
      break; }
    case 3: { /* absteigend sortiert, Vergleichsfunktion */
      int cmp = (*func)(i,x);
      if (cmp == 0) return (m);
      if (cmp >  0) r = m-1; else l = m+1;
      break; }
    default:
      rs_error("_lsFloat_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsFloat_bsearchLtFunc(lsFloat_t *il, float i, lsFloat_cmp_t *func)
{
  return (_lsFloat_bsearchX(il,i,2,func));
}

int lsFloat_bsearchGtFunc(lsFloat_t *il, float i, lsFloat_cmp_t *func)
{
  return (_lsFloat_bsearchX(il,i,3,func));
}

int _lsFloat_bsearchX_2(lsFloat_t *il, float i,
		   int mode, lsFloat_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    float x = LS_GET(il,m);
    switch(mode) {
#if defined(LS_IS_lvalue)
    case 0: /* aufsteigend sortiert */
      if (i == x) return (m);
      if (i <  x) r = m-1; else l = m+1;
      break;
    case 1: /* absteigend sortiert */
      if (i == x) return (m);
      if (i >  x) r = m-1; else l = m+1;
      break;
#endif
    case 2: { /* aufsteigend sortiert, Vergleichsfunktion */
      int cmp = (*func)(i,x,arg);
      if (cmp == 0) return (m);
      if (cmp <  0) r = m-1; else l = m+1;
      break; }
    case 3: { /* absteigend sortiert, Vergleichsfunktion */
      int cmp = (*func)(i,x,arg);
      if (cmp == 0) return (m);
      if (cmp >  0) r = m-1; else l = m+1;
      break; }
    default:
      rs_error("_lsFloat_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsFloat_bsearchLtFunc_2(lsFloat_t *il,float i,lsFloat_cmp_2_t *func,void *arg)
{
  return (_lsFloat_bsearchX_2(il,i,2,func,arg));
}

int lsFloat_bsearchGtFunc_2(lsFloat_t *il,float i,lsFloat_cmp_2_t *func,void *arg)
{
  return (_lsFloat_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsFloat_bsearchLt(lsFloat_t *il, float i)
{
  return (_lsFloat_bsearchX(il,i,0,NULL));
}

int lsFloat_bsearchGt(lsFloat_t *il, float i)
{
  return (_lsFloat_bsearchX(il,i,1,NULL));
}

#endif 

int lsFloat_cmpFunc(lsFloat_t *il1, lsFloat_t *il2, lsFloat_cmp_t *func)
{
  int i;
  if (LS_isNIL(il1) && LS_isNIL(il2))
    return (0);
  if (LS_isNIL(il1))
    return (-1);
  LS_FORALL_ITEMS(il1,i) {
    if (!LS_EXISTS(il2,i))
      return (1);
    if (!func) {
#if defined(LS_IS_lvalue)
      float x = LS_GET(il1,i);
      float y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsFloat_cmpFunc: no cmp-function defined");
#endif
    } else {
      int cmp = (*func)(LS_GET(il1,i),LS_GET(il2,i));
      if (cmp == 0)
	continue;
      else
	return (cmp);
    }
  }
  return (LS_EXISTS(il2,i)) ? -1 : 0;
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsFloat_cmp(lsFloat_t *il1, lsFloat_t *il2)
{
  return (lsFloat_cmpFunc(il1,il2,NULL));
}

#endif
  

