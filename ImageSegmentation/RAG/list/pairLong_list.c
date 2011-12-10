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
#include "lsPairLong.h"
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

lsPairLong_t * lsPairLong_Nil(void)
{
  return (lsPairLong_nil(NULL));
}

lsPairLong_t * lsPairLong_realloc(lsPairLong_t *il, int n)
{
  if (!il) il = lsPairLong_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(pairLong_t), 
			  "list items");
  }
  return (il);
}

lsPairLong_t *  lsPairLong_nil(lsPairLong_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsPairLong_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsPairLong_t * lsPairLong_ConsNil(pairLong_t i)
{
  lsPairLong_t * il = rs_malloc(sizeof(lsPairLong_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairLong_t), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

pairLong_t lsPairLong_setIndex(lsPairLong_t *il, int index, pairLong_t i, pairLong_t i0)
{
  pairLong_t ret;
  int j;

  if (!il)
    rs_error("lsPairLong_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsPairLong_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(pairLong_t), 
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


lsPairLong_t * lsPairLong_setIndices(lsPairLong_t *il, lsInt_t *indices, pairLong_t x,
		       pairLong_t undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsPairLong_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsPairLong_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsPairLong_t * lsPairLong_setNil(lsPairLong_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsPairLong_t * lsPairLong_nsetIndex(lsPairLong_t *il, int index, int n, pairLong_t x, 
		      pairLong_t undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsPairLong_Nil();

  lsPairLong_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsPairLong_t * lsPairLong_setConsNil(lsPairLong_t * il, pairLong_t i)
{
  if (!il)
    return lsPairLong_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairLong_t), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsPairLong_getNewItemIndex(lsPairLong_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsPairLong_getNewItem(il);
  return (index);
}

pairLong_t *lsPairLong_getNewItem(lsPairLong_t *il)
{
  pairLong_t *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairLong_t), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsPairLong_t * lsPairLong_add(lsPairLong_t * il, pairLong_t i)
{
  if (!il)
    il = lsPairLong_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairLong_t), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsPairLong_t * lsPairLong_Add(lsPairLong_t * il, pairLong_t i)
{
  lsPairLong_t *il_to = lsPairLong_Nil();
  if (!il)
    return (lsPairLong_setConsNil(il_to,i));

  lsPairLong_realloc(il_to, il->n_list+LIST_BUF);
  lsPairLong_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsPairLong_t * lsPairLong_Cons(lsPairLong_t * il, pairLong_t i)
{
  lsPairLong_t *il_to = lsPairLong_Nil();
  if (!il)
    return (lsPairLong_setConsNil(il_to,i));

  lsPairLong_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(pairLong_t)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsPairLong_t * lsPairLong_cons(lsPairLong_t *il, pairLong_t i)
{
  if (!il)
    il = lsPairLong_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairLong_t), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(pairLong_t));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

pairLong_t lsPairLong_last(lsPairLong_t *il, pairLong_t undef)
{
  return (LS_LAST_CHECK(il,undef));
}

pairLong_t lsPairLong_head(lsPairLong_t *il, pairLong_t undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

pairLong_t lsPairLong_popLast(lsPairLong_t *il, pairLong_t undef)
{
  pairLong_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsPairLong_init(il);
  return (x);
}

pairLong_t lsPairLong_popHead(lsPairLong_t *il, pairLong_t undef)
{
  pairLong_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsPairLong_tail(il);
  return (x);
}

lsPairLong_t * lsPairLong_init(lsPairLong_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairLong_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsPairLong_t * lsPairLong_Init(lsPairLong_t * il)
{
  lsPairLong_t *il_to = lsPairLong_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsPairLong_Init: got empty list");
  }
  lsPairLong_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsPairLong_t * lsPairLong_tail(lsPairLong_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairLong_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(pairLong_t));
  return (il);
}
 
lsPairLong_t * lsPairLong_Tail(lsPairLong_t *il)
{
  lsPairLong_t *il_to = lsPairLong_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsPairLong_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsPairLong_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(pairLong_t));
  return (il_to);
}


lsPairLong_t * lsPairLong_take(lsPairLong_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsPairLong_t * lsPairLong_Take(lsPairLong_t *il, int n)
{
  lsPairLong_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsPairLong_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(pairLong_t));
  LS_N(il_to) = m;
  return (il_to);
}

pairLong_t lsPairLong_delSwap(lsPairLong_t *il, int i)
{
  pairLong_t x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

pairLong_t lsPairLong_delete(lsPairLong_t *il, int index, pairLong_t undef)
{
  pairLong_t x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(pairLong_t));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsPairLong_Free(lsPairLong_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsPairLong_free(lsPairLong_t * il)
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

pairLong_t lsPairLong_get(lsPairLong_t * il,int i)
{
  if (!il)
    rs_error("lsPairLong_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsPairLong_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

pairLong_t lsPairLong_getCheck(lsPairLong_t * il,int i,pairLong_t undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsPairLong_length(lsPairLong_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsPairLong_t *  lsPairLong_getRowPt(lsPairLong_t * row, lsPairLong_t * il, int i, int cols)
{
  row = lsPairLong_take(lsPairLong_dropPt(lsPairLong_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsPairLong_t *  lsPairLong_getRow(lsPairLong_t *row, lsPairLong_t *il, int i, int cols)
{
  lsPairLong_t rowPt;
  lsPairLong_getRowPt(lsPairLong_init(&rowPt), il, i, cols);

  lsPairLong_cpy(row, &rowPt);

  return (row);
}

lsPairLong_t *  lsPairLong_getCol(lsPairLong_t *col, lsPairLong_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  pairLong_t *item;

  if (n <= j)
    return (lsPairLong_setNil(col));

  col = lsPairLong_realloc(col, n / cols + 1);
  lsPairLong_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsPairLong_add(col,*item);

  return (col);
}

lsPairLong_t *  lsPairLong_setRow(lsPairLong_t *il, lsPairLong_t *row, int i, int cols)
{
  lsPairLong_t rowPt;
  int n;
  lsPairLong_getRowPt(lsPairLong_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsPairLong_cpy(&rowPt,lsPairLong_take(row,n));
  return (il);
}

lsPairLong_t *  lsPairLong_setCol(lsPairLong_t *il, lsPairLong_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  pairLong_t *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsPairLong_t *  lsPairLong_SetPt(int n, pairLong_t * items)
{
  return (lsPairLong_setPt(NULL,n,items));
}

lsPairLong_t *  lsPairLong_setPt(lsPairLong_t * il_to, int n, pairLong_t * items)
{
  if (!il_to)
    il_to = lsPairLong_Nil();
  lsPairLong_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsPairLong_t *  lsPairLong_CpyPt(lsPairLong_t * il_from)
{
  return (lsPairLong_cpyPt(NULL, il_from));
}

lsPairLong_t *  lsPairLong_cpyPt(lsPairLong_t * il_to, lsPairLong_t * il_from)
{
  if (!il_to)
    il_to = lsPairLong_Nil();
  lsPairLong_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsPairLong_t * lsPairLong_Cpy(const lsPairLong_t * il_from)
{
  return (lsPairLong_cpy(NULL,il_from));
}

lsPairLong_t * lsPairLong_cpy(lsPairLong_t * il_to, const lsPairLong_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (!il_from) return (il_to);

  lsPairLong_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(pairLong_t));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairLong_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsPairLong_t * lsPairLong_Cat(lsPairLong_t * il_1, lsPairLong_t * il_2)
{
  return (lsPairLong_cat(lsPairLong_Cpy(il_1), il_2));
}

lsPairLong_t * lsPairLong_cat(lsPairLong_t * il_to, lsPairLong_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsPairLong_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsPairLong_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(pairLong_t));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairLong_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsPairLong_t * lsPairLong_addCat(lsPairLong_t *il_to, lsPairLong_t *il)
{
  return lsPairLong_add(il_to, lsPairLong_Cat(il_to,il));
}

lsPairLong_t * lsPairLong_AddCat(lsPairLong_t *il_to, lsPairLong_t *il)
{
  return lsPairLong_Add(il_to, lsPairLong_Cat(il_to,il));
}
#endif

lsPairLong_t * lsPairLong_drop(lsPairLong_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsPairLong_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(pairLong_t));
  return (il);
}

lsPairLong_t * lsPairLong_Drop(lsPairLong_t *il, int i)
{
  lsPairLong_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsPairLong_setPt(lsPairLong_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsPairLong_Cpy(&tmp);
  return (il_split);
}

lsPairLong_t * lsPairLong_dropPt(lsPairLong_t *il, int i)
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

lsPairLong_t * lsPairLong_split(lsPairLong_t *il, int i)
{
  lsPairLong_t *il_drop = lsPairLong_Drop(il,i);
  lsPairLong_take(il,i);
  return (il_drop);
}

lsPt_t * lsPairLong_nsplit(lsPt_t *il_split, lsPairLong_t *il, lsInt_t *is)
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
      lsPairLong_t *split = lsPairLong_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsPairLong_t *lsPairLong_range(lsPairLong_t *il, int i0, int iN)
{
  lsPairLong_take(il,iN);
  if (i0 > 0) {
    lsPairLong_drop(il,i0-1);
  }
  return (il);
}

lsPairLong_t *lsPairLong_Range(lsPairLong_t *il, int i0, int iN)
{
  int n;
  lsPairLong_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsPairLong_Drop(il,i0);
  return (lsPairLong_take(il_to,iN));
}

lsPairLong_t *lsPairLong_rangePt(lsPairLong_t *il, int i0, int iN)
{
  return (lsPairLong_dropPt(lsPairLong_take(il,iN),i0));
}

lsPairLong_t *lsPairLong_cpyRange(lsPairLong_t *il_to, lsPairLong_t *il, int i0, int iN)
{
  lsPairLong_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairLong_rangePt(lsPairLong_cpyPt(lsPairLong_nil(&tmp),il),i0,iN);
  return (lsPairLong_cpy(il_to,&tmp));
}

lsPairLong_t *lsPairLong_catRange(lsPairLong_t *il_to, lsPairLong_t *il, int i0, int iN)
{
  lsPairLong_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairLong_rangePt(lsPairLong_cpyPt(lsPairLong_nil(&tmp),il),i0,iN);
  return (lsPairLong_cat(il_to,&tmp));
}
  
lsPairLong_t *lsPairLong_reverse(lsPairLong_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    pairLong_t tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsPairLong_makeIndex(lsInt_t *index, lsPairLong_t *il)
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
lsPairLong_t * lsPairLong_join(lsPairLong_t *il_to, lsPairLong_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairLong_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    pairLong_t x = LS_GET(il_from,i);
    if (lsPairLong_elem(il_to,x)) continue;

    lsPairLong_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsPairLong_t * lsPairLong_filterByValue(lsPairLong_t *il, pairLong_t undef, lsInt_t *indices)
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

lsPairLong_t * lsPairLong_CpyFilterByValue(lsPairLong_t *il, pairLong_t undef, lsInt_t *indices)
{
  return (lsPairLong_cpyFilterByValue(NULL,il,undef,indices));
}

lsPairLong_t * lsPairLong_cpyFilterByValue(lsPairLong_t *il_to, lsPairLong_t *il_from, pairLong_t undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsPairLong_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsPairLong_t * lsPairLong_filterByIndex(lsPairLong_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsPairLong_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsPairLong_t * lsPairLong_CpyFilterByIndex(lsPairLong_t *il, lsInt_t *indices)
{
  return (lsPairLong_cpyFilterByIndex(NULL,il,indices));
}

lsPairLong_t * lsPairLong_cpyFilterByIndex(lsPairLong_t *il_to, lsPairLong_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsPairLong_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsPairLong_t * lsPairLong_joinInts(lsPairLong_t *il_to, lsPairLong_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsPairLong_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairLong_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsPairLong_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsPairLong_elem(lsPairLong_t * il, pairLong_t item)
{
  return (lsPairLong_getLastIndex(il,item) >= 0);
}

int lsPairLong_getLastIndex(lsPairLong_t *il, pairLong_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsPairLong_getFstIndex(lsPairLong_t *il, pairLong_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsPairLong_getIndex(lsPairLong_t * il, pairLong_t item)
{
  return (lsPairLong_getLastIndex(il,item));
}

int lsPairLong_neqElem(lsPairLong_t * il, pairLong_t item)
{
  return (lsPairLong_getLastNeqIndex(il,item) >= 0);
}

int lsPairLong_getLastNeqIndex(lsPairLong_t * il, pairLong_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsPairLong_getFstNeqIndex(lsPairLong_t * il, pairLong_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsPairLong_disjoint(lsPairLong_t *il1, lsPairLong_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsPairLong_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsPairLong_t *lsPairLong_subst(lsPairLong_t *il, pairLong_t i, pairLong_t j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsPairLong_subBag(lsPairLong_t *il_sub, lsPairLong_t *il_super, pairLong_t undef)
{
  return lsPairLong_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsPairLong_subBagIndices(lsInt_t *indices,
		     lsPairLong_t *il_sub, lsPairLong_t *il_super, pairLong_t undef)
{
  lsPairLong_t _sub;
  lsPairLong_t _super;
  int i;
  pairLong_t last;
  pairLong_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairLong_qsortLt(lsPairLong_cpy(lsPairLong_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairLong_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairLong_take(&_super,last_index);
      if ((last_index = lsPairLong_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairLong_cpyPt(lsPairLong_nil(&_super),il_super);

      if ((last_index = lsPairLong_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairLong_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsPairLong_subBagLimitedIndices(lsInt_t *indices,
			    lsPairLong_t *il_sub, lsPairLong_t *il_super, 
			    lsInt_t *limit, pairLong_t undef)
{
  lsPairLong_t _sub;
  lsPairLong_t _super;
  int i;
  pairLong_t last;
  pairLong_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairLong_qsortLt(lsPairLong_cpy(lsPairLong_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairLong_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairLong_take(&_super,last_index);
      if ((last_index = lsPairLong_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairLong_cpyPt(lsPairLong_nil(&_super),il_super);
      if (limit) lsPairLong_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsPairLong_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairLong_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsPairLong_elemFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func)
{
 if (!func) rs_error("lsPairLong_elemFunc: cmp-function undefined.");
 return (lsPairLong_getIndexFunc(il,item,func) >= 0);
}

int lsPairLong_getIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairLong_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsPairLong_neqElemFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func)
{
  if (!func) rs_error("lsPairLong_neqElemFunc: cmp-function undefined.");
  return (lsPairLong_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsPairLong_getLastNeqIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairLong_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsPairLong_getFstNeqIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairLong_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsPairLong_t * lsPairLong_insert(lsPairLong_t *il, int index, pairLong_t item, pairLong_t item0)
{
  int i;
  if (!il)
    il = lsPairLong_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairLong_insert: illegal index %d.",index);

    lsPairLong_setIndex(il,index,item,item0);
    return (il);
  }
  lsPairLong_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(pairLong_t));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsPairLong_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsPairLong_t * lsPairLong_insertN(lsPairLong_t *il, int index, int n, pairLong_t item, 
		    pairLong_t item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsPairLong_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairLong_insert: illegal index %d.",index);

    lsPairLong_setIndex(il,index,item,item0);
    lsPairLong_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsPairLong_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(pairLong_t));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsPairLong_t * lsPairLong_insSwap(lsPairLong_t *il, int index, pairLong_t item, pairLong_t item0)
{
  int i,n;
  pairLong_t _item;

  if (!il)
    il = lsPairLong_Nil();

  if (index < 0)
    rs_error("lsPairLong_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsPairLong_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsPairLong_add(il,_item);
#else
  if (index < n)
    lsPairLong_add(il,_item);
#endif

  return (il);
}

pairLong_t lsPairLong_getFlip(int i, lsPairLong_t *il, pairLong_t undef)
{
  pairLong_t item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsPairLong_t * lsPairLong_map(lsPairLong_t * il, lsPairLong_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairLong_t * lsPairLong_map_2(lsPairLong_t * il, lsPairLong_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairLong_t * lsPairLong_map_3(lsPairLong_t * il, lsPairLong_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairLong_t * lsPairLong_mapSet(lsPairLong_t * il, lsPairLong_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairLong_t * lsPairLong_mapSet_2(lsPairLong_t * il, lsPairLong_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairLong_t * lsPairLong_mapSet_3(lsPairLong_t * il, lsPairLong_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairLong_t * lsPairLong_CpyMap(lsPairLong_t * il_from, lsPairLong_map_t *func)
{
  return (lsPairLong_cpyMap(NULL,il_from,func));
}

lsPairLong_t * lsPairLong_cpyMap(lsPairLong_t * il_to, lsPairLong_t * il_from, lsPairLong_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairLong_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsPairLong_t *  lsPairLong_CpyMap_2(lsPairLong_t * il_from, lsPairLong_map_2_t *func, void *arg)
{
  return (lsPairLong_cpyMap_2(NULL, il_from, func, arg));
}

lsPairLong_t *  lsPairLong_CpyMap_3(lsPairLong_t * il_from, lsPairLong_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsPairLong_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsPairLong_t *  lsPairLong_cpyMap_2(lsPairLong_t * il_to, lsPairLong_t * il_from, 
		      lsPairLong_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairLong_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsPairLong_t *  lsPairLong_cpyMap_3(lsPairLong_t * il_to, lsPairLong_t * il_from, 
		      lsPairLong_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairLong_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsPairLong_t * lsPairLong_CartProd(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func)
{
  return (lsPairLong_cpyCartProd(NULL,il1,il2,func));
}

lsPairLong_t * lsPairLong_cpyCartProd(lsPairLong_t *il_to, 
			lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsPairLong_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsPairLong_cpy(il_to,il1);
  if (!il_to)
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairLong_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairLong_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsPairLong_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsPairLong_t * lsPairLong_cartProd(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func)
{
  int i,j;
  lsPairLong_t tmp;
  if (LS_isNIL(il1)) return lsPairLong_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairLong_cpy(lsPairLong_nil(&tmp),il1);
  lsPairLong_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairLong_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairLong_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairLong_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsPairLong_t * lsPairLong_cartProd_2(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsPairLong_t tmp;
  if (LS_isNIL(il1)) return lsPairLong_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairLong_cpy(lsPairLong_nil(&tmp),il1);
  lsPairLong_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairLong_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairLong_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairLong_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsPairLong_t * lsPairLong_filter(lsPairLong_t * il, lsPairLong_filter_t *func)
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

lsPairLong_t * lsPairLong_CpyFilter(lsPairLong_t *il_from, lsPairLong_filter_t *func)
{
  return (lsPairLong_cpyFilter(NULL,il_from,func));
}

lsPairLong_t * lsPairLong_cpyFilter(lsPairLong_t *il_to, lsPairLong_t *il_from, lsPairLong_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsPairLong_Nil();
  else
    lsPairLong_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsPairLong_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

pairLong_t lsPairLong_foldl(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_t *func)
{
  int i;
  pairLong_t result = item0;
  if (!func)
    rs_error("lsPairLong_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

pairLong_t lsPairLong_foldr(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_t *func)
{
  int i;
  pairLong_t result = item0;
  if (!func)
    rs_error("lsPairLong_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

pairLong_t lsPairLong_foldl_2(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_2_t *func,
		     void *arg)
{
  int i;
  pairLong_t result = item0;
  if (!func)
    rs_error("lsPairLong_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

pairLong_t lsPairLong_foldr_2(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_2_t *func,
		     void *arg)
{
  int i;
  pairLong_t result = item0;
  if (!func)
    rs_error("lsPairLong_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsPairLong_t * lsPairLong_sscan_chr(lsPairLong_t *il, char t, char *s)
{
  char *p;
  pairLong_t v;

  if (!il)
    il = lsPairLong_Nil();
  else
    lsPairLong_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsPairLong_add(il,v);
    }
  }
  return (il);
}

char * lsPairLong_sprint_chr(char *s, lsPairLong_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairLong_t x = LS_GET(il,i);
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

char * lsPairLong_sprintf_chr(char *s, lsPairLong_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairLong_t x = LS_GET(il,i);
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
 
int lsPairLong_fwrite(FILE *fp, lsPairLong_t *il)
{
    pairLong_t *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(pairLong_t), m, fp)) != m) {
	    rs_error("lsPairLong_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(pairLong_t), l, fp)) != l) {
	    rs_error("lsPairLong_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsPairLong_fread(lsPairLong_t *il, int k, FILE *fp)
{
    pairLong_t *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsPairLong_realloc(lsPairLong_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(pairLong_t), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(pairLong_t), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsPairLong_lsPairLong_minmax(lsPairLong_t *il, int mode, lsPairLong_cmp_t *func)
{
  int i,iminmax;
  pairLong_t minmax;
  if (LS_isNIL(il)) 
    rs_error("lsPairLong_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    pairLong_t tmp = LS_GET(il,i);
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
      rs_error("_lsPairLong_lsPairLong_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

pairLong_t lsPairLong_maxFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  int i = _lsPairLong_lsPairLong_minmax(il,3,func);
  return (LS_GET(il,i));
}

pairLong_t lsPairLong_minFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  int i = _lsPairLong_lsPairLong_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsPairLong_maxIndexFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  return _lsPairLong_lsPairLong_minmax(il,3,func);
}

int lsPairLong_minIndexFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  return _lsPairLong_lsPairLong_minmax(il,2,func);
}

lsPairLong_t * lsPairLong_sortByIndex(lsPairLong_t *il, lsInt_t *index, pairLong_t undef)
{
  lsPairLong_t *tmp = NULL;
  tmp = lsPairLong_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsPairLong_cpy(il,tmp);
      lsPairLong_Free(tmp);
  }
  return (il);
}

lsPairLong_t * lsPairLong_cpySortByIndex(lsPairLong_t *il_to, lsPairLong_t *il_from, 
			   lsInt_t *index, pairLong_t undef)
{
  int i;
  lsPairLong_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsPairLong_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    pairLong_t item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsPairLong_setIndex(il_to,i,item,undef);
    }
#else
    lsPairLong_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsPairLong_qsortX(pairLong_t *v, int left, int right, 
		int mode, lsPairLong_cmp_t *func)
{
  pairLong_t x = v[left]; /* bestimme Trennelement */
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
      pairLong_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairLong_qsortX(v,left,r,mode,func);
  if (l < right) _lsPairLong_qsortX(v,l,right,mode,func);
}

lsPairLong_t * lsPairLong_qsortLtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsPairLong_t * lsPairLong_qsortGtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsPairLong_qsortX_2(pairLong_t *v, int left, int right, 
		  int mode, lsPairLong_cmp_2_t *func, void *arg)
{
  pairLong_t x = v[left]; /* bestimme Trennelement */
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
      pairLong_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairLong_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsPairLong_qsortX_2(v,l,right,mode,func,arg);
}

lsPairLong_t * lsPairLong_qsortLtFunc_2(lsPairLong_t *il, lsPairLong_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsPairLong_t * lsPairLong_qsortGtFunc_2(lsPairLong_t *il, lsPairLong_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsPairLong_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsPairLong_t *il = tplFst(arg);
  lsPairLong_cmp_t *func = (lsPairLong_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairLong_t x = LS_GET(il,i);
      pairLong_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairLong_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairLong_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsPairLong_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairLong_t x = LS_GET(il,i);
      pairLong_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairLong_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairLong_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsPairLong_t *il = tplFst(arg);
  lsPairLong_cmp_t *func = (lsPairLong_cmp_t *) tplSnd(arg);
  pairLong_t *value = (pairLong_t *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairLong_t x = *value;
      pairLong_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairLong_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairLong_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsPairLong_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  pairLong_t *value = (pairLong_t *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairLong_t x = *value;
      pairLong_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairLong_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsPairLong_qsortIndexLtFunc(lsInt_t *index, lsPairLong_t *il, 
			      lsPairLong_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairLong_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairLong_qsortIndexLt(lsInt_t *index, lsPairLong_t *il)
{
  return (lsPairLong_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsPairLong_qsortIndexGtFunc(lsInt_t *index, lsPairLong_t *il, 
			      lsPairLong_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairLong_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairLong_qsortIndexGt(lsInt_t *index, lsPairLong_t *il)
{
  return (lsPairLong_qsortIndexGtFunc(index,il,NULL));
}

void _lsPairLong_mergeX(pairLong_t *v, pairLong_t *w, int ll, int rl, int rr,
		int mode, lsPairLong_cmp_t *func)
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
      rs_error("_lsPairLong_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsPairLong_msortX(pairLong_t *v, pairLong_t *w, int left, int right, 
		int mode, lsPairLong_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsPairLong_msortX(v,w,left,m,mode,func);
  _lsPairLong_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsPairLong_mergeX(w,v,left,m,right,mode,func);
}

lsPairLong_t * lsPairLong_msortLtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  static lsPairLong_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairLong_realloc(_il,LS_N(il));
    _lsPairLong_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsPairLong_t * lsPairLong_msortGtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func)
{
  static lsPairLong_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairLong_realloc(_il,LS_N(il));
    _lsPairLong_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

pairLong_t lsPairLong_sum(lsPairLong_t *il)
{
  pairLong_t sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

pairLong_t lsPairLong_prod(lsPairLong_t *il)
{
  pairLong_t sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsPairLong_t * lsPairLong_scale(lsPairLong_t *il, pairLong_t s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsPairLong_t * lsPairLong_delta(lsPairLong_t *il_to, lsPairLong_t *il_from, pairLong_t base)
{
  int i;
  lsPairLong_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsPairLong_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

pairLong_t lsPairLong_max(lsPairLong_t *il)
{
  int i = _lsPairLong_lsPairLong_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

pairLong_t lsPairLong_min(lsPairLong_t *il)
{
  int i = _lsPairLong_lsPairLong_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsPairLong_maxIndex(lsPairLong_t *il)
{
  return _lsPairLong_lsPairLong_minmax(il,1,NULL);
}

int lsPairLong_minIndex(lsPairLong_t *il)
{
  return _lsPairLong_lsPairLong_minmax(il,0,NULL);
}

lsPairLong_t *lsPairLong_rmdup(lsPairLong_t *il)
{
  int i,j;
  pairLong_t item;
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
      
    
lsPairLong_t * lsPairLong_qsortLt(lsPairLong_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsPairLong_t * lsPairLong_qsortGt(lsPairLong_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairLong_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsPairLong_t * lsPairLong_msortLt(lsPairLong_t *il)
{
  static lsPairLong_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairLong_realloc(_il,LS_N(il));
    _lsPairLong_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsPairLong_t * lsPairLong_msortGt(lsPairLong_t *il)
{
  static lsPairLong_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairLong_realloc(_il,LS_N(il));
    _lsPairLong_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsPairLong_bsearchX(lsPairLong_t *il, pairLong_t i,
		 int mode, lsPairLong_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairLong_t x = LS_GET(il,m);
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
      rs_error("_lsPairLong_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairLong_bsearchLtFunc(lsPairLong_t *il, pairLong_t i, lsPairLong_cmp_t *func)
{
  return (_lsPairLong_bsearchX(il,i,2,func));
}

int lsPairLong_bsearchGtFunc(lsPairLong_t *il, pairLong_t i, lsPairLong_cmp_t *func)
{
  return (_lsPairLong_bsearchX(il,i,3,func));
}

int _lsPairLong_bsearchX_2(lsPairLong_t *il, pairLong_t i,
		   int mode, lsPairLong_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairLong_t x = LS_GET(il,m);
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
      rs_error("_lsPairLong_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairLong_bsearchLtFunc_2(lsPairLong_t *il,pairLong_t i,lsPairLong_cmp_2_t *func,void *arg)
{
  return (_lsPairLong_bsearchX_2(il,i,2,func,arg));
}

int lsPairLong_bsearchGtFunc_2(lsPairLong_t *il,pairLong_t i,lsPairLong_cmp_2_t *func,void *arg)
{
  return (_lsPairLong_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsPairLong_bsearchLt(lsPairLong_t *il, pairLong_t i)
{
  return (_lsPairLong_bsearchX(il,i,0,NULL));
}

int lsPairLong_bsearchGt(lsPairLong_t *il, pairLong_t i)
{
  return (_lsPairLong_bsearchX(il,i,1,NULL));
}

#endif 

int lsPairLong_cmpFunc(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_cmp_t *func)
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
      pairLong_t x = LS_GET(il1,i);
      pairLong_t y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsPairLong_cmpFunc: no cmp-function defined");
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

int lsPairLong_cmp(lsPairLong_t *il1, lsPairLong_t *il2)
{
  return (lsPairLong_cmpFunc(il1,il2,NULL));
}

#endif
  

