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
#include "lsPt.h"
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

lsPt_t * lsPt_Nil(void)
{
  return (lsPt_nil(NULL));
}

lsPt_t * lsPt_realloc(lsPt_t *il, int n)
{
  if (!il) il = lsPt_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(void *), 
			  "list items");
  }
  return (il);
}

lsPt_t *  lsPt_nil(lsPt_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsPt_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsPt_t * lsPt_ConsNil(void * i)
{
  lsPt_t * il = rs_malloc(sizeof(lsPt_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(void *), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

void * lsPt_setIndex(lsPt_t *il, int index, void * i, void * i0)
{
  void * ret;
  int j;

  if (!il)
    rs_error("lsPt_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsPt_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(void *), 
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


lsPt_t * lsPt_setIndices(lsPt_t *il, lsInt_t *indices, void * x,
		       void * undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsPt_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsPt_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsPt_t * lsPt_setNil(lsPt_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsPt_t * lsPt_nsetIndex(lsPt_t *il, int index, int n, void * x, 
		      void * undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsPt_Nil();

  lsPt_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsPt_t * lsPt_setConsNil(lsPt_t * il, void * i)
{
  if (!il)
    return lsPt_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(void *), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsPt_getNewItemIndex(lsPt_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsPt_getNewItem(il);
  return (index);
}

void * *lsPt_getNewItem(lsPt_t *il)
{
  void * *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(void *), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsPt_t * lsPt_add(lsPt_t * il, void * i)
{
  if (!il)
    il = lsPt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(void *), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsPt_t * lsPt_Add(lsPt_t * il, void * i)
{
  lsPt_t *il_to = lsPt_Nil();
  if (!il)
    return (lsPt_setConsNil(il_to,i));

  lsPt_realloc(il_to, il->n_list+LIST_BUF);
  lsPt_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsPt_t * lsPt_Cons(lsPt_t * il, void * i)
{
  lsPt_t *il_to = lsPt_Nil();
  if (!il)
    return (lsPt_setConsNil(il_to,i));

  lsPt_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(void *)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsPt_t * lsPt_cons(lsPt_t *il, void * i)
{
  if (!il)
    il = lsPt_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(void *), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(void *));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

void * lsPt_last(lsPt_t *il, void * undef)
{
  return (LS_LAST_CHECK(il,undef));
}

void * lsPt_head(lsPt_t *il, void * undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

void * lsPt_popLast(lsPt_t *il, void * undef)
{
  void * x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsPt_init(il);
  return (x);
}

void * lsPt_popHead(lsPt_t *il, void * undef)
{
  void * x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsPt_tail(il);
  return (x);
}

lsPt_t * lsPt_init(lsPt_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPt_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsPt_t * lsPt_Init(lsPt_t * il)
{
  lsPt_t *il_to = lsPt_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsPt_Init: got empty list");
  }
  lsPt_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsPt_t * lsPt_tail(lsPt_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsPt_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(void *));
  return (il);
}
 
lsPt_t * lsPt_Tail(lsPt_t *il)
{
  lsPt_t *il_to = lsPt_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsPt_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsPt_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(void *));
  return (il_to);
}


lsPt_t * lsPt_take(lsPt_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsPt_t * lsPt_Take(lsPt_t *il, int n)
{
  lsPt_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsPt_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(void *));
  LS_N(il_to) = m;
  return (il_to);
}

void * lsPt_delSwap(lsPt_t *il, int i)
{
  void * x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

void * lsPt_delete(lsPt_t *il, int index, void * undef)
{
  void * x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(void *));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsPt_Free(lsPt_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsPt_free(lsPt_t * il)
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

void * lsPt_get(lsPt_t * il,int i)
{
  if (!il)
    rs_error("lsPt_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsPt_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

void * lsPt_getCheck(lsPt_t * il,int i,void * undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsPt_length(lsPt_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsPt_t *  lsPt_getRowPt(lsPt_t * row, lsPt_t * il, int i, int cols)
{
  row = lsPt_take(lsPt_dropPt(lsPt_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsPt_t *  lsPt_getRow(lsPt_t *row, lsPt_t *il, int i, int cols)
{
  lsPt_t rowPt;
  lsPt_getRowPt(lsPt_init(&rowPt), il, i, cols);

  lsPt_cpy(row, &rowPt);

  return (row);
}

lsPt_t *  lsPt_getCol(lsPt_t *col, lsPt_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  void * *item;

  if (n <= j)
    return (lsPt_setNil(col));

  col = lsPt_realloc(col, n / cols + 1);
  lsPt_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsPt_add(col,*item);

  return (col);
}

lsPt_t *  lsPt_setRow(lsPt_t *il, lsPt_t *row, int i, int cols)
{
  lsPt_t rowPt;
  int n;
  lsPt_getRowPt(lsPt_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsPt_cpy(&rowPt,lsPt_take(row,n));
  return (il);
}

lsPt_t *  lsPt_setCol(lsPt_t *il, lsPt_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  void * *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsPt_t *  lsPt_SetPt(int n, void * * items)
{
  return (lsPt_setPt(NULL,n,items));
}

lsPt_t *  lsPt_setPt(lsPt_t * il_to, int n, void * * items)
{
  if (!il_to)
    il_to = lsPt_Nil();
  lsPt_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsPt_t *  lsPt_CpyPt(lsPt_t * il_from)
{
  return (lsPt_cpyPt(NULL, il_from));
}

lsPt_t *  lsPt_cpyPt(lsPt_t * il_to, lsPt_t * il_from)
{
  if (!il_to)
    il_to = lsPt_Nil();
  lsPt_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsPt_t * lsPt_Cpy(const lsPt_t * il_from)
{
  return (lsPt_cpy(NULL,il_from));
}

lsPt_t * lsPt_cpy(lsPt_t * il_to, const lsPt_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (!il_from) return (il_to);

  lsPt_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(void *));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsPt_t * lsPt_Cat(lsPt_t * il_1, lsPt_t * il_2)
{
  return (lsPt_cat(lsPt_Cpy(il_1), il_2));
}

lsPt_t * lsPt_cat(lsPt_t * il_to, lsPt_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsPt_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsPt_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(void *));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsPt_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsPt_t * lsPt_addCat(lsPt_t *il_to, lsPt_t *il)
{
  return lsPt_add(il_to, lsPt_Cat(il_to,il));
}

lsPt_t * lsPt_AddCat(lsPt_t *il_to, lsPt_t *il)
{
  return lsPt_Add(il_to, lsPt_Cat(il_to,il));
}
#endif

lsPt_t * lsPt_drop(lsPt_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsPt_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(void *));
  return (il);
}

lsPt_t * lsPt_Drop(lsPt_t *il, int i)
{
  lsPt_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsPt_setPt(lsPt_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsPt_Cpy(&tmp);
  return (il_split);
}

lsPt_t * lsPt_dropPt(lsPt_t *il, int i)
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

lsPt_t * lsPt_split(lsPt_t *il, int i)
{
  lsPt_t *il_drop = lsPt_Drop(il,i);
  lsPt_take(il,i);
  return (il_drop);
}

lsPt_t * lsPt_nsplit(lsPt_t *il_split, lsPt_t *il, lsInt_t *is)
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
      lsPt_t *split = lsPt_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsPt_t *lsPt_range(lsPt_t *il, int i0, int iN)
{
  lsPt_take(il,iN);
  if (i0 > 0) {
    lsPt_drop(il,i0-1);
  }
  return (il);
}

lsPt_t *lsPt_Range(lsPt_t *il, int i0, int iN)
{
  int n;
  lsPt_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsPt_Drop(il,i0);
  return (lsPt_take(il_to,iN));
}

lsPt_t *lsPt_rangePt(lsPt_t *il, int i0, int iN)
{
  return (lsPt_dropPt(lsPt_take(il,iN),i0));
}

lsPt_t *lsPt_cpyRange(lsPt_t *il_to, lsPt_t *il, int i0, int iN)
{
  lsPt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPt_rangePt(lsPt_cpyPt(lsPt_nil(&tmp),il),i0,iN);
  return (lsPt_cpy(il_to,&tmp));
}

lsPt_t *lsPt_catRange(lsPt_t *il_to, lsPt_t *il, int i0, int iN)
{
  lsPt_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsPt_rangePt(lsPt_cpyPt(lsPt_nil(&tmp),il),i0,iN);
  return (lsPt_cat(il_to,&tmp));
}
  
lsPt_t *lsPt_reverse(lsPt_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    void * tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsPt_makeIndex(lsInt_t *index, lsPt_t *il)
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
lsPt_t * lsPt_join(lsPt_t *il_to, lsPt_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    void * x = LS_GET(il_from,i);
    if (lsPt_elem(il_to,x)) continue;

    lsPt_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsPt_t * lsPt_filterByValue(lsPt_t *il, void * undef, lsInt_t *indices)
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

lsPt_t * lsPt_CpyFilterByValue(lsPt_t *il, void * undef, lsInt_t *indices)
{
  return (lsPt_cpyFilterByValue(NULL,il,undef,indices));
}

lsPt_t * lsPt_cpyFilterByValue(lsPt_t *il_to, lsPt_t *il_from, void * undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsPt_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsPt_t * lsPt_filterByIndex(lsPt_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsPt_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsPt_t * lsPt_CpyFilterByIndex(lsPt_t *il, lsInt_t *indices)
{
  return (lsPt_cpyFilterByIndex(NULL,il,indices));
}

lsPt_t * lsPt_cpyFilterByIndex(lsPt_t *il_to, lsPt_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsPt_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsPt_t * lsPt_joinInts(lsPt_t *il_to, lsPt_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsPt_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsPt_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsPt_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsPt_elem(lsPt_t * il, void * item)
{
  return (lsPt_getLastIndex(il,item) >= 0);
}

int lsPt_getLastIndex(lsPt_t *il, void * item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsPt_getFstIndex(lsPt_t *il, void * item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsPt_getIndex(lsPt_t * il, void * item)
{
  return (lsPt_getLastIndex(il,item));
}

int lsPt_neqElem(lsPt_t * il, void * item)
{
  return (lsPt_getLastNeqIndex(il,item) >= 0);
}

int lsPt_getLastNeqIndex(lsPt_t * il, void * item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsPt_getFstNeqIndex(lsPt_t * il, void * item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsPt_disjoint(lsPt_t *il1, lsPt_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsPt_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsPt_t *lsPt_subst(lsPt_t *il, void * i, void * j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsPt_subBag(lsPt_t *il_sub, lsPt_t *il_super, void * undef)
{
  return lsPt_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsPt_subBagIndices(lsInt_t *indices,
		     lsPt_t *il_sub, lsPt_t *il_super, void * undef)
{
  lsPt_t _sub;
  lsPt_t _super;
  int i;
  void * last;
  void * item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPt_qsortLt(lsPt_cpy(lsPt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPt_take(&_super,last_index);
      if ((last_index = lsPt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPt_cpyPt(lsPt_nil(&_super),il_super);

      if ((last_index = lsPt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsPt_subBagLimitedIndices(lsInt_t *indices,
			    lsPt_t *il_sub, lsPt_t *il_super, 
			    lsInt_t *limit, void * undef)
{
  lsPt_t _sub;
  lsPt_t _super;
  int i;
  void * last;
  void * item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsPt_qsortLt(lsPt_cpy(lsPt_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsPt_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsPt_take(&_super,last_index);
      if ((last_index = lsPt_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsPt_cpyPt(lsPt_nil(&_super),il_super);
      if (limit) lsPt_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsPt_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsPt_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsPt_elemFunc(lsPt_t * il, void * item, lsPt_cmp_t *func)
{
 if (!func) rs_error("lsPt_elemFunc: cmp-function undefined.");
 return (lsPt_getIndexFunc(il,item,func) >= 0);
}

int lsPt_getIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPt_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsPt_neqElemFunc(lsPt_t * il, void * item, lsPt_cmp_t *func)
{
  if (!func) rs_error("lsPt_neqElemFunc: cmp-function undefined.");
  return (lsPt_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsPt_getLastNeqIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPt_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsPt_getFstNeqIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsPt_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsPt_t * lsPt_insert(lsPt_t *il, int index, void * item, void * item0)
{
  int i;
  if (!il)
    il = lsPt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPt_insert: illegal index %d.",index);

    lsPt_setIndex(il,index,item,item0);
    return (il);
  }
  lsPt_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(void *));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsPt_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsPt_t * lsPt_insertN(lsPt_t *il, int index, int n, void * item, 
		    void * item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsPt_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsPt_insert: illegal index %d.",index);

    lsPt_setIndex(il,index,item,item0);
    lsPt_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsPt_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(void *));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsPt_t * lsPt_insSwap(lsPt_t *il, int index, void * item, void * item0)
{
  int i,n;
  void * _item;

  if (!il)
    il = lsPt_Nil();

  if (index < 0)
    rs_error("lsPt_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsPt_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsPt_add(il,_item);
#else
  if (index < n)
    lsPt_add(il,_item);
#endif

  return (il);
}

void * lsPt_getFlip(int i, lsPt_t *il, void * undef)
{
  void * item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsPt_t * lsPt_map(lsPt_t * il, lsPt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPt_t * lsPt_map_2(lsPt_t * il, lsPt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPt_t * lsPt_map_3(lsPt_t * il, lsPt_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPt_t * lsPt_mapSet(lsPt_t * il, lsPt_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsPt_t * lsPt_mapSet_2(lsPt_t * il, lsPt_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsPt_t * lsPt_mapSet_3(lsPt_t * il, lsPt_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsPt_t * lsPt_CpyMap(lsPt_t * il_from, lsPt_map_t *func)
{
  return (lsPt_cpyMap(NULL,il_from,func));
}

lsPt_t * lsPt_cpyMap(lsPt_t * il_to, lsPt_t * il_from, lsPt_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPt_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsPt_t *  lsPt_CpyMap_2(lsPt_t * il_from, lsPt_map_2_t *func, void *arg)
{
  return (lsPt_cpyMap_2(NULL, il_from, func, arg));
}

lsPt_t *  lsPt_CpyMap_3(lsPt_t * il_from, lsPt_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsPt_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsPt_t *  lsPt_cpyMap_2(lsPt_t * il_to, lsPt_t * il_from, 
		      lsPt_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPt_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsPt_t *  lsPt_cpyMap_3(lsPt_t * il_to, lsPt_t * il_from, 
		      lsPt_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsPt_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsPt_t * lsPt_CartProd(lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func)
{
  return (lsPt_cpyCartProd(NULL,il1,il2,func));
}

lsPt_t * lsPt_cpyCartProd(lsPt_t *il_to, 
			lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsPt_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsPt_cpy(il_to,il1);
  if (!il_to)
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPt_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPt_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsPt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsPt_t * lsPt_cartProd(lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func)
{
  int i,j;
  lsPt_t tmp;
  if (LS_isNIL(il1)) return lsPt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPt_cpy(lsPt_nil(&tmp),il1);
  lsPt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsPt_t * lsPt_cartProd_2(lsPt_t *il1, lsPt_t *il2, lsPt_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsPt_t tmp;
  if (LS_isNIL(il1)) return lsPt_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsPt_cpy(lsPt_nil(&tmp),il1);
  lsPt_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsPt_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsPt_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsPt_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsPt_t * lsPt_filter(lsPt_t * il, lsPt_filter_t *func)
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

lsPt_t * lsPt_CpyFilter(lsPt_t *il_from, lsPt_filter_t *func)
{
  return (lsPt_cpyFilter(NULL,il_from,func));
}

lsPt_t * lsPt_cpyFilter(lsPt_t *il_to, lsPt_t *il_from, lsPt_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsPt_Nil();
  else
    lsPt_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsPt_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

void * lsPt_foldl(lsPt_t *il, void * item0, lsPt_fold_t *func)
{
  int i;
  void * result = item0;
  if (!func)
    rs_error("lsPt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

void * lsPt_foldr(lsPt_t *il, void * item0, lsPt_fold_t *func)
{
  int i;
  void * result = item0;
  if (!func)
    rs_error("lsPt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

void * lsPt_foldl_2(lsPt_t *il, void * item0, lsPt_fold_2_t *func,
		     void *arg)
{
  int i;
  void * result = item0;
  if (!func)
    rs_error("lsPt_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

void * lsPt_foldr_2(lsPt_t *il, void * item0, lsPt_fold_2_t *func,
		     void *arg)
{
  int i;
  void * result = item0;
  if (!func)
    rs_error("lsPt_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsPt_t * lsPt_sscan_chr(lsPt_t *il, char t, char *s)
{
  char *p;
  void * v;

  if (!il)
    il = lsPt_Nil();
  else
    lsPt_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsPt_add(il,v);
    }
  }
  return (il);
}

char * lsPt_sprint_chr(char *s, lsPt_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    void * x = LS_GET(il,i);
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

char * lsPt_sprintf_chr(char *s, lsPt_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    void * x = LS_GET(il,i);
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
 
int lsPt_fwrite(FILE *fp, lsPt_t *il)
{
    void * *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(void *), m, fp)) != m) {
	    rs_error("lsPt_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(void *), l, fp)) != l) {
	    rs_error("lsPt_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsPt_fread(lsPt_t *il, int k, FILE *fp)
{
    void * *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsPt_realloc(lsPt_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(void *), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(void *), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsPt_lsPt_minmax(lsPt_t *il, int mode, lsPt_cmp_t *func)
{
  int i,iminmax;
  void * minmax;
  if (LS_isNIL(il)) 
    rs_error("lsPt_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    void * tmp = LS_GET(il,i);
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
      rs_error("_lsPt_lsPt_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

void * lsPt_maxFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  int i = _lsPt_lsPt_minmax(il,3,func);
  return (LS_GET(il,i));
}

void * lsPt_minFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  int i = _lsPt_lsPt_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsPt_maxIndexFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  return _lsPt_lsPt_minmax(il,3,func);
}

int lsPt_minIndexFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  return _lsPt_lsPt_minmax(il,2,func);
}

lsPt_t * lsPt_sortByIndex(lsPt_t *il, lsInt_t *index, void * undef)
{
  lsPt_t *tmp = NULL;
  tmp = lsPt_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsPt_cpy(il,tmp);
      lsPt_Free(tmp);
  }
  return (il);
}

lsPt_t * lsPt_cpySortByIndex(lsPt_t *il_to, lsPt_t *il_from, 
			   lsInt_t *index, void * undef)
{
  int i;
  lsPt_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsPt_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    void * item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsPt_setIndex(il_to,i,item,undef);
    }
#else
    lsPt_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsPt_qsortX(void * *v, int left, int right, 
		int mode, lsPt_cmp_t *func)
{
  void * x = v[left]; /* bestimme Trennelement */
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
      void * h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPt_qsortX(v,left,r,mode,func);
  if (l < right) _lsPt_qsortX(v,l,right,mode,func);
}

lsPt_t * lsPt_qsortLtFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsPt_t * lsPt_qsortGtFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsPt_qsortX_2(void * *v, int left, int right, 
		  int mode, lsPt_cmp_2_t *func, void *arg)
{
  void * x = v[left]; /* bestimme Trennelement */
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
      void * h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsPt_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsPt_qsortX_2(v,l,right,mode,func,arg);
}

lsPt_t * lsPt_qsortLtFunc_2(lsPt_t *il, lsPt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsPt_t * lsPt_qsortGtFunc_2(lsPt_t *il, lsPt_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsPt_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsPt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      void * x = LS_GET(il,i);
      void * y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPt_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsPt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      void * x = LS_GET(il,i);
      void * y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPt_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsPt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  void * *value = (void * *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      void * x = *value;
      void * y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPt_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsPt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsPt_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  void * *value = (void * *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      void * x = *value;
      void * y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsPt_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsPt_qsortIndexLtFunc(lsInt_t *index, lsPt_t *il, 
			      lsPt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPt_qsortIndexLt(lsInt_t *index, lsPt_t *il)
{
  return (lsPt_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsPt_qsortIndexGtFunc(lsInt_t *index, lsPt_t *il, 
			      lsPt_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsPt_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsPt_qsortIndexGt(lsInt_t *index, lsPt_t *il)
{
  return (lsPt_qsortIndexGtFunc(index,il,NULL));
}

void _lsPt_mergeX(void * *v, void * *w, int ll, int rl, int rr,
		int mode, lsPt_cmp_t *func)
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
      rs_error("_lsPt_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsPt_msortX(void * *v, void * *w, int left, int right, 
		int mode, lsPt_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsPt_msortX(v,w,left,m,mode,func);
  _lsPt_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsPt_mergeX(w,v,left,m,right,mode,func);
}

lsPt_t * lsPt_msortLtFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  static lsPt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPt_realloc(_il,LS_N(il));
    _lsPt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsPt_t * lsPt_msortGtFunc(lsPt_t *il, lsPt_cmp_t *func)
{
  static lsPt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPt_realloc(_il,LS_N(il));
    _lsPt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

void * lsPt_sum(lsPt_t *il)
{
  void * sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

void * lsPt_prod(lsPt_t *il)
{
  void * sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsPt_t * lsPt_scale(lsPt_t *il, void * s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsPt_t * lsPt_delta(lsPt_t *il_to, lsPt_t *il_from, void * base)
{
  int i;
  lsPt_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsPt_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

void * lsPt_max(lsPt_t *il)
{
  int i = _lsPt_lsPt_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

void * lsPt_min(lsPt_t *il)
{
  int i = _lsPt_lsPt_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsPt_maxIndex(lsPt_t *il)
{
  return _lsPt_lsPt_minmax(il,1,NULL);
}

int lsPt_minIndex(lsPt_t *il)
{
  return _lsPt_lsPt_minmax(il,0,NULL);
}

lsPt_t *lsPt_rmdup(lsPt_t *il)
{
  int i,j;
  void * item;
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
      
    
lsPt_t * lsPt_qsortLt(lsPt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsPt_t * lsPt_qsortGt(lsPt_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsPt_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsPt_t * lsPt_msortLt(lsPt_t *il)
{
  static lsPt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPt_realloc(_il,LS_N(il));
    _lsPt_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsPt_t * lsPt_msortGt(lsPt_t *il)
{
  static lsPt_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsPt_realloc(_il,LS_N(il));
    _lsPt_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsPt_bsearchX(lsPt_t *il, void * i,
		 int mode, lsPt_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    void * x = LS_GET(il,m);
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
      rs_error("_lsPt_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPt_bsearchLtFunc(lsPt_t *il, void * i, lsPt_cmp_t *func)
{
  return (_lsPt_bsearchX(il,i,2,func));
}

int lsPt_bsearchGtFunc(lsPt_t *il, void * i, lsPt_cmp_t *func)
{
  return (_lsPt_bsearchX(il,i,3,func));
}

int _lsPt_bsearchX_2(lsPt_t *il, void * i,
		   int mode, lsPt_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    void * x = LS_GET(il,m);
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
      rs_error("_lsPt_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsPt_bsearchLtFunc_2(lsPt_t *il,void * i,lsPt_cmp_2_t *func,void *arg)
{
  return (_lsPt_bsearchX_2(il,i,2,func,arg));
}

int lsPt_bsearchGtFunc_2(lsPt_t *il,void * i,lsPt_cmp_2_t *func,void *arg)
{
  return (_lsPt_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsPt_bsearchLt(lsPt_t *il, void * i)
{
  return (_lsPt_bsearchX(il,i,0,NULL));
}

int lsPt_bsearchGt(lsPt_t *il, void * i)
{
  return (_lsPt_bsearchX(il,i,1,NULL));
}

#endif 

int lsPt_cmpFunc(lsPt_t *il1, lsPt_t *il2, lsPt_cmp_t *func)
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
      void * x = LS_GET(il1,i);
      void * y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsPt_cmpFunc: no cmp-function defined");
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

int lsPt_cmp(lsPt_t *il1, lsPt_t *il2)
{
  return (lsPt_cmpFunc(il1,il2,NULL));
}

#endif
  

