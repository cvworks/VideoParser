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
#include "lsULInt.h"
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

lsULInt_t * lsULInt_Nil(void)
{
  return (lsULInt_nil(NULL));
}

lsULInt_t * lsULInt_realloc(lsULInt_t *il, int n)
{
  if (!il) il = lsULInt_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(unsigned long int), 
			  "list items");
  }
  return (il);
}

lsULInt_t *  lsULInt_nil(lsULInt_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsULInt_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsULInt_t * lsULInt_ConsNil(unsigned long int i)
{
  lsULInt_t * il = rs_malloc(sizeof(lsULInt_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(unsigned long int), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

unsigned long int lsULInt_setIndex(lsULInt_t *il, int index, unsigned long int i, unsigned long int i0)
{
  unsigned long int ret;
  int j;

  if (!il)
    rs_error("lsULInt_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsULInt_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(unsigned long int), 
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


lsULInt_t * lsULInt_setIndices(lsULInt_t *il, lsInt_t *indices, unsigned long int x,
		       unsigned long int undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsULInt_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsULInt_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsULInt_t * lsULInt_setNil(lsULInt_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsULInt_t * lsULInt_nsetIndex(lsULInt_t *il, int index, int n, unsigned long int x, 
		      unsigned long int undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsULInt_Nil();

  lsULInt_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsULInt_t * lsULInt_setConsNil(lsULInt_t * il, unsigned long int i)
{
  if (!il)
    return lsULInt_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(unsigned long int), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsULInt_getNewItemIndex(lsULInt_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsULInt_getNewItem(il);
  return (index);
}

unsigned long int *lsULInt_getNewItem(lsULInt_t *il)
{
  unsigned long int *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(unsigned long int), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsULInt_t * lsULInt_add(lsULInt_t * il, unsigned long int i)
{
  if (!il)
    il = lsULInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(unsigned long int), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsULInt_t * lsULInt_Add(lsULInt_t * il, unsigned long int i)
{
  lsULInt_t *il_to = lsULInt_Nil();
  if (!il)
    return (lsULInt_setConsNil(il_to,i));

  lsULInt_realloc(il_to, il->n_list+LIST_BUF);
  lsULInt_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsULInt_t * lsULInt_Cons(lsULInt_t * il, unsigned long int i)
{
  lsULInt_t *il_to = lsULInt_Nil();
  if (!il)
    return (lsULInt_setConsNil(il_to,i));

  lsULInt_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(unsigned long int)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsULInt_t * lsULInt_cons(lsULInt_t *il, unsigned long int i)
{
  if (!il)
    il = lsULInt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(unsigned long int), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(unsigned long int));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

unsigned long int lsULInt_last(lsULInt_t *il, unsigned long int undef)
{
  return (LS_LAST_CHECK(il,undef));
}

unsigned long int lsULInt_head(lsULInt_t *il, unsigned long int undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

unsigned long int lsULInt_popLast(lsULInt_t *il, unsigned long int undef)
{
  unsigned long int x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsULInt_init(il);
  return (x);
}

unsigned long int lsULInt_popHead(lsULInt_t *il, unsigned long int undef)
{
  unsigned long int x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsULInt_tail(il);
  return (x);
}

lsULInt_t * lsULInt_init(lsULInt_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsULInt_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsULInt_t * lsULInt_Init(lsULInt_t * il)
{
  lsULInt_t *il_to = lsULInt_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsULInt_Init: got empty list");
  }
  lsULInt_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsULInt_t * lsULInt_tail(lsULInt_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsULInt_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(unsigned long int));
  return (il);
}
 
lsULInt_t * lsULInt_Tail(lsULInt_t *il)
{
  lsULInt_t *il_to = lsULInt_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsULInt_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsULInt_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(unsigned long int));
  return (il_to);
}


lsULInt_t * lsULInt_take(lsULInt_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsULInt_t * lsULInt_Take(lsULInt_t *il, int n)
{
  lsULInt_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsULInt_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(unsigned long int));
  LS_N(il_to) = m;
  return (il_to);
}

unsigned long int lsULInt_delSwap(lsULInt_t *il, int i)
{
  unsigned long int x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

unsigned long int lsULInt_delete(lsULInt_t *il, int index, unsigned long int undef)
{
  unsigned long int x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(unsigned long int));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsULInt_Free(lsULInt_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsULInt_free(lsULInt_t * il)
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

unsigned long int lsULInt_get(lsULInt_t * il,int i)
{
  if (!il)
    rs_error("lsULInt_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsULInt_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

unsigned long int lsULInt_getCheck(lsULInt_t * il,int i,unsigned long int undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsULInt_length(lsULInt_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsULInt_t *  lsULInt_getRowPt(lsULInt_t * row, lsULInt_t * il, int i, int cols)
{
  row = lsULInt_take(lsULInt_dropPt(lsULInt_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsULInt_t *  lsULInt_getRow(lsULInt_t *row, lsULInt_t *il, int i, int cols)
{
  lsULInt_t rowPt;
  lsULInt_getRowPt(lsULInt_init(&rowPt), il, i, cols);

  lsULInt_cpy(row, &rowPt);

  return (row);
}

lsULInt_t *  lsULInt_getCol(lsULInt_t *col, lsULInt_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  unsigned long int *item;

  if (n <= j)
    return (lsULInt_setNil(col));

  col = lsULInt_realloc(col, n / cols + 1);
  lsULInt_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsULInt_add(col,*item);

  return (col);
}

lsULInt_t *  lsULInt_setRow(lsULInt_t *il, lsULInt_t *row, int i, int cols)
{
  lsULInt_t rowPt;
  int n;
  lsULInt_getRowPt(lsULInt_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsULInt_cpy(&rowPt,lsULInt_take(row,n));
  return (il);
}

lsULInt_t *  lsULInt_setCol(lsULInt_t *il, lsULInt_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  unsigned long int *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsULInt_t *  lsULInt_SetPt(int n, unsigned long int * items)
{
  return (lsULInt_setPt(NULL,n,items));
}

lsULInt_t *  lsULInt_setPt(lsULInt_t * il_to, int n, unsigned long int * items)
{
  if (!il_to)
    il_to = lsULInt_Nil();
  lsULInt_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsULInt_t *  lsULInt_CpyPt(lsULInt_t * il_from)
{
  return (lsULInt_cpyPt(NULL, il_from));
}

lsULInt_t *  lsULInt_cpyPt(lsULInt_t * il_to, lsULInt_t * il_from)
{
  if (!il_to)
    il_to = lsULInt_Nil();
  lsULInt_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsULInt_t * lsULInt_Cpy(const lsULInt_t * il_from)
{
  return (lsULInt_cpy(NULL,il_from));
}

lsULInt_t * lsULInt_cpy(lsULInt_t * il_to, const lsULInt_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (!il_from) return (il_to);

  lsULInt_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(unsigned long int));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsULInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsULInt_t * lsULInt_Cat(lsULInt_t * il_1, lsULInt_t * il_2)
{
  return (lsULInt_cat(lsULInt_Cpy(il_1), il_2));
}

lsULInt_t * lsULInt_cat(lsULInt_t * il_to, lsULInt_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsULInt_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsULInt_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(unsigned long int));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsULInt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsULInt_t * lsULInt_addCat(lsULInt_t *il_to, lsULInt_t *il)
{
  return lsULInt_add(il_to, lsULInt_Cat(il_to,il));
}

lsULInt_t * lsULInt_AddCat(lsULInt_t *il_to, lsULInt_t *il)
{
  return lsULInt_Add(il_to, lsULInt_Cat(il_to,il));
}
#endif

lsULInt_t * lsULInt_drop(lsULInt_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsULInt_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(unsigned long int));
  return (il);
}

lsULInt_t * lsULInt_Drop(lsULInt_t *il, int i)
{
  lsULInt_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsULInt_setPt(lsULInt_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsULInt_Cpy(&tmp);
  return (il_split);
}

lsULInt_t * lsULInt_dropPt(lsULInt_t *il, int i)
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

lsULInt_t * lsULInt_split(lsULInt_t *il, int i)
{
  lsULInt_t *il_drop = lsULInt_Drop(il,i);
  lsULInt_take(il,i);
  return (il_drop);
}

lsPt_t * lsULInt_nsplit(lsPt_t *il_split, lsULInt_t *il, lsInt_t *is)
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
      lsULInt_t *split = lsULInt_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsULInt_t *lsULInt_range(lsULInt_t *il, int i0, int iN)
{
  lsULInt_take(il,iN);
  if (i0 > 0) {
    lsULInt_drop(il,i0-1);
  }
  return (il);
}

lsULInt_t *lsULInt_Range(lsULInt_t *il, int i0, int iN)
{
  int n;
  lsULInt_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsULInt_Drop(il,i0);
  return (lsULInt_take(il_to,iN));
}

lsULInt_t *lsULInt_rangePt(lsULInt_t *il, int i0, int iN)
{
  return (lsULInt_dropPt(lsULInt_take(il,iN),i0));
}

lsULInt_t *lsULInt_cpyRange(lsULInt_t *il_to, lsULInt_t *il, int i0, int iN)
{
  lsULInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsULInt_rangePt(lsULInt_cpyPt(lsULInt_nil(&tmp),il),i0,iN);
  return (lsULInt_cpy(il_to,&tmp));
}

lsULInt_t *lsULInt_catRange(lsULInt_t *il_to, lsULInt_t *il, int i0, int iN)
{
  lsULInt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsULInt_rangePt(lsULInt_cpyPt(lsULInt_nil(&tmp),il),i0,iN);
  return (lsULInt_cat(il_to,&tmp));
}
  
lsULInt_t *lsULInt_reverse(lsULInt_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    unsigned long int tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsULInt_makeIndex(lsInt_t *index, lsULInt_t *il)
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
lsULInt_t * lsULInt_join(lsULInt_t *il_to, lsULInt_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsULInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    unsigned long int x = LS_GET(il_from,i);
    if (lsULInt_elem(il_to,x)) continue;

    lsULInt_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsULInt_t * lsULInt_filterByValue(lsULInt_t *il, unsigned long int undef, lsInt_t *indices)
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

lsULInt_t * lsULInt_CpyFilterByValue(lsULInt_t *il, unsigned long int undef, lsInt_t *indices)
{
  return (lsULInt_cpyFilterByValue(NULL,il,undef,indices));
}

lsULInt_t * lsULInt_cpyFilterByValue(lsULInt_t *il_to, lsULInt_t *il_from, unsigned long int undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsULInt_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsULInt_t * lsULInt_filterByIndex(lsULInt_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsULInt_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsULInt_t * lsULInt_CpyFilterByIndex(lsULInt_t *il, lsInt_t *indices)
{
  return (lsULInt_cpyFilterByIndex(NULL,il,indices));
}

lsULInt_t * lsULInt_cpyFilterByIndex(lsULInt_t *il_to, lsULInt_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsULInt_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsULInt_t * lsULInt_joinInts(lsULInt_t *il_to, lsULInt_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsULInt_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsULInt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsULInt_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsULInt_elem(lsULInt_t * il, unsigned long int item)
{
  return (lsULInt_getLastIndex(il,item) >= 0);
}

int lsULInt_getLastIndex(lsULInt_t *il, unsigned long int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsULInt_getFstIndex(lsULInt_t *il, unsigned long int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsULInt_getIndex(lsULInt_t * il, unsigned long int item)
{
  return (lsULInt_getLastIndex(il,item));
}

int lsULInt_neqElem(lsULInt_t * il, unsigned long int item)
{
  return (lsULInt_getLastNeqIndex(il,item) >= 0);
}

int lsULInt_getLastNeqIndex(lsULInt_t * il, unsigned long int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsULInt_getFstNeqIndex(lsULInt_t * il, unsigned long int item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsULInt_disjoint(lsULInt_t *il1, lsULInt_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsULInt_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsULInt_t *lsULInt_subst(lsULInt_t *il, unsigned long int i, unsigned long int j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsULInt_subBag(lsULInt_t *il_sub, lsULInt_t *il_super, unsigned long int undef)
{
  return lsULInt_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsULInt_subBagIndices(lsInt_t *indices,
		     lsULInt_t *il_sub, lsULInt_t *il_super, unsigned long int undef)
{
  lsULInt_t _sub;
  lsULInt_t _super;
  int i;
  unsigned long int last;
  unsigned long int item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsULInt_qsortLt(lsULInt_cpy(lsULInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsULInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsULInt_take(&_super,last_index);
      if ((last_index = lsULInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsULInt_cpyPt(lsULInt_nil(&_super),il_super);

      if ((last_index = lsULInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsULInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsULInt_subBagLimitedIndices(lsInt_t *indices,
			    lsULInt_t *il_sub, lsULInt_t *il_super, 
			    lsInt_t *limit, unsigned long int undef)
{
  lsULInt_t _sub;
  lsULInt_t _super;
  int i;
  unsigned long int last;
  unsigned long int item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsULInt_qsortLt(lsULInt_cpy(lsULInt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsULInt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsULInt_take(&_super,last_index);
      if ((last_index = lsULInt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsULInt_cpyPt(lsULInt_nil(&_super),il_super);
      if (limit) lsULInt_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsULInt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsULInt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsULInt_elemFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func)
{
 if (!func) rs_error("lsULInt_elemFunc: cmp-function undefined.");
 return (lsULInt_getIndexFunc(il,item,func) >= 0);
}

int lsULInt_getIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsULInt_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsULInt_neqElemFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func)
{
  if (!func) rs_error("lsULInt_neqElemFunc: cmp-function undefined.");
  return (lsULInt_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsULInt_getLastNeqIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsULInt_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsULInt_getFstNeqIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsULInt_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsULInt_t * lsULInt_insert(lsULInt_t *il, int index, unsigned long int item, unsigned long int item0)
{
  int i;
  if (!il)
    il = lsULInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsULInt_insert: illegal index %d.",index);

    lsULInt_setIndex(il,index,item,item0);
    return (il);
  }
  lsULInt_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(unsigned long int));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsULInt_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsULInt_t * lsULInt_insertN(lsULInt_t *il, int index, int n, unsigned long int item, 
		    unsigned long int item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsULInt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsULInt_insert: illegal index %d.",index);

    lsULInt_setIndex(il,index,item,item0);
    lsULInt_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsULInt_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(unsigned long int));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsULInt_t * lsULInt_insSwap(lsULInt_t *il, int index, unsigned long int item, unsigned long int item0)
{
  int i,n;
  unsigned long int _item;

  if (!il)
    il = lsULInt_Nil();

  if (index < 0)
    rs_error("lsULInt_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsULInt_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsULInt_add(il,_item);
#else
  if (index < n)
    lsULInt_add(il,_item);
#endif

  return (il);
}

unsigned long int lsULInt_getFlip(int i, lsULInt_t *il, unsigned long int undef)
{
  unsigned long int item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsULInt_t * lsULInt_map(lsULInt_t * il, lsULInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsULInt_t * lsULInt_map_2(lsULInt_t * il, lsULInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsULInt_t * lsULInt_map_3(lsULInt_t * il, lsULInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsULInt_t * lsULInt_mapSet(lsULInt_t * il, lsULInt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsULInt_t * lsULInt_mapSet_2(lsULInt_t * il, lsULInt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsULInt_t * lsULInt_mapSet_3(lsULInt_t * il, lsULInt_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsULInt_t * lsULInt_CpyMap(lsULInt_t * il_from, lsULInt_map_t *func)
{
  return (lsULInt_cpyMap(NULL,il_from,func));
}

lsULInt_t * lsULInt_cpyMap(lsULInt_t * il_to, lsULInt_t * il_from, lsULInt_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsULInt_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsULInt_t *  lsULInt_CpyMap_2(lsULInt_t * il_from, lsULInt_map_2_t *func, void *arg)
{
  return (lsULInt_cpyMap_2(NULL, il_from, func, arg));
}

lsULInt_t *  lsULInt_CpyMap_3(lsULInt_t * il_from, lsULInt_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsULInt_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsULInt_t *  lsULInt_cpyMap_2(lsULInt_t * il_to, lsULInt_t * il_from, 
		      lsULInt_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsULInt_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsULInt_t *  lsULInt_cpyMap_3(lsULInt_t * il_to, lsULInt_t * il_from, 
		      lsULInt_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsULInt_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsULInt_t * lsULInt_CartProd(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func)
{
  return (lsULInt_cpyCartProd(NULL,il1,il2,func));
}

lsULInt_t * lsULInt_cpyCartProd(lsULInt_t *il_to, 
			lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsULInt_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsULInt_cpy(il_to,il1);
  if (!il_to)
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsULInt_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsULInt_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsULInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsULInt_t * lsULInt_cartProd(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func)
{
  int i,j;
  lsULInt_t tmp;
  if (LS_isNIL(il1)) return lsULInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsULInt_cpy(lsULInt_nil(&tmp),il1);
  lsULInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsULInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsULInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsULInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsULInt_t * lsULInt_cartProd_2(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsULInt_t tmp;
  if (LS_isNIL(il1)) return lsULInt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsULInt_cpy(lsULInt_nil(&tmp),il1);
  lsULInt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsULInt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsULInt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsULInt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsULInt_t * lsULInt_filter(lsULInt_t * il, lsULInt_filter_t *func)
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

lsULInt_t * lsULInt_CpyFilter(lsULInt_t *il_from, lsULInt_filter_t *func)
{
  return (lsULInt_cpyFilter(NULL,il_from,func));
}

lsULInt_t * lsULInt_cpyFilter(lsULInt_t *il_to, lsULInt_t *il_from, lsULInt_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsULInt_Nil();
  else
    lsULInt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsULInt_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

unsigned long int lsULInt_foldl(lsULInt_t *il, unsigned long int item0, lsULInt_fold_t *func)
{
  int i;
  unsigned long int result = item0;
  if (!func)
    rs_error("lsULInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

unsigned long int lsULInt_foldr(lsULInt_t *il, unsigned long int item0, lsULInt_fold_t *func)
{
  int i;
  unsigned long int result = item0;
  if (!func)
    rs_error("lsULInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

unsigned long int lsULInt_foldl_2(lsULInt_t *il, unsigned long int item0, lsULInt_fold_2_t *func,
		     void *arg)
{
  int i;
  unsigned long int result = item0;
  if (!func)
    rs_error("lsULInt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

unsigned long int lsULInt_foldr_2(lsULInt_t *il, unsigned long int item0, lsULInt_fold_2_t *func,
		     void *arg)
{
  int i;
  unsigned long int result = item0;
  if (!func)
    rs_error("lsULInt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsULInt_t * lsULInt_sscan_chr(lsULInt_t *il, char t, char *s)
{
  char *p;
  unsigned long int v;

  if (!il)
    il = lsULInt_Nil();
  else
    lsULInt_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsULInt_add(il,v);
    }
  }
  return (il);
}

char * lsULInt_sprint_chr(char *s, lsULInt_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    unsigned long int x = LS_GET(il,i);
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

char * lsULInt_sprintf_chr(char *s, lsULInt_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    unsigned long int x = LS_GET(il,i);
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
 
int lsULInt_fwrite(FILE *fp, lsULInt_t *il)
{
    unsigned long int *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(unsigned long int), m, fp)) != m) {
	    rs_error("lsULInt_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(unsigned long int), l, fp)) != l) {
	    rs_error("lsULInt_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsULInt_fread(lsULInt_t *il, int k, FILE *fp)
{
    unsigned long int *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsULInt_realloc(lsULInt_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(unsigned long int), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(unsigned long int), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsULInt_lsULInt_minmax(lsULInt_t *il, int mode, lsULInt_cmp_t *func)
{
  int i,iminmax;
  unsigned long int minmax;
  if (LS_isNIL(il)) 
    rs_error("lsULInt_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    unsigned long int tmp = LS_GET(il,i);
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
      rs_error("_lsULInt_lsULInt_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

unsigned long int lsULInt_maxFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  int i = _lsULInt_lsULInt_minmax(il,3,func);
  return (LS_GET(il,i));
}

unsigned long int lsULInt_minFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  int i = _lsULInt_lsULInt_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsULInt_maxIndexFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  return _lsULInt_lsULInt_minmax(il,3,func);
}

int lsULInt_minIndexFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  return _lsULInt_lsULInt_minmax(il,2,func);
}

lsULInt_t * lsULInt_sortByIndex(lsULInt_t *il, lsInt_t *index, unsigned long int undef)
{
  lsULInt_t *tmp = NULL;
  tmp = lsULInt_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsULInt_cpy(il,tmp);
      lsULInt_Free(tmp);
  }
  return (il);
}

lsULInt_t * lsULInt_cpySortByIndex(lsULInt_t *il_to, lsULInt_t *il_from, 
			   lsInt_t *index, unsigned long int undef)
{
  int i;
  lsULInt_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsULInt_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    unsigned long int item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsULInt_setIndex(il_to,i,item,undef);
    }
#else
    lsULInt_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsULInt_qsortX(unsigned long int *v, int left, int right, 
		int mode, lsULInt_cmp_t *func)
{
  unsigned long int x = v[left]; /* bestimme Trennelement */
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
      unsigned long int h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsULInt_qsortX(v,left,r,mode,func);
  if (l < right) _lsULInt_qsortX(v,l,right,mode,func);
}

lsULInt_t * lsULInt_qsortLtFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsULInt_t * lsULInt_qsortGtFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsULInt_qsortX_2(unsigned long int *v, int left, int right, 
		  int mode, lsULInt_cmp_2_t *func, void *arg)
{
  unsigned long int x = v[left]; /* bestimme Trennelement */
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
      unsigned long int h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsULInt_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsULInt_qsortX_2(v,l,right,mode,func,arg);
}

lsULInt_t * lsULInt_qsortLtFunc_2(lsULInt_t *il, lsULInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsULInt_t * lsULInt_qsortGtFunc_2(lsULInt_t *il, lsULInt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsULInt_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsULInt_t *il = tplFst(arg);
  lsULInt_cmp_t *func = (lsULInt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      unsigned long int x = LS_GET(il,i);
      unsigned long int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsULInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsULInt_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsULInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      unsigned long int x = LS_GET(il,i);
      unsigned long int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsULInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsULInt_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsULInt_t *il = tplFst(arg);
  lsULInt_cmp_t *func = (lsULInt_cmp_t *) tplSnd(arg);
  unsigned long int *value = (unsigned long int *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      unsigned long int x = *value;
      unsigned long int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsULInt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsULInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsULInt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  unsigned long int *value = (unsigned long int *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      unsigned long int x = *value;
      unsigned long int y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsULInt_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsULInt_qsortIndexLtFunc(lsInt_t *index, lsULInt_t *il, 
			      lsULInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsULInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsULInt_qsortIndexLt(lsInt_t *index, lsULInt_t *il)
{
  return (lsULInt_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsULInt_qsortIndexGtFunc(lsInt_t *index, lsULInt_t *il, 
			      lsULInt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsULInt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsULInt_qsortIndexGt(lsInt_t *index, lsULInt_t *il)
{
  return (lsULInt_qsortIndexGtFunc(index,il,NULL));
}

void _lsULInt_mergeX(unsigned long int *v, unsigned long int *w, int ll, int rl, int rr,
		int mode, lsULInt_cmp_t *func)
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
      rs_error("_lsULInt_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsULInt_msortX(unsigned long int *v, unsigned long int *w, int left, int right, 
		int mode, lsULInt_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsULInt_msortX(v,w,left,m,mode,func);
  _lsULInt_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsULInt_mergeX(w,v,left,m,right,mode,func);
}

lsULInt_t * lsULInt_msortLtFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  static lsULInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsULInt_realloc(_il,LS_N(il));
    _lsULInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsULInt_t * lsULInt_msortGtFunc(lsULInt_t *il, lsULInt_cmp_t *func)
{
  static lsULInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsULInt_realloc(_il,LS_N(il));
    _lsULInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

unsigned long int lsULInt_sum(lsULInt_t *il)
{
  unsigned long int sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

unsigned long int lsULInt_prod(lsULInt_t *il)
{
  unsigned long int sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsULInt_t * lsULInt_scale(lsULInt_t *il, unsigned long int s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsULInt_t * lsULInt_delta(lsULInt_t *il_to, lsULInt_t *il_from, unsigned long int base)
{
  int i;
  lsULInt_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsULInt_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

unsigned long int lsULInt_max(lsULInt_t *il)
{
  int i = _lsULInt_lsULInt_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

unsigned long int lsULInt_min(lsULInt_t *il)
{
  int i = _lsULInt_lsULInt_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsULInt_maxIndex(lsULInt_t *il)
{
  return _lsULInt_lsULInt_minmax(il,1,NULL);
}

int lsULInt_minIndex(lsULInt_t *il)
{
  return _lsULInt_lsULInt_minmax(il,0,NULL);
}

lsULInt_t *lsULInt_rmdup(lsULInt_t *il)
{
  int i,j;
  unsigned long int item;
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
      
    
lsULInt_t * lsULInt_qsortLt(lsULInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsULInt_t * lsULInt_qsortGt(lsULInt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsULInt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsULInt_t * lsULInt_msortLt(lsULInt_t *il)
{
  static lsULInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsULInt_realloc(_il,LS_N(il));
    _lsULInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsULInt_t * lsULInt_msortGt(lsULInt_t *il)
{
  static lsULInt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsULInt_realloc(_il,LS_N(il));
    _lsULInt_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsULInt_bsearchX(lsULInt_t *il, unsigned long int i,
		 int mode, lsULInt_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    unsigned long int x = LS_GET(il,m);
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
      rs_error("_lsULInt_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsULInt_bsearchLtFunc(lsULInt_t *il, unsigned long int i, lsULInt_cmp_t *func)
{
  return (_lsULInt_bsearchX(il,i,2,func));
}

int lsULInt_bsearchGtFunc(lsULInt_t *il, unsigned long int i, lsULInt_cmp_t *func)
{
  return (_lsULInt_bsearchX(il,i,3,func));
}

int _lsULInt_bsearchX_2(lsULInt_t *il, unsigned long int i,
		   int mode, lsULInt_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    unsigned long int x = LS_GET(il,m);
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
      rs_error("_lsULInt_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsULInt_bsearchLtFunc_2(lsULInt_t *il,unsigned long int i,lsULInt_cmp_2_t *func,void *arg)
{
  return (_lsULInt_bsearchX_2(il,i,2,func,arg));
}

int lsULInt_bsearchGtFunc_2(lsULInt_t *il,unsigned long int i,lsULInt_cmp_2_t *func,void *arg)
{
  return (_lsULInt_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsULInt_bsearchLt(lsULInt_t *il, unsigned long int i)
{
  return (_lsULInt_bsearchX(il,i,0,NULL));
}

int lsULInt_bsearchGt(lsULInt_t *il, unsigned long int i)
{
  return (_lsULInt_bsearchX(il,i,1,NULL));
}

#endif 

int lsULInt_cmpFunc(lsULInt_t *il1, lsULInt_t *il2, lsULInt_cmp_t *func)
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
      unsigned long int x = LS_GET(il1,i);
      unsigned long int y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsULInt_cmpFunc: no cmp-function defined");
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

int lsULInt_cmp(lsULInt_t *il1, lsULInt_t *il2)
{
  return (lsULInt_cmpFunc(il1,il2,NULL));
}

#endif
  

