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
#include "lsTripleInt.h"
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

lsTripleInt_t * lsTripleInt_Nil(void)
{
  return (lsTripleInt_nil(NULL));
}

lsTripleInt_t * lsTripleInt_realloc(lsTripleInt_t *il, int n)
{
  if (!il) il = lsTripleInt_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(tripleInt_t), 
			  "list items");
  }
  return (il);
}

lsTripleInt_t *  lsTripleInt_nil(lsTripleInt_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsTripleInt_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsTripleInt_t * lsTripleInt_ConsNil(tripleInt_t i)
{
  lsTripleInt_t * il = rs_malloc(sizeof(lsTripleInt_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(tripleInt_t), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

tripleInt_t lsTripleInt_setIndex(lsTripleInt_t *il, int index, tripleInt_t i, tripleInt_t i0)
{
  tripleInt_t ret;
  int j;

  if (!il)
    rs_error("lsTripleInt_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsTripleInt_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(tripleInt_t), 
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


lsTripleInt_t * lsTripleInt_setIndices(lsTripleInt_t *il, lsInt_t *indices, tripleInt_t x,
		       tripleInt_t undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsTripleInt_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsTripleInt_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsTripleInt_t * lsTripleInt_setNil(lsTripleInt_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsTripleInt_t * lsTripleInt_nsetIndex(lsTripleInt_t *il, int index, int n, tripleInt_t x, 
		      tripleInt_t undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsTripleInt_Nil();

  lsTripleInt_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsTripleInt_t * lsTripleInt_setConsNil(lsTripleInt_t * il, tripleInt_t i)
{
  if (!il)
    return lsTripleInt_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(tripleInt_t), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsTripleInt_getNewItemIndex(lsTripleInt_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsTripleInt_getNewItem(il);
  return (index);
}

tripleInt_t *lsTripleInt_getNewItem(lsTripleInt_t *il)
{
  tripleInt_t *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(tripleInt_t), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsTripleInt_t * lsTripleInt_add(lsTripleInt_t * il, tripleInt_t i)
{
  if (!il)
    il = lsTripleInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(tripleInt_t), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsTripleInt_t * lsTripleInt_Add(lsTripleInt_t * il, tripleInt_t i)
{
  lsTripleInt_t *il_to = lsTripleInt_Nil();
  if (!il)
    return (lsTripleInt_setConsNil(il_to,i));

  lsTripleInt_realloc(il_to, il->n_list+LIST_BUF);
  lsTripleInt_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsTripleInt_t * lsTripleInt_Cons(lsTripleInt_t * il, tripleInt_t i)
{
  lsTripleInt_t *il_to = lsTripleInt_Nil();
  if (!il)
    return (lsTripleInt_setConsNil(il_to,i));

  lsTripleInt_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(tripleInt_t)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsTripleInt_t * lsTripleInt_cons(lsTripleInt_t *il, tripleInt_t i)
{
  if (!il)
    il = lsTripleInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(tripleInt_t), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(tripleInt_t));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

tripleInt_t lsTripleInt_last(lsTripleInt_t *il, tripleInt_t undef)
{
  return (LS_LAST_CHECK(il,undef));
}

tripleInt_t lsTripleInt_head(lsTripleInt_t *il, tripleInt_t undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

tripleInt_t lsTripleInt_popLast(lsTripleInt_t *il, tripleInt_t undef)
{
  tripleInt_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsTripleInt_init(il);
  return (x);
}

tripleInt_t lsTripleInt_popHead(lsTripleInt_t *il, tripleInt_t undef)
{
  tripleInt_t x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsTripleInt_tail(il);
  return (x);
}

lsTripleInt_t * lsTripleInt_init(lsTripleInt_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsTripleInt_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsTripleInt_t * lsTripleInt_Init(lsTripleInt_t * il)
{
  lsTripleInt_t *il_to = lsTripleInt_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsTripleInt_Init: got empty list");
  }
  lsTripleInt_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsTripleInt_t * lsTripleInt_tail(lsTripleInt_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsTripleInt_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(tripleInt_t));
  return (il);
}
 
lsTripleInt_t * lsTripleInt_Tail(lsTripleInt_t *il)
{
  lsTripleInt_t *il_to = lsTripleInt_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsTripleInt_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsTripleInt_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(tripleInt_t));
  return (il_to);
}


lsTripleInt_t * lsTripleInt_take(lsTripleInt_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsTripleInt_t * lsTripleInt_Take(lsTripleInt_t *il, int n)
{
  lsTripleInt_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsTripleInt_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(tripleInt_t));
  LS_N(il_to) = m;
  return (il_to);
}

tripleInt_t lsTripleInt_delSwap(lsTripleInt_t *il, int i)
{
  tripleInt_t x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

tripleInt_t lsTripleInt_delete(lsTripleInt_t *il, int index, tripleInt_t undef)
{
  tripleInt_t x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(tripleInt_t));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsTripleInt_Free(lsTripleInt_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsTripleInt_free(lsTripleInt_t * il)
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

tripleInt_t lsTripleInt_get(lsTripleInt_t * il,int i)
{
  if (!il)
    rs_error("lsTripleInt_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsTripleInt_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

tripleInt_t lsTripleInt_getCheck(lsTripleInt_t * il,int i,tripleInt_t undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsTripleInt_length(lsTripleInt_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsTripleInt_t *  lsTripleInt_getRowPt(lsTripleInt_t * row, lsTripleInt_t * il, int i, int cols)
{
  row = lsTripleInt_take(lsTripleInt_dropPt(lsTripleInt_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsTripleInt_t *  lsTripleInt_getRow(lsTripleInt_t *row, lsTripleInt_t *il, int i, int cols)
{
  lsTripleInt_t rowPt;
  lsTripleInt_getRowPt(lsTripleInt_init(&rowPt), il, i, cols);

  lsTripleInt_cpy(row, &rowPt);

  return (row);
}

lsTripleInt_t *  lsTripleInt_getCol(lsTripleInt_t *col, lsTripleInt_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  tripleInt_t *item;

  if (n <= j)
    return (lsTripleInt_setNil(col));

  col = lsTripleInt_realloc(col, n / cols + 1);
  lsTripleInt_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsTripleInt_add(col,*item);

  return (col);
}

lsTripleInt_t *  lsTripleInt_setRow(lsTripleInt_t *il, lsTripleInt_t *row, int i, int cols)
{
  lsTripleInt_t rowPt;
  int n;
  lsTripleInt_getRowPt(lsTripleInt_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsTripleInt_cpy(&rowPt,lsTripleInt_take(row,n));
  return (il);
}

lsTripleInt_t *  lsTripleInt_setCol(lsTripleInt_t *il, lsTripleInt_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  tripleInt_t *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsTripleInt_t *  lsTripleInt_SetPt(int n, tripleInt_t * items)
{
  return (lsTripleInt_setPt(NULL,n,items));
}

lsTripleInt_t *  lsTripleInt_setPt(lsTripleInt_t * il_to, int n, tripleInt_t * items)
{
  if (!il_to)
    il_to = lsTripleInt_Nil();
  lsTripleInt_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsTripleInt_t *  lsTripleInt_CpyPt(lsTripleInt_t * il_from)
{
  return (lsTripleInt_cpyPt(NULL, il_from));
}

lsTripleInt_t *  lsTripleInt_cpyPt(lsTripleInt_t * il_to, lsTripleInt_t * il_from)
{
  if (!il_to)
    il_to = lsTripleInt_Nil();
  lsTripleInt_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsTripleInt_t * lsTripleInt_Cpy(const lsTripleInt_t * il_from)
{
  return (lsTripleInt_cpy(NULL,il_from));
}

lsTripleInt_t * lsTripleInt_cpy(lsTripleInt_t * il_to, const lsTripleInt_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (!il_from) return (il_to);

  lsTripleInt_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(tripleInt_t));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsTripleInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsTripleInt_t * lsTripleInt_Cat(lsTripleInt_t * il_1, lsTripleInt_t * il_2)
{
  return (lsTripleInt_cat(lsTripleInt_Cpy(il_1), il_2));
}

lsTripleInt_t * lsTripleInt_cat(lsTripleInt_t * il_to, lsTripleInt_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsTripleInt_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsTripleInt_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(tripleInt_t));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsTripleInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsTripleInt_t * lsTripleInt_addCat(lsTripleInt_t *il_to, lsTripleInt_t *il)
{
  return lsTripleInt_add(il_to, lsTripleInt_Cat(il_to,il));
}

lsTripleInt_t * lsTripleInt_AddCat(lsTripleInt_t *il_to, lsTripleInt_t *il)
{
  return lsTripleInt_Add(il_to, lsTripleInt_Cat(il_to,il));
}
#endif

lsTripleInt_t * lsTripleInt_drop(lsTripleInt_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsTripleInt_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(tripleInt_t));
  return (il);
}

lsTripleInt_t * lsTripleInt_Drop(lsTripleInt_t *il, int i)
{
  lsTripleInt_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsTripleInt_setPt(lsTripleInt_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsTripleInt_Cpy(&tmp);
  return (il_split);
}

lsTripleInt_t * lsTripleInt_dropPt(lsTripleInt_t *il, int i)
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

lsTripleInt_t * lsTripleInt_split(lsTripleInt_t *il, int i)
{
  lsTripleInt_t *il_drop = lsTripleInt_Drop(il,i);
  lsTripleInt_take(il,i);
  return (il_drop);
}

lsPt_t * lsTripleInt_nsplit(lsPt_t *il_split, lsTripleInt_t *il, lsInt_t *is)
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
      lsTripleInt_t *split = lsTripleInt_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsTripleInt_t *lsTripleInt_range(lsTripleInt_t *il, int i0, int iN)
{
  lsTripleInt_take(il,iN);
  if (i0 > 0) {
    lsTripleInt_drop(il,i0-1);
  }
  return (il);
}

lsTripleInt_t *lsTripleInt_Range(lsTripleInt_t *il, int i0, int iN)
{
  int n;
  lsTripleInt_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsTripleInt_Drop(il,i0);
  return (lsTripleInt_take(il_to,iN));
}

lsTripleInt_t *lsTripleInt_rangePt(lsTripleInt_t *il, int i0, int iN)
{
  return (lsTripleInt_dropPt(lsTripleInt_take(il,iN),i0));
}

lsTripleInt_t *lsTripleInt_cpyRange(lsTripleInt_t *il_to, lsTripleInt_t *il, int i0, int iN)
{
  lsTripleInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsTripleInt_rangePt(lsTripleInt_cpyPt(lsTripleInt_nil(&tmp),il),i0,iN);
  return (lsTripleInt_cpy(il_to,&tmp));
}

lsTripleInt_t *lsTripleInt_catRange(lsTripleInt_t *il_to, lsTripleInt_t *il, int i0, int iN)
{
  lsTripleInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsTripleInt_rangePt(lsTripleInt_cpyPt(lsTripleInt_nil(&tmp),il),i0,iN);
  return (lsTripleInt_cat(il_to,&tmp));
}
  
lsTripleInt_t *lsTripleInt_reverse(lsTripleInt_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    tripleInt_t tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsTripleInt_makeIndex(lsInt_t *index, lsTripleInt_t *il)
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
lsTripleInt_t * lsTripleInt_join(lsTripleInt_t *il_to, lsTripleInt_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsTripleInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    tripleInt_t x = LS_GET(il_from,i);
    if (lsTripleInt_elem(il_to,x)) continue;

    lsTripleInt_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsTripleInt_t * lsTripleInt_filterByValue(lsTripleInt_t *il, tripleInt_t undef, lsInt_t *indices)
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

lsTripleInt_t * lsTripleInt_CpyFilterByValue(lsTripleInt_t *il, tripleInt_t undef, lsInt_t *indices)
{
  return (lsTripleInt_cpyFilterByValue(NULL,il,undef,indices));
}

lsTripleInt_t * lsTripleInt_cpyFilterByValue(lsTripleInt_t *il_to, lsTripleInt_t *il_from, tripleInt_t undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsTripleInt_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsTripleInt_t * lsTripleInt_filterByIndex(lsTripleInt_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsTripleInt_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsTripleInt_t * lsTripleInt_CpyFilterByIndex(lsTripleInt_t *il, lsInt_t *indices)
{
  return (lsTripleInt_cpyFilterByIndex(NULL,il,indices));
}

lsTripleInt_t * lsTripleInt_cpyFilterByIndex(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsTripleInt_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsTripleInt_t * lsTripleInt_joinInts(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsTripleInt_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsTripleInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsTripleInt_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsTripleInt_elem(lsTripleInt_t * il, tripleInt_t item)
{
  return (lsTripleInt_getLastIndex(il,item) >= 0);
}

int lsTripleInt_getLastIndex(lsTripleInt_t *il, tripleInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsTripleInt_getFstIndex(lsTripleInt_t *il, tripleInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsTripleInt_getIndex(lsTripleInt_t * il, tripleInt_t item)
{
  return (lsTripleInt_getLastIndex(il,item));
}

int lsTripleInt_neqElem(lsTripleInt_t * il, tripleInt_t item)
{
  return (lsTripleInt_getLastNeqIndex(il,item) >= 0);
}

int lsTripleInt_getLastNeqIndex(lsTripleInt_t * il, tripleInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsTripleInt_getFstNeqIndex(lsTripleInt_t * il, tripleInt_t item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsTripleInt_disjoint(lsTripleInt_t *il1, lsTripleInt_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsTripleInt_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsTripleInt_t *lsTripleInt_subst(lsTripleInt_t *il, tripleInt_t i, tripleInt_t j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsTripleInt_subBag(lsTripleInt_t *il_sub, lsTripleInt_t *il_super, tripleInt_t undef)
{
  return lsTripleInt_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsTripleInt_subBagIndices(lsInt_t *indices,
		     lsTripleInt_t *il_sub, lsTripleInt_t *il_super, tripleInt_t undef)
{
  lsTripleInt_t _sub;
  lsTripleInt_t _super;
  int i;
  tripleInt_t last;
  tripleInt_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsTripleInt_qsortLt(lsTripleInt_cpy(lsTripleInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsTripleInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsTripleInt_take(&_super,last_index);
      if ((last_index = lsTripleInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsTripleInt_cpyPt(lsTripleInt_nil(&_super),il_super);

      if ((last_index = lsTripleInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsTripleInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsTripleInt_subBagLimitedIndices(lsInt_t *indices,
			    lsTripleInt_t *il_sub, lsTripleInt_t *il_super, 
			    lsInt_t *limit, tripleInt_t undef)
{
  lsTripleInt_t _sub;
  lsTripleInt_t _super;
  int i;
  tripleInt_t last;
  tripleInt_t item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsTripleInt_qsortLt(lsTripleInt_cpy(lsTripleInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsTripleInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsTripleInt_take(&_super,last_index);
      if ((last_index = lsTripleInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsTripleInt_cpyPt(lsTripleInt_nil(&_super),il_super);
      if (limit) lsTripleInt_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsTripleInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsTripleInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsTripleInt_elemFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func)
{
 if (!func) rs_error("lsTripleInt_elemFunc: cmp-function undefined.");
 return (lsTripleInt_getIndexFunc(il,item,func) >= 0);
}

int lsTripleInt_getIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsTripleInt_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsTripleInt_neqElemFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func)
{
  if (!func) rs_error("lsTripleInt_neqElemFunc: cmp-function undefined.");
  return (lsTripleInt_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsTripleInt_getLastNeqIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsTripleInt_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsTripleInt_getFstNeqIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsTripleInt_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsTripleInt_t * lsTripleInt_insert(lsTripleInt_t *il, int index, tripleInt_t item, tripleInt_t item0)
{
  int i;
  if (!il)
    il = lsTripleInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsTripleInt_insert: illegal index %d.",index);

    lsTripleInt_setIndex(il,index,item,item0);
    return (il);
  }
  lsTripleInt_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(tripleInt_t));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsTripleInt_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsTripleInt_t * lsTripleInt_insertN(lsTripleInt_t *il, int index, int n, tripleInt_t item, 
		    tripleInt_t item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsTripleInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsTripleInt_insert: illegal index %d.",index);

    lsTripleInt_setIndex(il,index,item,item0);
    lsTripleInt_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsTripleInt_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(tripleInt_t));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsTripleInt_t * lsTripleInt_insSwap(lsTripleInt_t *il, int index, tripleInt_t item, tripleInt_t item0)
{
  int i,n;
  tripleInt_t _item;

  if (!il)
    il = lsTripleInt_Nil();

  if (index < 0)
    rs_error("lsTripleInt_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsTripleInt_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsTripleInt_add(il,_item);
#else
  if (index < n)
    lsTripleInt_add(il,_item);
#endif

  return (il);
}

tripleInt_t lsTripleInt_getFlip(int i, lsTripleInt_t *il, tripleInt_t undef)
{
  tripleInt_t item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsTripleInt_t * lsTripleInt_map(lsTripleInt_t * il, lsTripleInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_map_2(lsTripleInt_t * il, lsTripleInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_map_3(lsTripleInt_t * il, lsTripleInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_mapSet(lsTripleInt_t * il, lsTripleInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_mapSet_2(lsTripleInt_t * il, lsTripleInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_mapSet_3(lsTripleInt_t * il, lsTripleInt_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_CpyMap(lsTripleInt_t * il_from, lsTripleInt_map_t *func)
{
  return (lsTripleInt_cpyMap(NULL,il_from,func));
}

lsTripleInt_t * lsTripleInt_cpyMap(lsTripleInt_t * il_to, lsTripleInt_t * il_from, lsTripleInt_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsTripleInt_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsTripleInt_t *  lsTripleInt_CpyMap_2(lsTripleInt_t * il_from, lsTripleInt_map_2_t *func, void *arg)
{
  return (lsTripleInt_cpyMap_2(NULL, il_from, func, arg));
}

lsTripleInt_t *  lsTripleInt_CpyMap_3(lsTripleInt_t * il_from, lsTripleInt_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsTripleInt_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsTripleInt_t *  lsTripleInt_cpyMap_2(lsTripleInt_t * il_to, lsTripleInt_t * il_from, 
		      lsTripleInt_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsTripleInt_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsTripleInt_t *  lsTripleInt_cpyMap_3(lsTripleInt_t * il_to, lsTripleInt_t * il_from, 
		      lsTripleInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsTripleInt_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsTripleInt_t * lsTripleInt_CartProd(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func)
{
  return (lsTripleInt_cpyCartProd(NULL,il1,il2,func));
}

lsTripleInt_t * lsTripleInt_cpyCartProd(lsTripleInt_t *il_to, 
			lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsTripleInt_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsTripleInt_cpy(il_to,il1);
  if (!il_to)
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsTripleInt_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsTripleInt_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsTripleInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsTripleInt_t * lsTripleInt_cartProd(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func)
{
  int i,j;
  lsTripleInt_t tmp;
  if (LS_isNIL(il1)) return lsTripleInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsTripleInt_cpy(lsTripleInt_nil(&tmp),il1);
  lsTripleInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsTripleInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsTripleInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsTripleInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsTripleInt_t * lsTripleInt_cartProd_2(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsTripleInt_t tmp;
  if (LS_isNIL(il1)) return lsTripleInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsTripleInt_cpy(lsTripleInt_nil(&tmp),il1);
  lsTripleInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsTripleInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsTripleInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsTripleInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsTripleInt_t * lsTripleInt_filter(lsTripleInt_t * il, lsTripleInt_filter_t *func)
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

lsTripleInt_t * lsTripleInt_CpyFilter(lsTripleInt_t *il_from, lsTripleInt_filter_t *func)
{
  return (lsTripleInt_cpyFilter(NULL,il_from,func));
}

lsTripleInt_t * lsTripleInt_cpyFilter(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsTripleInt_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsTripleInt_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

tripleInt_t lsTripleInt_foldl(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_t *func)
{
  int i;
  tripleInt_t result = item0;
  if (!func)
    rs_error("lsTripleInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

tripleInt_t lsTripleInt_foldr(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_t *func)
{
  int i;
  tripleInt_t result = item0;
  if (!func)
    rs_error("lsTripleInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

tripleInt_t lsTripleInt_foldl_2(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_2_t *func,
		     void *arg)
{
  int i;
  tripleInt_t result = item0;
  if (!func)
    rs_error("lsTripleInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

tripleInt_t lsTripleInt_foldr_2(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_2_t *func,
		     void *arg)
{
  int i;
  tripleInt_t result = item0;
  if (!func)
    rs_error("lsTripleInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsTripleInt_t * lsTripleInt_sscan_chr(lsTripleInt_t *il, char t, char *s)
{
  char *p;
  tripleInt_t v;

  if (!il)
    il = lsTripleInt_Nil();
  else
    lsTripleInt_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsTripleInt_add(il,v);
    }
  }
  return (il);
}

char * lsTripleInt_sprint_chr(char *s, lsTripleInt_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    tripleInt_t x = LS_GET(il,i);
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

char * lsTripleInt_sprintf_chr(char *s, lsTripleInt_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    tripleInt_t x = LS_GET(il,i);
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
 
int lsTripleInt_fwrite(FILE *fp, lsTripleInt_t *il)
{
    tripleInt_t *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(tripleInt_t), m, fp)) != m) {
	    rs_error("lsTripleInt_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(tripleInt_t), l, fp)) != l) {
	    rs_error("lsTripleInt_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsTripleInt_fread(lsTripleInt_t *il, int k, FILE *fp)
{
    tripleInt_t *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsTripleInt_realloc(lsTripleInt_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(tripleInt_t), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(tripleInt_t), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsTripleInt_lsTripleInt_minmax(lsTripleInt_t *il, int mode, lsTripleInt_cmp_t *func)
{
  int i,iminmax;
  tripleInt_t minmax;
  if (LS_isNIL(il)) 
    rs_error("lsTripleInt_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    tripleInt_t tmp = LS_GET(il,i);
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
      rs_error("_lsTripleInt_lsTripleInt_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

tripleInt_t lsTripleInt_maxFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  int i = _lsTripleInt_lsTripleInt_minmax(il,3,func);
  return (LS_GET(il,i));
}

tripleInt_t lsTripleInt_minFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  int i = _lsTripleInt_lsTripleInt_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsTripleInt_maxIndexFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  return _lsTripleInt_lsTripleInt_minmax(il,3,func);
}

int lsTripleInt_minIndexFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  return _lsTripleInt_lsTripleInt_minmax(il,2,func);
}

lsTripleInt_t * lsTripleInt_sortByIndex(lsTripleInt_t *il, lsInt_t *index, tripleInt_t undef)
{
  lsTripleInt_t *tmp = NULL;
  tmp = lsTripleInt_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsTripleInt_cpy(il,tmp);
      lsTripleInt_Free(tmp);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_cpySortByIndex(lsTripleInt_t *il_to, lsTripleInt_t *il_from, 
			   lsInt_t *index, tripleInt_t undef)
{
  int i;
  lsTripleInt_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsTripleInt_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    tripleInt_t item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsTripleInt_setIndex(il_to,i,item,undef);
    }
#else
    lsTripleInt_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsTripleInt_qsortX(tripleInt_t *v, int left, int right, 
		int mode, lsTripleInt_cmp_t *func)
{
  tripleInt_t x = v[left]; /* bestimme Trennelement */
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
      tripleInt_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsTripleInt_qsortX(v,left,r,mode,func);
  if (l < right) _lsTripleInt_qsortX(v,l,right,mode,func);
}

lsTripleInt_t * lsTripleInt_qsortLtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_qsortGtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsTripleInt_qsortX_2(tripleInt_t *v, int left, int right, 
		  int mode, lsTripleInt_cmp_2_t *func, void *arg)
{
  tripleInt_t x = v[left]; /* bestimme Trennelement */
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
      tripleInt_t h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsTripleInt_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsTripleInt_qsortX_2(v,l,right,mode,func,arg);
}

lsTripleInt_t * lsTripleInt_qsortLtFunc_2(lsTripleInt_t *il, lsTripleInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_qsortGtFunc_2(lsTripleInt_t *il, lsTripleInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsTripleInt_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsTripleInt_t *il = tplFst(arg);
  lsTripleInt_cmp_t *func = (lsTripleInt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      tripleInt_t x = LS_GET(il,i);
      tripleInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsTripleInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsTripleInt_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsTripleInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      tripleInt_t x = LS_GET(il,i);
      tripleInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsTripleInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsTripleInt_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsTripleInt_t *il = tplFst(arg);
  lsTripleInt_cmp_t *func = (lsTripleInt_cmp_t *) tplSnd(arg);
  tripleInt_t *value = (tripleInt_t *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      tripleInt_t x = *value;
      tripleInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsTripleInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsTripleInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsTripleInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  tripleInt_t *value = (tripleInt_t *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      tripleInt_t x = *value;
      tripleInt_t y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsTripleInt_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsTripleInt_qsortIndexLtFunc(lsInt_t *index, lsTripleInt_t *il, 
			      lsTripleInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsTripleInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsTripleInt_qsortIndexLt(lsInt_t *index, lsTripleInt_t *il)
{
  return (lsTripleInt_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsTripleInt_qsortIndexGtFunc(lsInt_t *index, lsTripleInt_t *il, 
			      lsTripleInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsTripleInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsTripleInt_qsortIndexGt(lsInt_t *index, lsTripleInt_t *il)
{
  return (lsTripleInt_qsortIndexGtFunc(index,il,NULL));
}

void _lsTripleInt_mergeX(tripleInt_t *v, tripleInt_t *w, int ll, int rl, int rr,
		int mode, lsTripleInt_cmp_t *func)
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
      rs_error("_lsTripleInt_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsTripleInt_msortX(tripleInt_t *v, tripleInt_t *w, int left, int right, 
		int mode, lsTripleInt_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsTripleInt_msortX(v,w,left,m,mode,func);
  _lsTripleInt_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsTripleInt_mergeX(w,v,left,m,right,mode,func);
}

lsTripleInt_t * lsTripleInt_msortLtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  static lsTripleInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsTripleInt_realloc(_il,LS_N(il));
    _lsTripleInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_msortGtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func)
{
  static lsTripleInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsTripleInt_realloc(_il,LS_N(il));
    _lsTripleInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

tripleInt_t lsTripleInt_sum(lsTripleInt_t *il)
{
  tripleInt_t sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

tripleInt_t lsTripleInt_prod(lsTripleInt_t *il)
{
  tripleInt_t sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsTripleInt_t * lsTripleInt_scale(lsTripleInt_t *il, tripleInt_t s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_delta(lsTripleInt_t *il_to, lsTripleInt_t *il_from, tripleInt_t base)
{
  int i;
  lsTripleInt_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsTripleInt_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

tripleInt_t lsTripleInt_max(lsTripleInt_t *il)
{
  int i = _lsTripleInt_lsTripleInt_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

tripleInt_t lsTripleInt_min(lsTripleInt_t *il)
{
  int i = _lsTripleInt_lsTripleInt_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsTripleInt_maxIndex(lsTripleInt_t *il)
{
  return _lsTripleInt_lsTripleInt_minmax(il,1,NULL);
}

int lsTripleInt_minIndex(lsTripleInt_t *il)
{
  return _lsTripleInt_lsTripleInt_minmax(il,0,NULL);
}

lsTripleInt_t *lsTripleInt_rmdup(lsTripleInt_t *il)
{
  int i,j;
  tripleInt_t item;
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
      
    
lsTripleInt_t * lsTripleInt_qsortLt(lsTripleInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_qsortGt(lsTripleInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsTripleInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_msortLt(lsTripleInt_t *il)
{
  static lsTripleInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsTripleInt_realloc(_il,LS_N(il));
    _lsTripleInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsTripleInt_t * lsTripleInt_msortGt(lsTripleInt_t *il)
{
  static lsTripleInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsTripleInt_realloc(_il,LS_N(il));
    _lsTripleInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsTripleInt_bsearchX(lsTripleInt_t *il, tripleInt_t i,
		 int mode, lsTripleInt_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    tripleInt_t x = LS_GET(il,m);
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
      rs_error("_lsTripleInt_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsTripleInt_bsearchLtFunc(lsTripleInt_t *il, tripleInt_t i, lsTripleInt_cmp_t *func)
{
  return (_lsTripleInt_bsearchX(il,i,2,func));
}

int lsTripleInt_bsearchGtFunc(lsTripleInt_t *il, tripleInt_t i, lsTripleInt_cmp_t *func)
{
  return (_lsTripleInt_bsearchX(il,i,3,func));
}

int _lsTripleInt_bsearchX_2(lsTripleInt_t *il, tripleInt_t i,
		   int mode, lsTripleInt_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    tripleInt_t x = LS_GET(il,m);
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
      rs_error("_lsTripleInt_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsTripleInt_bsearchLtFunc_2(lsTripleInt_t *il,tripleInt_t i,lsTripleInt_cmp_2_t *func,void *arg)
{
  return (_lsTripleInt_bsearchX_2(il,i,2,func,arg));
}

int lsTripleInt_bsearchGtFunc_2(lsTripleInt_t *il,tripleInt_t i,lsTripleInt_cmp_2_t *func,void *arg)
{
  return (_lsTripleInt_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsTripleInt_bsearchLt(lsTripleInt_t *il, tripleInt_t i)
{
  return (_lsTripleInt_bsearchX(il,i,0,NULL));
}

int lsTripleInt_bsearchGt(lsTripleInt_t *il, tripleInt_t i)
{
  return (_lsTripleInt_bsearchX(il,i,1,NULL));
}

#endif 

int lsTripleInt_cmpFunc(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_cmp_t *func)
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
      tripleInt_t x = LS_GET(il1,i);
      tripleInt_t y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsTripleInt_cmpFunc: no cmp-function defined");
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

int lsTripleInt_cmp(lsTripleInt_t *il1, lsTripleInt_t *il2)
{
  return (lsTripleInt_cmpFunc(il1,il2,NULL));
}

#endif
  

