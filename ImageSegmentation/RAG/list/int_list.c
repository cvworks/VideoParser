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
#include "lsInt.h"
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

lsInt_t * lsInt_Nil(void)
{
  return (lsInt_nil(NULL));
}

lsInt_t * lsInt_realloc(lsInt_t *il, int n)
{
  if (!il) il = lsInt_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(int), 
			  "list items");
  }
  return (il);
}

lsInt_t *  lsInt_nil(lsInt_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsInt_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsInt_t * lsInt_ConsNil(int i)
{
  lsInt_t * il = rs_malloc(sizeof(lsInt_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(int), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsInt_setIndex(lsInt_t *il, int index, int i, int i0)
{
  int ret;
  int j;

  if (!il)
    rs_error("lsInt_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsInt_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(int), 
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


lsInt_t * lsInt_setIndices(lsInt_t *il, lsInt_t *indices, int x,
		       int undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsInt_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsInt_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsInt_t * lsInt_setNil(lsInt_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsInt_t * lsInt_nsetIndex(lsInt_t *il, int index, int n, int x, 
		      int undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsInt_Nil();

  lsInt_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsInt_t * lsInt_setConsNil(lsInt_t * il, int i)
{
  if (!il)
    return lsInt_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(int), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsInt_getNewItemIndex(lsInt_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsInt_getNewItem(il);
  return (index);
}

int *lsInt_getNewItem(lsInt_t *il)
{
  int *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(int), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsInt_t * lsInt_add(lsInt_t * il, int i)
{
  if (!il)
    il = lsInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(int), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsInt_t * lsInt_Add(lsInt_t * il, int i)
{
  lsInt_t *il_to = lsInt_Nil();
  if (!il)
    return (lsInt_setConsNil(il_to,i));

  lsInt_realloc(il_to, il->n_list+LIST_BUF);
  lsInt_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsInt_t * lsInt_Cons(lsInt_t * il, int i)
{
  lsInt_t *il_to = lsInt_Nil();
  if (!il)
    return (lsInt_setConsNil(il_to,i));

  lsInt_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(int)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsInt_t * lsInt_cons(lsInt_t *il, int i)
{
  if (!il)
    il = lsInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(int), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(int));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

int lsInt_last(lsInt_t *il, int undef)
{
  return (LS_LAST_CHECK(il,undef));
}

int lsInt_head(lsInt_t *il, int undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

int lsInt_popLast(lsInt_t *il, int undef)
{
  int x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsInt_init(il);
  return (x);
}

int lsInt_popHead(lsInt_t *il, int undef)
{
  int x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsInt_tail(il);
  return (x);
}

lsInt_t * lsInt_init(lsInt_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsInt_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsInt_t * lsInt_Init(lsInt_t * il)
{
  lsInt_t *il_to = lsInt_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsInt_Init: got empty list");
  }
  lsInt_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsInt_t * lsInt_tail(lsInt_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsInt_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(int));
  return (il);
}
 
lsInt_t * lsInt_Tail(lsInt_t *il)
{
  lsInt_t *il_to = lsInt_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsInt_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsInt_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(int));
  return (il_to);
}


lsInt_t * lsInt_take(lsInt_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsInt_t * lsInt_Take(lsInt_t *il, int n)
{
  lsInt_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsInt_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(int));
  LS_N(il_to) = m;
  return (il_to);
}

int lsInt_delSwap(lsInt_t *il, int i)
{
  int x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

int lsInt_delete(lsInt_t *il, int index, int undef)
{
  int x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(int));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsInt_Free(lsInt_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsInt_free(lsInt_t * il)
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

int lsInt_get(lsInt_t * il,int i)
{
  if (!il)
    rs_error("lsInt_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsInt_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

int lsInt_getCheck(lsInt_t * il,int i,int undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsInt_length(lsInt_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsInt_t *  lsInt_getRowPt(lsInt_t * row, lsInt_t * il, int i, int cols)
{
  row = lsInt_take(lsInt_dropPt(lsInt_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsInt_t *  lsInt_getRow(lsInt_t *row, lsInt_t *il, int i, int cols)
{
  lsInt_t rowPt;
  lsInt_getRowPt(lsInt_init(&rowPt), il, i, cols);

  lsInt_cpy(row, &rowPt);

  return (row);
}

lsInt_t *  lsInt_getCol(lsInt_t *col, lsInt_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int *item;

  if (n <= j)
    return (lsInt_setNil(col));

  col = lsInt_realloc(col, n / cols + 1);
  lsInt_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsInt_add(col,*item);

  return (col);
}

lsInt_t *  lsInt_setRow(lsInt_t *il, lsInt_t *row, int i, int cols)
{
  lsInt_t rowPt;
  int n;
  lsInt_getRowPt(lsInt_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsInt_cpy(&rowPt,lsInt_take(row,n));
  return (il);
}

lsInt_t *  lsInt_setCol(lsInt_t *il, lsInt_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  int *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsInt_t *  lsInt_SetPt(int n, int * items)
{
  return (lsInt_setPt(NULL,n,items));
}

lsInt_t *  lsInt_setPt(lsInt_t * il_to, int n, int * items)
{
  if (!il_to)
    il_to = lsInt_Nil();
  lsInt_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsInt_t *  lsInt_CpyPt(lsInt_t * il_from)
{
  return (lsInt_cpyPt(NULL, il_from));
}

lsInt_t *  lsInt_cpyPt(lsInt_t * il_to, lsInt_t * il_from)
{
  if (!il_to)
    il_to = lsInt_Nil();
  lsInt_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsInt_t * lsInt_Cpy(const lsInt_t * il_from)
{
  return (lsInt_cpy(NULL,il_from));
}

lsInt_t * lsInt_cpy(lsInt_t * il_to, const lsInt_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (!il_from) return (il_to);

  lsInt_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(int));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsInt_t * lsInt_Cat(lsInt_t * il_1, lsInt_t * il_2)
{
  return (lsInt_cat(lsInt_Cpy(il_1), il_2));
}

lsInt_t * lsInt_cat(lsInt_t * il_to, lsInt_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsInt_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsInt_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(int));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsInt_t * lsInt_addCat(lsInt_t *il_to, lsInt_t *il)
{
  return lsInt_add(il_to, lsInt_Cat(il_to,il));
}

lsInt_t * lsInt_AddCat(lsInt_t *il_to, lsInt_t *il)
{
  return lsInt_Add(il_to, lsInt_Cat(il_to,il));
}
#endif

lsInt_t * lsInt_drop(lsInt_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsInt_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(int));
  return (il);
}

lsInt_t * lsInt_Drop(lsInt_t *il, int i)
{
  lsInt_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsInt_setPt(lsInt_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsInt_Cpy(&tmp);
  return (il_split);
}

lsInt_t * lsInt_dropPt(lsInt_t *il, int i)
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

lsInt_t * lsInt_split(lsInt_t *il, int i)
{
  lsInt_t *il_drop = lsInt_Drop(il,i);
  lsInt_take(il,i);
  return (il_drop);
}

lsPt_t * lsInt_nsplit(lsPt_t *il_split, lsInt_t *il, lsInt_t *is)
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
      lsInt_t *split = lsInt_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsInt_t *lsInt_range(lsInt_t *il, int i0, int iN)
{
  lsInt_take(il,iN);
  if (i0 > 0) {
    lsInt_drop(il,i0-1);
  }
  return (il);
}

lsInt_t *lsInt_Range(lsInt_t *il, int i0, int iN)
{
  int n;
  lsInt_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsInt_Drop(il,i0);
  return (lsInt_take(il_to,iN));
}

lsInt_t *lsInt_rangePt(lsInt_t *il, int i0, int iN)
{
  return (lsInt_dropPt(lsInt_take(il,iN),i0));
}

lsInt_t *lsInt_cpyRange(lsInt_t *il_to, lsInt_t *il, int i0, int iN)
{
  lsInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsInt_rangePt(lsInt_cpyPt(lsInt_nil(&tmp),il),i0,iN);
  return (lsInt_cpy(il_to,&tmp));
}

lsInt_t *lsInt_catRange(lsInt_t *il_to, lsInt_t *il, int i0, int iN)
{
  lsInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsInt_rangePt(lsInt_cpyPt(lsInt_nil(&tmp),il),i0,iN);
  return (lsInt_cat(il_to,&tmp));
}
  
lsInt_t *lsInt_reverse(lsInt_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    int tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsInt_makeIndex(lsInt_t *index, lsInt_t *il)
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
lsInt_t * lsInt_join(lsInt_t *il_to, lsInt_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (lsInt_elem(il_to,x)) continue;

    lsInt_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsInt_t * lsInt_filterByValue(lsInt_t *il, int undef, lsInt_t *indices)
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

lsInt_t * lsInt_CpyFilterByValue(lsInt_t *il, int undef, lsInt_t *indices)
{
  return (lsInt_cpyFilterByValue(NULL,il,undef,indices));
}

lsInt_t * lsInt_cpyFilterByValue(lsInt_t *il_to, lsInt_t *il_from, int undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsInt_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsInt_t * lsInt_filterByIndex(lsInt_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsInt_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsInt_t * lsInt_CpyFilterByIndex(lsInt_t *il, lsInt_t *indices)
{
  return (lsInt_cpyFilterByIndex(NULL,il,indices));
}

lsInt_t * lsInt_cpyFilterByIndex(lsInt_t *il_to, lsInt_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsInt_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsInt_t * lsInt_joinInts(lsInt_t *il_to, lsInt_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsInt_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsInt_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsInt_elem(lsInt_t * il, int item)
{
  return (lsInt_getLastIndex(il,item) >= 0);
}

int lsInt_getLastIndex(lsInt_t *il, int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsInt_getFstIndex(lsInt_t *il, int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsInt_getIndex(lsInt_t * il, int item)
{
  return (lsInt_getLastIndex(il,item));
}

int lsInt_neqElem(lsInt_t * il, int item)
{
  return (lsInt_getLastNeqIndex(il,item) >= 0);
}

int lsInt_getLastNeqIndex(lsInt_t * il, int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsInt_getFstNeqIndex(lsInt_t * il, int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsInt_disjoint(lsInt_t *il1, lsInt_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsInt_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsInt_t *lsInt_subst(lsInt_t *il, int i, int j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsInt_subBag(lsInt_t *il_sub, lsInt_t *il_super, int undef)
{
  return lsInt_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsInt_subBagIndices(lsInt_t *indices,
		     lsInt_t *il_sub, lsInt_t *il_super, int undef)
{
  lsInt_t _sub;
  lsInt_t _super;
  int i;
  int last;
  int item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsInt_qsortLt(lsInt_cpy(lsInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsInt_take(&_super,last_index);
      if ((last_index = lsInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsInt_cpyPt(lsInt_nil(&_super),il_super);

      if ((last_index = lsInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsInt_subBagLimitedIndices(lsInt_t *indices,
			    lsInt_t *il_sub, lsInt_t *il_super, 
			    lsInt_t *limit, int undef)
{
  lsInt_t _sub;
  lsInt_t _super;
  int i;
  int last;
  int item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsInt_qsortLt(lsInt_cpy(lsInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsInt_take(&_super,last_index);
      if ((last_index = lsInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsInt_cpyPt(lsInt_nil(&_super),il_super);
      if (limit) lsInt_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsInt_elemFunc(lsInt_t * il, int item, lsInt_cmp_t *func)
{
 if (!func) rs_error("lsInt_elemFunc: cmp-function undefined.");
 return (lsInt_getIndexFunc(il,item,func) >= 0);
}

int lsInt_getIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsInt_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsInt_neqElemFunc(lsInt_t * il, int item, lsInt_cmp_t *func)
{
  if (!func) rs_error("lsInt_neqElemFunc: cmp-function undefined.");
  return (lsInt_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsInt_getLastNeqIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsInt_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsInt_getFstNeqIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsInt_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsInt_t * lsInt_insert(lsInt_t *il, int index, int item, int item0)
{
  int i;
  if (!il)
    il = lsInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsInt_insert: illegal index %d.",index);

    lsInt_setIndex(il,index,item,item0);
    return (il);
  }
  lsInt_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(int));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsInt_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsInt_t * lsInt_insertN(lsInt_t *il, int index, int n, int item, 
		    int item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsInt_insert: illegal index %d.",index);

    lsInt_setIndex(il,index,item,item0);
    lsInt_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsInt_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(int));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsInt_t * lsInt_insSwap(lsInt_t *il, int index, int item, int item0)
{
  int i,n;
  int _item;

  if (!il)
    il = lsInt_Nil();

  if (index < 0)
    rs_error("lsInt_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsInt_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsInt_add(il,_item);
#else
  if (index < n)
    lsInt_add(il,_item);
#endif

  return (il);
}

int lsInt_getFlip(int i, lsInt_t *il, int undef)
{
  int item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsInt_t * lsInt_map(lsInt_t * il, lsInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsInt_t * lsInt_map_2(lsInt_t * il, lsInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsInt_t * lsInt_map_3(lsInt_t * il, lsInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsInt_t * lsInt_mapSet(lsInt_t * il, lsInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsInt_t * lsInt_mapSet_2(lsInt_t * il, lsInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsInt_t * lsInt_mapSet_3(lsInt_t * il, lsInt_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsInt_t * lsInt_CpyMap(lsInt_t * il_from, lsInt_map_t *func)
{
  return (lsInt_cpyMap(NULL,il_from,func));
}

lsInt_t * lsInt_cpyMap(lsInt_t * il_to, lsInt_t * il_from, lsInt_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsInt_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsInt_t *  lsInt_CpyMap_2(lsInt_t * il_from, lsInt_map_2_t *func, void *arg)
{
  return (lsInt_cpyMap_2(NULL, il_from, func, arg));
}

lsInt_t *  lsInt_CpyMap_3(lsInt_t * il_from, lsInt_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsInt_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsInt_t *  lsInt_cpyMap_2(lsInt_t * il_to, lsInt_t * il_from, 
		      lsInt_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsInt_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsInt_t *  lsInt_cpyMap_3(lsInt_t * il_to, lsInt_t * il_from, 
		      lsInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsInt_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsInt_t * lsInt_CartProd(lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func)
{
  return (lsInt_cpyCartProd(NULL,il1,il2,func));
}

lsInt_t * lsInt_cpyCartProd(lsInt_t *il_to, 
			lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsInt_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsInt_cpy(il_to,il1);
  if (!il_to)
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsInt_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsInt_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsInt_t * lsInt_cartProd(lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func)
{
  int i,j;
  lsInt_t tmp;
  if (LS_isNIL(il1)) return lsInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsInt_cpy(lsInt_nil(&tmp),il1);
  lsInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsInt_t * lsInt_cartProd_2(lsInt_t *il1, lsInt_t *il2, lsInt_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsInt_t tmp;
  if (LS_isNIL(il1)) return lsInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsInt_cpy(lsInt_nil(&tmp),il1);
  lsInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsInt_t * lsInt_filter(lsInt_t * il, lsInt_filter_t *func)
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

lsInt_t * lsInt_CpyFilter(lsInt_t *il_from, lsInt_filter_t *func)
{
  return (lsInt_cpyFilter(NULL,il_from,func));
}

lsInt_t * lsInt_cpyFilter(lsInt_t *il_to, lsInt_t *il_from, lsInt_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsInt_Nil();
  else
    lsInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsInt_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

int lsInt_foldl(lsInt_t *il, int item0, lsInt_fold_t *func)
{
  int i;
  int result = item0;
  if (!func)
    rs_error("lsInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

int lsInt_foldr(lsInt_t *il, int item0, lsInt_fold_t *func)
{
  int i;
  int result = item0;
  if (!func)
    rs_error("lsInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

int lsInt_foldl_2(lsInt_t *il, int item0, lsInt_fold_2_t *func,
		     void *arg)
{
  int i;
  int result = item0;
  if (!func)
    rs_error("lsInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

int lsInt_foldr_2(lsInt_t *il, int item0, lsInt_fold_2_t *func,
		     void *arg)
{
  int i;
  int result = item0;
  if (!func)
    rs_error("lsInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsInt_t * lsInt_sscan_chr(lsInt_t *il, char t, char *s)
{
  char *p;
  int v;

  if (!il)
    il = lsInt_Nil();
  else
    lsInt_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsInt_add(il,v);
    }
  }
  return (il);
}

char * lsInt_sprint_chr(char *s, lsInt_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    int x = LS_GET(il,i);
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

char * lsInt_sprintf_chr(char *s, lsInt_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    int x = LS_GET(il,i);
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
 
int lsInt_fwrite(FILE *fp, lsInt_t *il)
{
    int *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(int), m, fp)) != m) {
	    rs_error("lsInt_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(int), l, fp)) != l) {
	    rs_error("lsInt_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsInt_fread(lsInt_t *il, int k, FILE *fp)
{
    int *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsInt_realloc(lsInt_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(int), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(int), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsInt_lsInt_minmax(lsInt_t *il, int mode, lsInt_cmp_t *func)
{
  int i,iminmax;
  int minmax;
  if (LS_isNIL(il)) 
    rs_error("lsInt_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    int tmp = LS_GET(il,i);
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
      rs_error("_lsInt_lsInt_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

int lsInt_maxFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  int i = _lsInt_lsInt_minmax(il,3,func);
  return (LS_GET(il,i));
}

int lsInt_minFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  int i = _lsInt_lsInt_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsInt_maxIndexFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  return _lsInt_lsInt_minmax(il,3,func);
}

int lsInt_minIndexFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  return _lsInt_lsInt_minmax(il,2,func);
}

lsInt_t * lsInt_sortByIndex(lsInt_t *il, lsInt_t *index, int undef)
{
  lsInt_t *tmp = NULL;
  tmp = lsInt_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsInt_cpy(il,tmp);
      lsInt_Free(tmp);
  }
  return (il);
}

lsInt_t * lsInt_cpySortByIndex(lsInt_t *il_to, lsInt_t *il_from, 
			   lsInt_t *index, int undef)
{
  int i;
  lsInt_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsInt_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    int item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsInt_setIndex(il_to,i,item,undef);
    }
#else
    lsInt_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsInt_qsortX(int *v, int left, int right, 
		int mode, lsInt_cmp_t *func)
{
  int x = v[left]; /* bestimme Trennelement */
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
      int h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsInt_qsortX(v,left,r,mode,func);
  if (l < right) _lsInt_qsortX(v,l,right,mode,func);
}

lsInt_t * lsInt_qsortLtFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsInt_t * lsInt_qsortGtFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsInt_qsortX_2(int *v, int left, int right, 
		  int mode, lsInt_cmp_2_t *func, void *arg)
{
  int x = v[left]; /* bestimme Trennelement */
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
      int h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsInt_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsInt_qsortX_2(v,l,right,mode,func,arg);
}

lsInt_t * lsInt_qsortLtFunc_2(lsInt_t *il, lsInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsInt_t * lsInt_qsortGtFunc_2(lsInt_t *il, lsInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsInt_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsInt_t *il = tplFst(arg);
  lsInt_cmp_t *func = (lsInt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      int x = LS_GET(il,i);
      int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsInt_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      int x = LS_GET(il,i);
      int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsInt_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsInt_t *il = tplFst(arg);
  lsInt_cmp_t *func = (lsInt_cmp_t *) tplSnd(arg);
  int *value = (int *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      int x = *value;
      int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  int *value = (int *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      int x = *value;
      int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsInt_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsInt_qsortIndexLtFunc(lsInt_t *index, lsInt_t *il, 
			      lsInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsInt_qsortIndexLt(lsInt_t *index, lsInt_t *il)
{
  return (lsInt_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsInt_qsortIndexGtFunc(lsInt_t *index, lsInt_t *il, 
			      lsInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsInt_qsortIndexGt(lsInt_t *index, lsInt_t *il)
{
  return (lsInt_qsortIndexGtFunc(index,il,NULL));
}

void _lsInt_mergeX(int *v, int *w, int ll, int rl, int rr,
		int mode, lsInt_cmp_t *func)
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
      rs_error("_lsInt_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsInt_msortX(int *v, int *w, int left, int right, 
		int mode, lsInt_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsInt_msortX(v,w,left,m,mode,func);
  _lsInt_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsInt_mergeX(w,v,left,m,right,mode,func);
}

lsInt_t * lsInt_msortLtFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  static lsInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsInt_realloc(_il,LS_N(il));
    _lsInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsInt_t * lsInt_msortGtFunc(lsInt_t *il, lsInt_cmp_t *func)
{
  static lsInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsInt_realloc(_il,LS_N(il));
    _lsInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsInt_sum(lsInt_t *il)
{
  int sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

int lsInt_prod(lsInt_t *il)
{
  int sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsInt_t * lsInt_scale(lsInt_t *il, int s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsInt_t * lsInt_delta(lsInt_t *il_to, lsInt_t *il_from, int base)
{
  int i;
  lsInt_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsInt_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

int lsInt_max(lsInt_t *il)
{
  int i = _lsInt_lsInt_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

int lsInt_min(lsInt_t *il)
{
  int i = _lsInt_lsInt_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsInt_maxIndex(lsInt_t *il)
{
  return _lsInt_lsInt_minmax(il,1,NULL);
}

int lsInt_minIndex(lsInt_t *il)
{
  return _lsInt_lsInt_minmax(il,0,NULL);
}

lsInt_t *lsInt_rmdup(lsInt_t *il)
{
  int i,j;
  int item;
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
      
    
lsInt_t * lsInt_qsortLt(lsInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsInt_t * lsInt_qsortGt(lsInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsInt_t * lsInt_msortLt(lsInt_t *il)
{
  static lsInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsInt_realloc(_il,LS_N(il));
    _lsInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsInt_t * lsInt_msortGt(lsInt_t *il)
{
  static lsInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsInt_realloc(_il,LS_N(il));
    _lsInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsInt_bsearchX(lsInt_t *il, int i,
		 int mode, lsInt_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    int x = LS_GET(il,m);
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
      rs_error("_lsInt_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsInt_bsearchLtFunc(lsInt_t *il, int i, lsInt_cmp_t *func)
{
  return (_lsInt_bsearchX(il,i,2,func));
}

int lsInt_bsearchGtFunc(lsInt_t *il, int i, lsInt_cmp_t *func)
{
  return (_lsInt_bsearchX(il,i,3,func));
}

int _lsInt_bsearchX_2(lsInt_t *il, int i,
		   int mode, lsInt_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    int x = LS_GET(il,m);
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
      rs_error("_lsInt_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsInt_bsearchLtFunc_2(lsInt_t *il,int i,lsInt_cmp_2_t *func,void *arg)
{
  return (_lsInt_bsearchX_2(il,i,2,func,arg));
}

int lsInt_bsearchGtFunc_2(lsInt_t *il,int i,lsInt_cmp_2_t *func,void *arg)
{
  return (_lsInt_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsInt_bsearchLt(lsInt_t *il, int i)
{
  return (_lsInt_bsearchX(il,i,0,NULL));
}

int lsInt_bsearchGt(lsInt_t *il, int i)
{
  return (_lsInt_bsearchX(il,i,1,NULL));
}

#endif 

int lsInt_cmpFunc(lsInt_t *il1, lsInt_t *il2, lsInt_cmp_t *func)
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
      int x = LS_GET(il1,i);
      int y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsInt_cmpFunc: no cmp-function defined");
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

int lsInt_cmp(lsInt_t *il1, lsInt_t *il2)
{
  return (lsInt_cmpFunc(il1,il2,NULL));
}

#endif
  

