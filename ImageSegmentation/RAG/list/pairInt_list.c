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
#include "lsPairInt.h"
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

lsPairInt_t * lsPairInt_Nil(void)
{
  return (lsPairInt_nil(NULL));
}

lsPairInt_t * lsPairInt_realloc(lsPairInt_t *il, int n)
{
  if (!il) il = lsPairInt_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(pairInt_t), 
			  "list items");
  }
  return (il);
}

lsPairInt_t *  lsPairInt_nil(lsPairInt_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsPairInt_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsPairInt_t * lsPairInt_ConsNil(pairInt_t i)
{
  lsPairInt_t * il = rs_malloc(sizeof(lsPairInt_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairInt_t), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

pairInt_t lsPairInt_setIndex(lsPairInt_t *il, int index, pairInt_t i, pairInt_t i0)
{
  pairInt_t ret;
  int j;

  if (!il)
    rs_error("lsPairInt_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsPairInt_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(pairInt_t), 
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


lsPairInt_t * lsPairInt_setIndices(lsPairInt_t *il, lsInt_t *indices, pairInt_t x,
		       pairInt_t undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsPairInt_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsPairInt_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsPairInt_t * lsPairInt_setNil(lsPairInt_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsPairInt_t * lsPairInt_nsetIndex(lsPairInt_t *il, int index, int n, pairInt_t x, 
		      pairInt_t undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsPairInt_Nil();

  lsPairInt_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsPairInt_t * lsPairInt_setConsNil(lsPairInt_t * il, pairInt_t i)
{
  if (!il)
    return lsPairInt_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(pairInt_t), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsPairInt_getNewItemIndex(lsPairInt_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsPairInt_getNewItem(il);
  return (index);
}

pairInt_t *lsPairInt_getNewItem(lsPairInt_t *il)
{
  pairInt_t *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairInt_t), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsPairInt_t * lsPairInt_add(lsPairInt_t * il, pairInt_t i)
{
  if (!il)
    il = lsPairInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairInt_t), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsPairInt_t * lsPairInt_Add(lsPairInt_t * il, pairInt_t i)
{
  lsPairInt_t *il_to = lsPairInt_Nil();
  if (!il)
    return (lsPairInt_setConsNil(il_to,i));

  lsPairInt_realloc(il_to, il->n_list+LIST_BUF);
  lsPairInt_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsPairInt_t * lsPairInt_Cons(lsPairInt_t * il, pairInt_t i)
{
  lsPairInt_t *il_to = lsPairInt_Nil();
  if (!il)
    return (lsPairInt_setConsNil(il_to,i));

  lsPairInt_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(pairInt_t)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsPairInt_t * lsPairInt_cons(lsPairInt_t *il, pairInt_t i)
{
  if (!il)
    il = lsPairInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(pairInt_t), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(pairInt_t));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

pairInt_t lsPairInt_last(lsPairInt_t *il, pairInt_t undef)
{
  return (LS_LAST_CHECK(il,undef));
}

pairInt_t lsPairInt_head(lsPairInt_t *il, pairInt_t undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

pairInt_t lsPairInt_popLast(lsPairInt_t *il, pairInt_t undef)
{
  pairInt_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsPairInt_init(il);
  return (x);
}

pairInt_t lsPairInt_popHead(lsPairInt_t *il, pairInt_t undef)
{
  pairInt_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsPairInt_tail(il);
  return (x);
}

lsPairInt_t * lsPairInt_init(lsPairInt_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairInt_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsPairInt_t * lsPairInt_Init(lsPairInt_t * il)
{
  lsPairInt_t *il_to = lsPairInt_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsPairInt_Init: got empty list");
  }
  lsPairInt_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsPairInt_t * lsPairInt_tail(lsPairInt_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPairInt_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(pairInt_t));
  return (il);
}
 
lsPairInt_t * lsPairInt_Tail(lsPairInt_t *il)
{
  lsPairInt_t *il_to = lsPairInt_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsPairInt_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsPairInt_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(pairInt_t));
  return (il_to);
}


lsPairInt_t * lsPairInt_take(lsPairInt_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsPairInt_t * lsPairInt_Take(lsPairInt_t *il, int n)
{
  lsPairInt_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsPairInt_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(pairInt_t));
  LS_N(il_to) = m;
  return (il_to);
}

pairInt_t lsPairInt_delSwap(lsPairInt_t *il, int i)
{
  pairInt_t x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

pairInt_t lsPairInt_delete(lsPairInt_t *il, int index, pairInt_t undef)
{
  pairInt_t x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(pairInt_t));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsPairInt_Free(lsPairInt_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsPairInt_free(lsPairInt_t * il)
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

pairInt_t lsPairInt_get(lsPairInt_t * il,int i)
{
  if (!il)
    rs_error("lsPairInt_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsPairInt_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

pairInt_t lsPairInt_getCheck(lsPairInt_t * il,int i,pairInt_t undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsPairInt_length(lsPairInt_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsPairInt_t *  lsPairInt_getRowPt(lsPairInt_t * row, lsPairInt_t * il, int i, int cols)
{
  row = lsPairInt_take(lsPairInt_dropPt(lsPairInt_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsPairInt_t *  lsPairInt_getRow(lsPairInt_t *row, lsPairInt_t *il, int i, int cols)
{
  lsPairInt_t rowPt;
  lsPairInt_getRowPt(lsPairInt_init(&rowPt), il, i, cols);

  lsPairInt_cpy(row, &rowPt);

  return (row);
}

lsPairInt_t *  lsPairInt_getCol(lsPairInt_t *col, lsPairInt_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  pairInt_t *item;

  if (n <= j)
    return (lsPairInt_setNil(col));

  col = lsPairInt_realloc(col, n / cols + 1);
  lsPairInt_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsPairInt_add(col,*item);

  return (col);
}

lsPairInt_t *  lsPairInt_setRow(lsPairInt_t *il, lsPairInt_t *row, int i, int cols)
{
  lsPairInt_t rowPt;
  int n;
  lsPairInt_getRowPt(lsPairInt_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsPairInt_cpy(&rowPt,lsPairInt_take(row,n));
  return (il);
}

lsPairInt_t *  lsPairInt_setCol(lsPairInt_t *il, lsPairInt_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  pairInt_t *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsPairInt_t *  lsPairInt_SetPt(int n, pairInt_t * items)
{
  return (lsPairInt_setPt(NULL,n,items));
}

lsPairInt_t *  lsPairInt_setPt(lsPairInt_t * il_to, int n, pairInt_t * items)
{
  if (!il_to)
    il_to = lsPairInt_Nil();
  lsPairInt_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsPairInt_t *  lsPairInt_CpyPt(lsPairInt_t * il_from)
{
  return (lsPairInt_cpyPt(NULL, il_from));
}

lsPairInt_t *  lsPairInt_cpyPt(lsPairInt_t * il_to, lsPairInt_t * il_from)
{
  if (!il_to)
    il_to = lsPairInt_Nil();
  lsPairInt_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsPairInt_t * lsPairInt_Cpy(const lsPairInt_t * il_from)
{
  return (lsPairInt_cpy(NULL,il_from));
}

lsPairInt_t * lsPairInt_cpy(lsPairInt_t * il_to, const lsPairInt_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (!il_from) return (il_to);

  lsPairInt_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(pairInt_t));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsPairInt_t * lsPairInt_Cat(lsPairInt_t * il_1, lsPairInt_t * il_2)
{
  return (lsPairInt_cat(lsPairInt_Cpy(il_1), il_2));
}

lsPairInt_t * lsPairInt_cat(lsPairInt_t * il_to, lsPairInt_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsPairInt_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsPairInt_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(pairInt_t));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPairInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsPairInt_t * lsPairInt_addCat(lsPairInt_t *il_to, lsPairInt_t *il)
{
  return lsPairInt_add(il_to, lsPairInt_Cat(il_to,il));
}

lsPairInt_t * lsPairInt_AddCat(lsPairInt_t *il_to, lsPairInt_t *il)
{
  return lsPairInt_Add(il_to, lsPairInt_Cat(il_to,il));
}
#endif

lsPairInt_t * lsPairInt_drop(lsPairInt_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsPairInt_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(pairInt_t));
  return (il);
}

lsPairInt_t * lsPairInt_Drop(lsPairInt_t *il, int i)
{
  lsPairInt_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsPairInt_setPt(lsPairInt_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsPairInt_Cpy(&tmp);
  return (il_split);
}

lsPairInt_t * lsPairInt_dropPt(lsPairInt_t *il, int i)
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

lsPairInt_t * lsPairInt_split(lsPairInt_t *il, int i)
{
  lsPairInt_t *il_drop = lsPairInt_Drop(il,i);
  lsPairInt_take(il,i);
  return (il_drop);
}

lsPt_t * lsPairInt_nsplit(lsPt_t *il_split, lsPairInt_t *il, lsInt_t *is)
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
      lsPairInt_t *split = lsPairInt_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsPairInt_t *lsPairInt_range(lsPairInt_t *il, int i0, int iN)
{
  lsPairInt_take(il,iN);
  if (i0 > 0) {
    lsPairInt_drop(il,i0-1);
  }
  return (il);
}

lsPairInt_t *lsPairInt_Range(lsPairInt_t *il, int i0, int iN)
{
  int n;
  lsPairInt_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsPairInt_Drop(il,i0);
  return (lsPairInt_take(il_to,iN));
}

lsPairInt_t *lsPairInt_rangePt(lsPairInt_t *il, int i0, int iN)
{
  return (lsPairInt_dropPt(lsPairInt_take(il,iN),i0));
}

lsPairInt_t *lsPairInt_cpyRange(lsPairInt_t *il_to, lsPairInt_t *il, int i0, int iN)
{
  lsPairInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairInt_rangePt(lsPairInt_cpyPt(lsPairInt_nil(&tmp),il),i0,iN);
  return (lsPairInt_cpy(il_to,&tmp));
}

lsPairInt_t *lsPairInt_catRange(lsPairInt_t *il_to, lsPairInt_t *il, int i0, int iN)
{
  lsPairInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPairInt_rangePt(lsPairInt_cpyPt(lsPairInt_nil(&tmp),il),i0,iN);
  return (lsPairInt_cat(il_to,&tmp));
}
  
lsPairInt_t *lsPairInt_reverse(lsPairInt_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    pairInt_t tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsPairInt_makeIndex(lsInt_t *index, lsPairInt_t *il)
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
lsPairInt_t * lsPairInt_join(lsPairInt_t *il_to, lsPairInt_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    pairInt_t x = LS_GET(il_from,i);
    if (lsPairInt_elem(il_to,x)) continue;

    lsPairInt_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsPairInt_t * lsPairInt_filterByValue(lsPairInt_t *il, pairInt_t undef, lsInt_t *indices)
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

lsPairInt_t * lsPairInt_CpyFilterByValue(lsPairInt_t *il, pairInt_t undef, lsInt_t *indices)
{
  return (lsPairInt_cpyFilterByValue(NULL,il,undef,indices));
}

lsPairInt_t * lsPairInt_cpyFilterByValue(lsPairInt_t *il_to, lsPairInt_t *il_from, pairInt_t undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsPairInt_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsPairInt_t * lsPairInt_filterByIndex(lsPairInt_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsPairInt_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsPairInt_t * lsPairInt_CpyFilterByIndex(lsPairInt_t *il, lsInt_t *indices)
{
  return (lsPairInt_cpyFilterByIndex(NULL,il,indices));
}

lsPairInt_t * lsPairInt_cpyFilterByIndex(lsPairInt_t *il_to, lsPairInt_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsPairInt_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsPairInt_t * lsPairInt_joinInts(lsPairInt_t *il_to, lsPairInt_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsPairInt_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPairInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsPairInt_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsPairInt_elem(lsPairInt_t * il, pairInt_t item)
{
  return (lsPairInt_getLastIndex(il,item) >= 0);
}

int lsPairInt_getLastIndex(lsPairInt_t *il, pairInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsPairInt_getFstIndex(lsPairInt_t *il, pairInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsPairInt_getIndex(lsPairInt_t * il, pairInt_t item)
{
  return (lsPairInt_getLastIndex(il,item));
}

int lsPairInt_neqElem(lsPairInt_t * il, pairInt_t item)
{
  return (lsPairInt_getLastNeqIndex(il,item) >= 0);
}

int lsPairInt_getLastNeqIndex(lsPairInt_t * il, pairInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsPairInt_getFstNeqIndex(lsPairInt_t * il, pairInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsPairInt_disjoint(lsPairInt_t *il1, lsPairInt_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsPairInt_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsPairInt_t *lsPairInt_subst(lsPairInt_t *il, pairInt_t i, pairInt_t j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsPairInt_subBag(lsPairInt_t *il_sub, lsPairInt_t *il_super, pairInt_t undef)
{
  return lsPairInt_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsPairInt_subBagIndices(lsInt_t *indices,
		     lsPairInt_t *il_sub, lsPairInt_t *il_super, pairInt_t undef)
{
  lsPairInt_t _sub;
  lsPairInt_t _super;
  int i;
  pairInt_t last;
  pairInt_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairInt_qsortLt(lsPairInt_cpy(lsPairInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairInt_take(&_super,last_index);
      if ((last_index = lsPairInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairInt_cpyPt(lsPairInt_nil(&_super),il_super);

      if ((last_index = lsPairInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsPairInt_subBagLimitedIndices(lsInt_t *indices,
			    lsPairInt_t *il_sub, lsPairInt_t *il_super, 
			    lsInt_t *limit, pairInt_t undef)
{
  lsPairInt_t _sub;
  lsPairInt_t _super;
  int i;
  pairInt_t last;
  pairInt_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPairInt_qsortLt(lsPairInt_cpy(lsPairInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPairInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPairInt_take(&_super,last_index);
      if ((last_index = lsPairInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPairInt_cpyPt(lsPairInt_nil(&_super),il_super);
      if (limit) lsPairInt_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsPairInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPairInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsPairInt_elemFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func)
{
 if (!func) rs_error("lsPairInt_elemFunc: cmp-function undefined.");
 return (lsPairInt_getIndexFunc(il,item,func) >= 0);
}

int lsPairInt_getIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairInt_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsPairInt_neqElemFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func)
{
  if (!func) rs_error("lsPairInt_neqElemFunc: cmp-function undefined.");
  return (lsPairInt_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsPairInt_getLastNeqIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairInt_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsPairInt_getFstNeqIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPairInt_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsPairInt_t * lsPairInt_insert(lsPairInt_t *il, int index, pairInt_t item, pairInt_t item0)
{
  int i;
  if (!il)
    il = lsPairInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairInt_insert: illegal index %d.",index);

    lsPairInt_setIndex(il,index,item,item0);
    return (il);
  }
  lsPairInt_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(pairInt_t));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsPairInt_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsPairInt_t * lsPairInt_insertN(lsPairInt_t *il, int index, int n, pairInt_t item, 
		    pairInt_t item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsPairInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPairInt_insert: illegal index %d.",index);

    lsPairInt_setIndex(il,index,item,item0);
    lsPairInt_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsPairInt_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(pairInt_t));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsPairInt_t * lsPairInt_insSwap(lsPairInt_t *il, int index, pairInt_t item, pairInt_t item0)
{
  int i,n;
  pairInt_t _item;

  if (!il)
    il = lsPairInt_Nil();

  if (index < 0)
    rs_error("lsPairInt_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsPairInt_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsPairInt_add(il,_item);
#else
  if (index < n)
    lsPairInt_add(il,_item);
#endif

  return (il);
}

pairInt_t lsPairInt_getFlip(int i, lsPairInt_t *il, pairInt_t undef)
{
  pairInt_t item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsPairInt_t * lsPairInt_map(lsPairInt_t * il, lsPairInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairInt_t * lsPairInt_map_2(lsPairInt_t * il, lsPairInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairInt_t * lsPairInt_map_3(lsPairInt_t * il, lsPairInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairInt_t * lsPairInt_mapSet(lsPairInt_t * il, lsPairInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPairInt_t * lsPairInt_mapSet_2(lsPairInt_t * il, lsPairInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPairInt_t * lsPairInt_mapSet_3(lsPairInt_t * il, lsPairInt_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPairInt_t * lsPairInt_CpyMap(lsPairInt_t * il_from, lsPairInt_map_t *func)
{
  return (lsPairInt_cpyMap(NULL,il_from,func));
}

lsPairInt_t * lsPairInt_cpyMap(lsPairInt_t * il_to, lsPairInt_t * il_from, lsPairInt_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairInt_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsPairInt_t *  lsPairInt_CpyMap_2(lsPairInt_t * il_from, lsPairInt_map_2_t *func, void *arg)
{
  return (lsPairInt_cpyMap_2(NULL, il_from, func, arg));
}

lsPairInt_t *  lsPairInt_CpyMap_3(lsPairInt_t * il_from, lsPairInt_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsPairInt_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsPairInt_t *  lsPairInt_cpyMap_2(lsPairInt_t * il_to, lsPairInt_t * il_from, 
		      lsPairInt_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairInt_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsPairInt_t *  lsPairInt_cpyMap_3(lsPairInt_t * il_to, lsPairInt_t * il_from, 
		      lsPairInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPairInt_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsPairInt_t * lsPairInt_CartProd(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func)
{
  return (lsPairInt_cpyCartProd(NULL,il1,il2,func));
}

lsPairInt_t * lsPairInt_cpyCartProd(lsPairInt_t *il_to, 
			lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsPairInt_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsPairInt_cpy(il_to,il1);
  if (!il_to)
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairInt_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairInt_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsPairInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsPairInt_t * lsPairInt_cartProd(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func)
{
  int i,j;
  lsPairInt_t tmp;
  if (LS_isNIL(il1)) return lsPairInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairInt_cpy(lsPairInt_nil(&tmp),il1);
  lsPairInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsPairInt_t * lsPairInt_cartProd_2(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsPairInt_t tmp;
  if (LS_isNIL(il1)) return lsPairInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPairInt_cpy(lsPairInt_nil(&tmp),il1);
  lsPairInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPairInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPairInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPairInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsPairInt_t * lsPairInt_filter(lsPairInt_t * il, lsPairInt_filter_t *func)
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

lsPairInt_t * lsPairInt_CpyFilter(lsPairInt_t *il_from, lsPairInt_filter_t *func)
{
  return (lsPairInt_cpyFilter(NULL,il_from,func));
}

lsPairInt_t * lsPairInt_cpyFilter(lsPairInt_t *il_to, lsPairInt_t *il_from, lsPairInt_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsPairInt_Nil();
  else
    lsPairInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsPairInt_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

pairInt_t lsPairInt_foldl(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_t *func)
{
  int i;
  pairInt_t result = item0;
  if (!func)
    rs_error("lsPairInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

pairInt_t lsPairInt_foldr(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_t *func)
{
  int i;
  pairInt_t result = item0;
  if (!func)
    rs_error("lsPairInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

pairInt_t lsPairInt_foldl_2(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_2_t *func,
		     void *arg)
{
  int i;
  pairInt_t result = item0;
  if (!func)
    rs_error("lsPairInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

pairInt_t lsPairInt_foldr_2(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_2_t *func,
		     void *arg)
{
  int i;
  pairInt_t result = item0;
  if (!func)
    rs_error("lsPairInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsPairInt_t * lsPairInt_sscan_chr(lsPairInt_t *il, char t, char *s)
{
  char *p;
  pairInt_t v;

  if (!il)
    il = lsPairInt_Nil();
  else
    lsPairInt_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsPairInt_add(il,v);
    }
  }
  return (il);
}

char * lsPairInt_sprint_chr(char *s, lsPairInt_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairInt_t x = LS_GET(il,i);
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

char * lsPairInt_sprintf_chr(char *s, lsPairInt_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    pairInt_t x = LS_GET(il,i);
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
 
int lsPairInt_fwrite(FILE *fp, lsPairInt_t *il)
{
    pairInt_t *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(pairInt_t), m, fp)) != m) {
	    rs_error("lsPairInt_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(pairInt_t), l, fp)) != l) {
	    rs_error("lsPairInt_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsPairInt_fread(lsPairInt_t *il, int k, FILE *fp)
{
    pairInt_t *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsPairInt_realloc(lsPairInt_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(pairInt_t), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(pairInt_t), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsPairInt_lsPairInt_minmax(lsPairInt_t *il, int mode, lsPairInt_cmp_t *func)
{
  int i,iminmax;
  pairInt_t minmax;
  if (LS_isNIL(il)) 
    rs_error("lsPairInt_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    pairInt_t tmp = LS_GET(il,i);
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
      rs_error("_lsPairInt_lsPairInt_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

pairInt_t lsPairInt_maxFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  int i = _lsPairInt_lsPairInt_minmax(il,3,func);
  return (LS_GET(il,i));
}

pairInt_t lsPairInt_minFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  int i = _lsPairInt_lsPairInt_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsPairInt_maxIndexFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  return _lsPairInt_lsPairInt_minmax(il,3,func);
}

int lsPairInt_minIndexFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  return _lsPairInt_lsPairInt_minmax(il,2,func);
}

lsPairInt_t * lsPairInt_sortByIndex(lsPairInt_t *il, lsInt_t *index, pairInt_t undef)
{
  lsPairInt_t *tmp = NULL;
  tmp = lsPairInt_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsPairInt_cpy(il,tmp);
      lsPairInt_Free(tmp);
  }
  return (il);
}

lsPairInt_t * lsPairInt_cpySortByIndex(lsPairInt_t *il_to, lsPairInt_t *il_from, 
			   lsInt_t *index, pairInt_t undef)
{
  int i;
  lsPairInt_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsPairInt_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    pairInt_t item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsPairInt_setIndex(il_to,i,item,undef);
    }
#else
    lsPairInt_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsPairInt_qsortX(pairInt_t *v, int left, int right, 
		int mode, lsPairInt_cmp_t *func)
{
  pairInt_t x = v[left]; /* bestimme Trennelement */
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
      pairInt_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairInt_qsortX(v,left,r,mode,func);
  if (l < right) _lsPairInt_qsortX(v,l,right,mode,func);
}

lsPairInt_t * lsPairInt_qsortLtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsPairInt_t * lsPairInt_qsortGtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsPairInt_qsortX_2(pairInt_t *v, int left, int right, 
		  int mode, lsPairInt_cmp_2_t *func, void *arg)
{
  pairInt_t x = v[left]; /* bestimme Trennelement */
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
      pairInt_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPairInt_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsPairInt_qsortX_2(v,l,right,mode,func,arg);
}

lsPairInt_t * lsPairInt_qsortLtFunc_2(lsPairInt_t *il, lsPairInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsPairInt_t * lsPairInt_qsortGtFunc_2(lsPairInt_t *il, lsPairInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsPairInt_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsPairInt_t *il = tplFst(arg);
  lsPairInt_cmp_t *func = (lsPairInt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairInt_t x = LS_GET(il,i);
      pairInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairInt_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsPairInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairInt_t x = LS_GET(il,i);
      pairInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairInt_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsPairInt_t *il = tplFst(arg);
  lsPairInt_cmp_t *func = (lsPairInt_cmp_t *) tplSnd(arg);
  pairInt_t *value = (pairInt_t *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairInt_t x = *value;
      pairInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPairInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsPairInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  pairInt_t *value = (pairInt_t *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      pairInt_t x = *value;
      pairInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPairInt_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsPairInt_qsortIndexLtFunc(lsInt_t *index, lsPairInt_t *il, 
			      lsPairInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairInt_qsortIndexLt(lsInt_t *index, lsPairInt_t *il)
{
  return (lsPairInt_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsPairInt_qsortIndexGtFunc(lsInt_t *index, lsPairInt_t *il, 
			      lsPairInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPairInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPairInt_qsortIndexGt(lsInt_t *index, lsPairInt_t *il)
{
  return (lsPairInt_qsortIndexGtFunc(index,il,NULL));
}

void _lsPairInt_mergeX(pairInt_t *v, pairInt_t *w, int ll, int rl, int rr,
		int mode, lsPairInt_cmp_t *func)
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
      rs_error("_lsPairInt_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsPairInt_msortX(pairInt_t *v, pairInt_t *w, int left, int right, 
		int mode, lsPairInt_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsPairInt_msortX(v,w,left,m,mode,func);
  _lsPairInt_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsPairInt_mergeX(w,v,left,m,right,mode,func);
}

lsPairInt_t * lsPairInt_msortLtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  static lsPairInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairInt_realloc(_il,LS_N(il));
    _lsPairInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsPairInt_t * lsPairInt_msortGtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func)
{
  static lsPairInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairInt_realloc(_il,LS_N(il));
    _lsPairInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

pairInt_t lsPairInt_sum(lsPairInt_t *il)
{
  pairInt_t sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

pairInt_t lsPairInt_prod(lsPairInt_t *il)
{
  pairInt_t sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsPairInt_t * lsPairInt_scale(lsPairInt_t *il, pairInt_t s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsPairInt_t * lsPairInt_delta(lsPairInt_t *il_to, lsPairInt_t *il_from, pairInt_t base)
{
  int i;
  lsPairInt_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsPairInt_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

pairInt_t lsPairInt_max(lsPairInt_t *il)
{
  int i = _lsPairInt_lsPairInt_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

pairInt_t lsPairInt_min(lsPairInt_t *il)
{
  int i = _lsPairInt_lsPairInt_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsPairInt_maxIndex(lsPairInt_t *il)
{
  return _lsPairInt_lsPairInt_minmax(il,1,NULL);
}

int lsPairInt_minIndex(lsPairInt_t *il)
{
  return _lsPairInt_lsPairInt_minmax(il,0,NULL);
}

lsPairInt_t *lsPairInt_rmdup(lsPairInt_t *il)
{
  int i,j;
  pairInt_t item;
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
      
    
lsPairInt_t * lsPairInt_qsortLt(lsPairInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsPairInt_t * lsPairInt_qsortGt(lsPairInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPairInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsPairInt_t * lsPairInt_msortLt(lsPairInt_t *il)
{
  static lsPairInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairInt_realloc(_il,LS_N(il));
    _lsPairInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsPairInt_t * lsPairInt_msortGt(lsPairInt_t *il)
{
  static lsPairInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPairInt_realloc(_il,LS_N(il));
    _lsPairInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsPairInt_bsearchX(lsPairInt_t *il, pairInt_t i,
		 int mode, lsPairInt_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairInt_t x = LS_GET(il,m);
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
      rs_error("_lsPairInt_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairInt_bsearchLtFunc(lsPairInt_t *il, pairInt_t i, lsPairInt_cmp_t *func)
{
  return (_lsPairInt_bsearchX(il,i,2,func));
}

int lsPairInt_bsearchGtFunc(lsPairInt_t *il, pairInt_t i, lsPairInt_cmp_t *func)
{
  return (_lsPairInt_bsearchX(il,i,3,func));
}

int _lsPairInt_bsearchX_2(lsPairInt_t *il, pairInt_t i,
		   int mode, lsPairInt_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    pairInt_t x = LS_GET(il,m);
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
      rs_error("_lsPairInt_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPairInt_bsearchLtFunc_2(lsPairInt_t *il,pairInt_t i,lsPairInt_cmp_2_t *func,void *arg)
{
  return (_lsPairInt_bsearchX_2(il,i,2,func,arg));
}

int lsPairInt_bsearchGtFunc_2(lsPairInt_t *il,pairInt_t i,lsPairInt_cmp_2_t *func,void *arg)
{
  return (_lsPairInt_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsPairInt_bsearchLt(lsPairInt_t *il, pairInt_t i)
{
  return (_lsPairInt_bsearchX(il,i,0,NULL));
}

int lsPairInt_bsearchGt(lsPairInt_t *il, pairInt_t i)
{
  return (_lsPairInt_bsearchX(il,i,1,NULL));
}

#endif 

int lsPairInt_cmpFunc(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_cmp_t *func)
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
      pairInt_t x = LS_GET(il1,i);
      pairInt_t y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsPairInt_cmpFunc: no cmp-function defined");
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

int lsPairInt_cmp(lsPairInt_t *il1, lsPairInt_t *il2)
{
  return (lsPairInt_cmpFunc(il1,il2,NULL));
}

#endif
  

