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
#include "lsPairFloat.h"
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

lsPairFloat_t * lsPairFloat_Nil(void)
{
  return (lsPairFloat_nil(NULL));
}

lsPairFloat_t * lsPairFloat_realloc(lsPairFloat_t *il, int n)
{
  if (!il) il = lsPairFloat_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(pairFloat_t), 
			  "list items");
  }
  return (il);
}

lsPairFloat_t *  lsPairFloat_nil(lsPairFloat_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsPairFloat_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsPairFloat_t * lsPairFloat_ConsNil(pairFloat_t i)
{
  lsPairFloat_t * il = rs_malloc(sizeof(lsPairFloat_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairFloat_t), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

pairFloat_t lsPairFloat_setIndex(lsPairFloat_t *il, int index, pairFloat_t i, pairFloat_t i0)
{
  pairFloat_t ret;
  int j;

  if (!il)
    rs_error("lsPairFloat_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsPairFloat_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(pairFloat_t), 
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


lsPairFloat_t * lsPairFloat_setIndices(lsPairFloat_t *il, lsInt_t *indices, pairFloat_t x,
		       pairFloat_t undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsPairFloat_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsPairFloat_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsPairFloat_t * lsPairFloat_setNil(lsPairFloat_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsPairFloat_t * lsPairFloat_nsetIndex(lsPairFloat_t *il, int index, int n, pairFloat_t x, 
		      pairFloat_t undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsPairFloat_Nil();

  lsPairFloat_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsPairFloat_t * lsPairFloat_setConsNil(lsPairFloat_t * il, pairFloat_t i)
{
  if (!il)
    return lsPairFloat_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairFloat_t), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsPairFloat_getNewItemIndex(lsPairFloat_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsPairFloat_getNewItem(il);
  return (index);
}

pairFloat_t *lsPairFloat_getNewItem(lsPairFloat_t *il)
{
  pairFloat_t *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairFloat_t), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsPairFloat_t * lsPairFloat_add(lsPairFloat_t * il, pairFloat_t i)
{
  if (!il)
    il = lsPairFloat_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairFloat_t), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsPairFloat_t * lsPairFloat_Add(lsPairFloat_t * il, pairFloat_t i)
{
  lsPairFloat_t *il_to = lsPairFloat_Nil();
  if (!il)
    return (lsPairFloat_setConsNil(il_to,i));

  lsPairFloat_realloc(il_to, il->n_list+LIST_BUF);
  lsPairFloat_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsPairFloat_t * lsPairFloat_Cons(lsPairFloat_t * il, pairFloat_t i)
{
  lsPairFloat_t *il_to = lsPairFloat_Nil();
  if (!il)
    return (lsPairFloat_setConsNil(il_to,i));

  lsPairFloat_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(pairFloat_t)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsPairFloat_t * lsPairFloat_cons(lsPairFloat_t *il, pairFloat_t i)
{
  if (!il)
    il = lsPairFloat_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairFloat_t), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(pairFloat_t));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

pairFloat_t lsPairFloat_last(lsPairFloat_t *il, pairFloat_t undef)
{
  return (LS_LAST_CHECK(il,undef));
}

pairFloat_t lsPairFloat_head(lsPairFloat_t *il, pairFloat_t undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

pairFloat_t lsPairFloat_popLast(lsPairFloat_t *il, pairFloat_t undef)
{
  pairFloat_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsPairFloat_init(il);
  return (x);
}

pairFloat_t lsPairFloat_popHead(lsPairFloat_t *il, pairFloat_t undef)
{
  pairFloat_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsPairFloat_tail(il);
  return (x);
}

lsPairFloat_t * lsPairFloat_init(lsPairFloat_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairFloat_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsPairFloat_t * lsPairFloat_Init(lsPairFloat_t * il)
{
  lsPairFloat_t *il_to = lsPairFloat_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsPairFloat_Init: got empty list");
  }
  lsPairFloat_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsPairFloat_t * lsPairFloat_tail(lsPairFloat_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairFloat_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(pairFloat_t));
  return (il);
}
 
lsPairFloat_t * lsPairFloat_Tail(lsPairFloat_t *il)
{
  lsPairFloat_t *il_to = lsPairFloat_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsPairFloat_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsPairFloat_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(pairFloat_t));
  return (il_to);
}


lsPairFloat_t * lsPairFloat_take(lsPairFloat_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsPairFloat_t * lsPairFloat_Take(lsPairFloat_t *il, int n)
{
  lsPairFloat_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsPairFloat_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(pairFloat_t));
  LS_N(il_to) = m;
  return (il_to);
}

pairFloat_t lsPairFloat_delSwap(lsPairFloat_t *il, int i)
{
  pairFloat_t x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

pairFloat_t lsPairFloat_delete(lsPairFloat_t *il, int index, pairFloat_t undef)
{
  pairFloat_t x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(pairFloat_t));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsPairFloat_Free(lsPairFloat_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsPairFloat_free(lsPairFloat_t * il)
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

pairFloat_t lsPairFloat_get(lsPairFloat_t * il,int i)
{
  if (!il)
    rs_error("lsPairFloat_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsPairFloat_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

pairFloat_t lsPairFloat_getCheck(lsPairFloat_t * il,int i,pairFloat_t undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsPairFloat_length(lsPairFloat_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsPairFloat_t *  lsPairFloat_getRowPt(lsPairFloat_t * row, lsPairFloat_t * il, int i, int cols)
{
  row = lsPairFloat_take(lsPairFloat_dropPt(lsPairFloat_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsPairFloat_t *  lsPairFloat_getRow(lsPairFloat_t *row, lsPairFloat_t *il, int i, int cols)
{
  lsPairFloat_t rowPt;
  lsPairFloat_getRowPt(lsPairFloat_init(&rowPt), il, i, cols);

  lsPairFloat_cpy(row, &rowPt);

  return (row);
}

lsPairFloat_t *  lsPairFloat_getCol(lsPairFloat_t *col, lsPairFloat_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  pairFloat_t *item;

  if (n <= j)
    return (lsPairFloat_setNil(col));

  col = lsPairFloat_realloc(col, n / cols + 1);
  lsPairFloat_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsPairFloat_add(col,*item);

  return (col);
}

lsPairFloat_t *  lsPairFloat_setRow(lsPairFloat_t *il, lsPairFloat_t *row, int i, int cols)
{
  lsPairFloat_t rowPt;
  int n;
  lsPairFloat_getRowPt(lsPairFloat_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsPairFloat_cpy(&rowPt,lsPairFloat_take(row,n));
  return (il);
}

lsPairFloat_t *  lsPairFloat_setCol(lsPairFloat_t *il, lsPairFloat_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  pairFloat_t *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsPairFloat_t *  lsPairFloat_SetPt(int n, pairFloat_t * items)
{
  return (lsPairFloat_setPt(NULL,n,items));
}

lsPairFloat_t *  lsPairFloat_setPt(lsPairFloat_t * il_to, int n, pairFloat_t * items)
{
  if (!il_to)
    il_to = lsPairFloat_Nil();
  lsPairFloat_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsPairFloat_t *  lsPairFloat_CpyPt(lsPairFloat_t * il_from)
{
  return (lsPairFloat_cpyPt(NULL, il_from));
}

lsPairFloat_t *  lsPairFloat_cpyPt(lsPairFloat_t * il_to, lsPairFloat_t * il_from)
{
  if (!il_to)
    il_to = lsPairFloat_Nil();
  lsPairFloat_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsPairFloat_t * lsPairFloat_Cpy(const lsPairFloat_t * il_from)
{
  return (lsPairFloat_cpy(NULL,il_from));
}

lsPairFloat_t * lsPairFloat_cpy(lsPairFloat_t * il_to, const lsPairFloat_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (!il_from) return (il_to);

  lsPairFloat_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(pairFloat_t));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairFloat_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsPairFloat_t * lsPairFloat_Cat(lsPairFloat_t * il_1, lsPairFloat_t * il_2)
{
  return (lsPairFloat_cat(lsPairFloat_Cpy(il_1), il_2));
}

lsPairFloat_t * lsPairFloat_cat(lsPairFloat_t * il_to, lsPairFloat_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsPairFloat_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsPairFloat_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(pairFloat_t));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairFloat_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsPairFloat_t * lsPairFloat_addCat(lsPairFloat_t *il_to, lsPairFloat_t *il)
{
  return lsPairFloat_add(il_to, lsPairFloat_Cat(il_to,il));
}

lsPairFloat_t * lsPairFloat_AddCat(lsPairFloat_t *il_to, lsPairFloat_t *il)
{
  return lsPairFloat_Add(il_to, lsPairFloat_Cat(il_to,il));
}
#endif

lsPairFloat_t * lsPairFloat_drop(lsPairFloat_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsPairFloat_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(pairFloat_t));
  return (il);
}

lsPairFloat_t * lsPairFloat_Drop(lsPairFloat_t *il, int i)
{
  lsPairFloat_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsPairFloat_setPt(lsPairFloat_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsPairFloat_Cpy(&tmp);
  return (il_split);
}

lsPairFloat_t * lsPairFloat_dropPt(lsPairFloat_t *il, int i)
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

lsPairFloat_t * lsPairFloat_split(lsPairFloat_t *il, int i)
{
  lsPairFloat_t *il_drop = lsPairFloat_Drop(il,i);
  lsPairFloat_take(il,i);
  return (il_drop);
}

lsPt_t * lsPairFloat_nsplit(lsPt_t *il_split, lsPairFloat_t *il, lsInt_t *is)
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
      lsPairFloat_t *split = lsPairFloat_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsPairFloat_t *lsPairFloat_range(lsPairFloat_t *il, int i0, int iN)
{
  lsPairFloat_take(il,iN);
  if (i0 > 0) {
    lsPairFloat_drop(il,i0-1);
  }
  return (il);
}

lsPairFloat_t *lsPairFloat_Range(lsPairFloat_t *il, int i0, int iN)
{
  int n;
  lsPairFloat_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsPairFloat_Drop(il,i0);
  return (lsPairFloat_take(il_to,iN));
}

lsPairFloat_t *lsPairFloat_rangePt(lsPairFloat_t *il, int i0, int iN)
{
  return (lsPairFloat_dropPt(lsPairFloat_take(il,iN),i0));
}

lsPairFloat_t *lsPairFloat_cpyRange(lsPairFloat_t *il_to, lsPairFloat_t *il, int i0, int iN)
{
  lsPairFloat_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairFloat_rangePt(lsPairFloat_cpyPt(lsPairFloat_nil(&tmp),il),i0,iN);
  return (lsPairFloat_cpy(il_to,&tmp));
}

lsPairFloat_t *lsPairFloat_catRange(lsPairFloat_t *il_to, lsPairFloat_t *il, int i0, int iN)
{
  lsPairFloat_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairFloat_rangePt(lsPairFloat_cpyPt(lsPairFloat_nil(&tmp),il),i0,iN);
  return (lsPairFloat_cat(il_to,&tmp));
}
  
lsPairFloat_t *lsPairFloat_reverse(lsPairFloat_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    pairFloat_t tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsPairFloat_makeIndex(lsInt_t *index, lsPairFloat_t *il)
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
lsPairFloat_t * lsPairFloat_join(lsPairFloat_t *il_to, lsPairFloat_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairFloat_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    pairFloat_t x = LS_GET(il_from,i);
    if (lsPairFloat_elem(il_to,x)) continue;

    lsPairFloat_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsPairFloat_t * lsPairFloat_filterByValue(lsPairFloat_t *il, pairFloat_t undef, lsInt_t *indices)
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

lsPairFloat_t * lsPairFloat_CpyFilterByValue(lsPairFloat_t *il, pairFloat_t undef, lsInt_t *indices)
{
  return (lsPairFloat_cpyFilterByValue(NULL,il,undef,indices));
}

lsPairFloat_t * lsPairFloat_cpyFilterByValue(lsPairFloat_t *il_to, lsPairFloat_t *il_from, pairFloat_t undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsPairFloat_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsPairFloat_t * lsPairFloat_filterByIndex(lsPairFloat_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsPairFloat_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsPairFloat_t * lsPairFloat_CpyFilterByIndex(lsPairFloat_t *il, lsInt_t *indices)
{
  return (lsPairFloat_cpyFilterByIndex(NULL,il,indices));
}

lsPairFloat_t * lsPairFloat_cpyFilterByIndex(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsPairFloat_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsPairFloat_t * lsPairFloat_joinInts(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsPairFloat_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairFloat_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsPairFloat_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsPairFloat_elem(lsPairFloat_t * il, pairFloat_t item)
{
  return (lsPairFloat_getLastIndex(il,item) >= 0);
}

int lsPairFloat_getLastIndex(lsPairFloat_t *il, pairFloat_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsPairFloat_getFstIndex(lsPairFloat_t *il, pairFloat_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsPairFloat_getIndex(lsPairFloat_t * il, pairFloat_t item)
{
  return (lsPairFloat_getLastIndex(il,item));
}

int lsPairFloat_neqElem(lsPairFloat_t * il, pairFloat_t item)
{
  return (lsPairFloat_getLastNeqIndex(il,item) >= 0);
}

int lsPairFloat_getLastNeqIndex(lsPairFloat_t * il, pairFloat_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsPairFloat_getFstNeqIndex(lsPairFloat_t * il, pairFloat_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsPairFloat_disjoint(lsPairFloat_t *il1, lsPairFloat_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsPairFloat_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsPairFloat_t *lsPairFloat_subst(lsPairFloat_t *il, pairFloat_t i, pairFloat_t j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsPairFloat_subBag(lsPairFloat_t *il_sub, lsPairFloat_t *il_super, pairFloat_t undef)
{
  return lsPairFloat_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsPairFloat_subBagIndices(lsInt_t *indices,
		     lsPairFloat_t *il_sub, lsPairFloat_t *il_super, pairFloat_t undef)
{
  lsPairFloat_t _sub;
  lsPairFloat_t _super;
  int i;
  pairFloat_t last;
  pairFloat_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairFloat_qsortLt(lsPairFloat_cpy(lsPairFloat_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairFloat_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairFloat_take(&_super,last_index);
      if ((last_index = lsPairFloat_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairFloat_cpyPt(lsPairFloat_nil(&_super),il_super);

      if ((last_index = lsPairFloat_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairFloat_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsPairFloat_subBagLimitedIndices(lsInt_t *indices,
			    lsPairFloat_t *il_sub, lsPairFloat_t *il_super, 
			    lsInt_t *limit, pairFloat_t undef)
{
  lsPairFloat_t _sub;
  lsPairFloat_t _super;
  int i;
  pairFloat_t last;
  pairFloat_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairFloat_qsortLt(lsPairFloat_cpy(lsPairFloat_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairFloat_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairFloat_take(&_super,last_index);
      if ((last_index = lsPairFloat_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairFloat_cpyPt(lsPairFloat_nil(&_super),il_super);
      if (limit) lsPairFloat_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsPairFloat_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairFloat_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsPairFloat_elemFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func)
{
 if (!func) rs_error("lsPairFloat_elemFunc: cmp-function undefined.");
 return (lsPairFloat_getIndexFunc(il,item,func) >= 0);
}

int lsPairFloat_getIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairFloat_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsPairFloat_neqElemFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func)
{
  if (!func) rs_error("lsPairFloat_neqElemFunc: cmp-function undefined.");
  return (lsPairFloat_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsPairFloat_getLastNeqIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairFloat_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsPairFloat_getFstNeqIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairFloat_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsPairFloat_t * lsPairFloat_insert(lsPairFloat_t *il, int index, pairFloat_t item, pairFloat_t item0)
{
  int i;
  if (!il)
    il = lsPairFloat_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairFloat_insert: illegal index %d.",index);

    lsPairFloat_setIndex(il,index,item,item0);
    return (il);
  }
  lsPairFloat_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(pairFloat_t));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsPairFloat_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsPairFloat_t * lsPairFloat_insertN(lsPairFloat_t *il, int index, int n, pairFloat_t item, 
		    pairFloat_t item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsPairFloat_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairFloat_insert: illegal index %d.",index);

    lsPairFloat_setIndex(il,index,item,item0);
    lsPairFloat_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsPairFloat_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(pairFloat_t));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsPairFloat_t * lsPairFloat_insSwap(lsPairFloat_t *il, int index, pairFloat_t item, pairFloat_t item0)
{
  int i,n;
  pairFloat_t _item;

  if (!il)
    il = lsPairFloat_Nil();

  if (index < 0)
    rs_error("lsPairFloat_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsPairFloat_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsPairFloat_add(il,_item);
#else
  if (index < n)
    lsPairFloat_add(il,_item);
#endif

  return (il);
}

pairFloat_t lsPairFloat_getFlip(int i, lsPairFloat_t *il, pairFloat_t undef)
{
  pairFloat_t item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsPairFloat_t * lsPairFloat_map(lsPairFloat_t * il, lsPairFloat_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_map_2(lsPairFloat_t * il, lsPairFloat_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_map_3(lsPairFloat_t * il, lsPairFloat_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_mapSet(lsPairFloat_t * il, lsPairFloat_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_mapSet_2(lsPairFloat_t * il, lsPairFloat_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_mapSet_3(lsPairFloat_t * il, lsPairFloat_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_CpyMap(lsPairFloat_t * il_from, lsPairFloat_map_t *func)
{
  return (lsPairFloat_cpyMap(NULL,il_from,func));
}

lsPairFloat_t * lsPairFloat_cpyMap(lsPairFloat_t * il_to, lsPairFloat_t * il_from, lsPairFloat_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairFloat_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsPairFloat_t *  lsPairFloat_CpyMap_2(lsPairFloat_t * il_from, lsPairFloat_map_2_t *func, void *arg)
{
  return (lsPairFloat_cpyMap_2(NULL, il_from, func, arg));
}

lsPairFloat_t *  lsPairFloat_CpyMap_3(lsPairFloat_t * il_from, lsPairFloat_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsPairFloat_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsPairFloat_t *  lsPairFloat_cpyMap_2(lsPairFloat_t * il_to, lsPairFloat_t * il_from, 
		      lsPairFloat_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairFloat_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsPairFloat_t *  lsPairFloat_cpyMap_3(lsPairFloat_t * il_to, lsPairFloat_t * il_from, 
		      lsPairFloat_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairFloat_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsPairFloat_t * lsPairFloat_CartProd(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func)
{
  return (lsPairFloat_cpyCartProd(NULL,il1,il2,func));
}

lsPairFloat_t * lsPairFloat_cpyCartProd(lsPairFloat_t *il_to, 
			lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsPairFloat_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsPairFloat_cpy(il_to,il1);
  if (!il_to)
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairFloat_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairFloat_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsPairFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsPairFloat_t * lsPairFloat_cartProd(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func)
{
  int i,j;
  lsPairFloat_t tmp;
  if (LS_isNIL(il1)) return lsPairFloat_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairFloat_cpy(lsPairFloat_nil(&tmp),il1);
  lsPairFloat_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairFloat_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairFloat_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsPairFloat_t * lsPairFloat_cartProd_2(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsPairFloat_t tmp;
  if (LS_isNIL(il1)) return lsPairFloat_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairFloat_cpy(lsPairFloat_nil(&tmp),il1);
  lsPairFloat_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairFloat_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairFloat_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairFloat_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsPairFloat_t * lsPairFloat_filter(lsPairFloat_t * il, lsPairFloat_filter_t *func)
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

lsPairFloat_t * lsPairFloat_CpyFilter(lsPairFloat_t *il_from, lsPairFloat_filter_t *func)
{
  return (lsPairFloat_cpyFilter(NULL,il_from,func));
}

lsPairFloat_t * lsPairFloat_cpyFilter(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsPairFloat_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsPairFloat_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

pairFloat_t lsPairFloat_foldl(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_t *func)
{
  int i;
  pairFloat_t result = item0;
  if (!func)
    rs_error("lsPairFloat_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

pairFloat_t lsPairFloat_foldr(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_t *func)
{
  int i;
  pairFloat_t result = item0;
  if (!func)
    rs_error("lsPairFloat_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

pairFloat_t lsPairFloat_foldl_2(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_2_t *func,
		     void *arg)
{
  int i;
  pairFloat_t result = item0;
  if (!func)
    rs_error("lsPairFloat_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

pairFloat_t lsPairFloat_foldr_2(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_2_t *func,
		     void *arg)
{
  int i;
  pairFloat_t result = item0;
  if (!func)
    rs_error("lsPairFloat_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsPairFloat_t * lsPairFloat_sscan_chr(lsPairFloat_t *il, char t, char *s)
{
  char *p;
  pairFloat_t v;

  if (!il)
    il = lsPairFloat_Nil();
  else
    lsPairFloat_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsPairFloat_add(il,v);
    }
  }
  return (il);
}

char * lsPairFloat_sprint_chr(char *s, lsPairFloat_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairFloat_t x = LS_GET(il,i);
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

char * lsPairFloat_sprintf_chr(char *s, lsPairFloat_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairFloat_t x = LS_GET(il,i);
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
 
int lsPairFloat_fwrite(FILE *fp, lsPairFloat_t *il)
{
    pairFloat_t *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(pairFloat_t), m, fp)) != m) {
	    rs_error("lsPairFloat_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(pairFloat_t), l, fp)) != l) {
	    rs_error("lsPairFloat_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsPairFloat_fread(lsPairFloat_t *il, int k, FILE *fp)
{
    pairFloat_t *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsPairFloat_realloc(lsPairFloat_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(pairFloat_t), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(pairFloat_t), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsPairFloat_lsPairFloat_minmax(lsPairFloat_t *il, int mode, lsPairFloat_cmp_t *func)
{
  int i,iminmax;
  pairFloat_t minmax;
  if (LS_isNIL(il)) 
    rs_error("lsPairFloat_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    pairFloat_t tmp = LS_GET(il,i);
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
      rs_error("_lsPairFloat_lsPairFloat_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

pairFloat_t lsPairFloat_maxFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  int i = _lsPairFloat_lsPairFloat_minmax(il,3,func);
  return (LS_GET(il,i));
}

pairFloat_t lsPairFloat_minFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  int i = _lsPairFloat_lsPairFloat_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsPairFloat_maxIndexFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  return _lsPairFloat_lsPairFloat_minmax(il,3,func);
}

int lsPairFloat_minIndexFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  return _lsPairFloat_lsPairFloat_minmax(il,2,func);
}

lsPairFloat_t * lsPairFloat_sortByIndex(lsPairFloat_t *il, lsInt_t *index, pairFloat_t undef)
{
  lsPairFloat_t *tmp = NULL;
  tmp = lsPairFloat_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsPairFloat_cpy(il,tmp);
      lsPairFloat_Free(tmp);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_cpySortByIndex(lsPairFloat_t *il_to, lsPairFloat_t *il_from, 
			   lsInt_t *index, pairFloat_t undef)
{
  int i;
  lsPairFloat_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsPairFloat_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    pairFloat_t item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsPairFloat_setIndex(il_to,i,item,undef);
    }
#else
    lsPairFloat_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsPairFloat_qsortX(pairFloat_t *v, int left, int right, 
		int mode, lsPairFloat_cmp_t *func)
{
  pairFloat_t x = v[left]; /* bestimme Trennelement */
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
      pairFloat_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairFloat_qsortX(v,left,r,mode,func);
  if (l < right) _lsPairFloat_qsortX(v,l,right,mode,func);
}

lsPairFloat_t * lsPairFloat_qsortLtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_qsortGtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsPairFloat_qsortX_2(pairFloat_t *v, int left, int right, 
		  int mode, lsPairFloat_cmp_2_t *func, void *arg)
{
  pairFloat_t x = v[left]; /* bestimme Trennelement */
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
      pairFloat_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairFloat_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsPairFloat_qsortX_2(v,l,right,mode,func,arg);
}

lsPairFloat_t * lsPairFloat_qsortLtFunc_2(lsPairFloat_t *il, lsPairFloat_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_qsortGtFunc_2(lsPairFloat_t *il, lsPairFloat_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsPairFloat_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsPairFloat_t *il = tplFst(arg);
  lsPairFloat_cmp_t *func = (lsPairFloat_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairFloat_t x = LS_GET(il,i);
      pairFloat_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairFloat_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsPairFloat_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairFloat_t x = LS_GET(il,i);
      pairFloat_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairFloat_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsPairFloat_t *il = tplFst(arg);
  lsPairFloat_cmp_t *func = (lsPairFloat_cmp_t *) tplSnd(arg);
  pairFloat_t *value = (pairFloat_t *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairFloat_t x = *value;
      pairFloat_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairFloat_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairFloat_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsPairFloat_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  pairFloat_t *value = (pairFloat_t *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairFloat_t x = *value;
      pairFloat_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairFloat_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsPairFloat_qsortIndexLtFunc(lsInt_t *index, lsPairFloat_t *il, 
			      lsPairFloat_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairFloat_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairFloat_qsortIndexLt(lsInt_t *index, lsPairFloat_t *il)
{
  return (lsPairFloat_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsPairFloat_qsortIndexGtFunc(lsInt_t *index, lsPairFloat_t *il, 
			      lsPairFloat_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairFloat_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairFloat_qsortIndexGt(lsInt_t *index, lsPairFloat_t *il)
{
  return (lsPairFloat_qsortIndexGtFunc(index,il,NULL));
}

void _lsPairFloat_mergeX(pairFloat_t *v, pairFloat_t *w, int ll, int rl, int rr,
		int mode, lsPairFloat_cmp_t *func)
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
      rs_error("_lsPairFloat_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsPairFloat_msortX(pairFloat_t *v, pairFloat_t *w, int left, int right, 
		int mode, lsPairFloat_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsPairFloat_msortX(v,w,left,m,mode,func);
  _lsPairFloat_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsPairFloat_mergeX(w,v,left,m,right,mode,func);
}

lsPairFloat_t * lsPairFloat_msortLtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  static lsPairFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairFloat_realloc(_il,LS_N(il));
    _lsPairFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_msortGtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func)
{
  static lsPairFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairFloat_realloc(_il,LS_N(il));
    _lsPairFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

pairFloat_t lsPairFloat_sum(lsPairFloat_t *il)
{
  pairFloat_t sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

pairFloat_t lsPairFloat_prod(lsPairFloat_t *il)
{
  pairFloat_t sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsPairFloat_t * lsPairFloat_scale(lsPairFloat_t *il, pairFloat_t s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_delta(lsPairFloat_t *il_to, lsPairFloat_t *il_from, pairFloat_t base)
{
  int i;
  lsPairFloat_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsPairFloat_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

pairFloat_t lsPairFloat_max(lsPairFloat_t *il)
{
  int i = _lsPairFloat_lsPairFloat_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

pairFloat_t lsPairFloat_min(lsPairFloat_t *il)
{
  int i = _lsPairFloat_lsPairFloat_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsPairFloat_maxIndex(lsPairFloat_t *il)
{
  return _lsPairFloat_lsPairFloat_minmax(il,1,NULL);
}

int lsPairFloat_minIndex(lsPairFloat_t *il)
{
  return _lsPairFloat_lsPairFloat_minmax(il,0,NULL);
}

lsPairFloat_t *lsPairFloat_rmdup(lsPairFloat_t *il)
{
  int i,j;
  pairFloat_t item;
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
      
    
lsPairFloat_t * lsPairFloat_qsortLt(lsPairFloat_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_qsortGt(lsPairFloat_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairFloat_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_msortLt(lsPairFloat_t *il)
{
  static lsPairFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairFloat_realloc(_il,LS_N(il));
    _lsPairFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsPairFloat_t * lsPairFloat_msortGt(lsPairFloat_t *il)
{
  static lsPairFloat_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairFloat_realloc(_il,LS_N(il));
    _lsPairFloat_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsPairFloat_bsearchX(lsPairFloat_t *il, pairFloat_t i,
		 int mode, lsPairFloat_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairFloat_t x = LS_GET(il,m);
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
      rs_error("_lsPairFloat_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairFloat_bsearchLtFunc(lsPairFloat_t *il, pairFloat_t i, lsPairFloat_cmp_t *func)
{
  return (_lsPairFloat_bsearchX(il,i,2,func));
}

int lsPairFloat_bsearchGtFunc(lsPairFloat_t *il, pairFloat_t i, lsPairFloat_cmp_t *func)
{
  return (_lsPairFloat_bsearchX(il,i,3,func));
}

int _lsPairFloat_bsearchX_2(lsPairFloat_t *il, pairFloat_t i,
		   int mode, lsPairFloat_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairFloat_t x = LS_GET(il,m);
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
      rs_error("_lsPairFloat_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairFloat_bsearchLtFunc_2(lsPairFloat_t *il,pairFloat_t i,lsPairFloat_cmp_2_t *func,void *arg)
{
  return (_lsPairFloat_bsearchX_2(il,i,2,func,arg));
}

int lsPairFloat_bsearchGtFunc_2(lsPairFloat_t *il,pairFloat_t i,lsPairFloat_cmp_2_t *func,void *arg)
{
  return (_lsPairFloat_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsPairFloat_bsearchLt(lsPairFloat_t *il, pairFloat_t i)
{
  return (_lsPairFloat_bsearchX(il,i,0,NULL));
}

int lsPairFloat_bsearchGt(lsPairFloat_t *il, pairFloat_t i)
{
  return (_lsPairFloat_bsearchX(il,i,1,NULL));
}

#endif 

int lsPairFloat_cmpFunc(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_cmp_t *func)
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
      pairFloat_t x = LS_GET(il1,i);
      pairFloat_t y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsPairFloat_cmpFunc: no cmp-function defined");
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

int lsPairFloat_cmp(lsPairFloat_t *il1, lsPairFloat_t *il2)
{
  return (lsPairFloat_cmpFunc(il1,il2,NULL));
}

#endif
  

