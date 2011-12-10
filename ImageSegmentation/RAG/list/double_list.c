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
#include "lsDouble.h"
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

lsDouble_t * lsDouble_Nil(void)
{
  return (lsDouble_nil(NULL));
}

lsDouble_t * lsDouble_realloc(lsDouble_t *il, int n)
{
  if (!il) il = lsDouble_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(double), 
			  "list items");
  }
  return (il);
}

lsDouble_t *  lsDouble_nil(lsDouble_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsDouble_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsDouble_t * lsDouble_ConsNil(double i)
{
  lsDouble_t * il = rs_malloc(sizeof(lsDouble_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(double), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

double lsDouble_setIndex(lsDouble_t *il, int index, double i, double i0)
{
  double ret;
  int j;

  if (!il)
    rs_error("lsDouble_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsDouble_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(double), 
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


lsDouble_t * lsDouble_setIndices(lsDouble_t *il, lsInt_t *indices, double x,
		       double undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsDouble_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsDouble_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsDouble_t * lsDouble_setNil(lsDouble_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsDouble_t * lsDouble_nsetIndex(lsDouble_t *il, int index, int n, double x, 
		      double undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsDouble_Nil();

  lsDouble_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsDouble_t * lsDouble_setConsNil(lsDouble_t * il, double i)
{
  if (!il)
    return lsDouble_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(double), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsDouble_getNewItemIndex(lsDouble_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsDouble_getNewItem(il);
  return (index);
}

double *lsDouble_getNewItem(lsDouble_t *il)
{
  double *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(double), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsDouble_t * lsDouble_add(lsDouble_t * il, double i)
{
  if (!il)
    il = lsDouble_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(double), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsDouble_t * lsDouble_Add(lsDouble_t * il, double i)
{
  lsDouble_t *il_to = lsDouble_Nil();
  if (!il)
    return (lsDouble_setConsNil(il_to,i));

  lsDouble_realloc(il_to, il->n_list+LIST_BUF);
  lsDouble_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsDouble_t * lsDouble_Cons(lsDouble_t * il, double i)
{
  lsDouble_t *il_to = lsDouble_Nil();
  if (!il)
    return (lsDouble_setConsNil(il_to,i));

  lsDouble_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(double)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsDouble_t * lsDouble_cons(lsDouble_t *il, double i)
{
  if (!il)
    il = lsDouble_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(double), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(double));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

double lsDouble_last(lsDouble_t *il, double undef)
{
  return (LS_LAST_CHECK(il,undef));
}

double lsDouble_head(lsDouble_t *il, double undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

double lsDouble_popLast(lsDouble_t *il, double undef)
{
  double x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsDouble_init(il);
  return (x);
}

double lsDouble_popHead(lsDouble_t *il, double undef)
{
  double x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsDouble_tail(il);
  return (x);
}

lsDouble_t * lsDouble_init(lsDouble_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsDouble_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsDouble_t * lsDouble_Init(lsDouble_t * il)
{
  lsDouble_t *il_to = lsDouble_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsDouble_Init: got empty list");
  }
  lsDouble_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsDouble_t * lsDouble_tail(lsDouble_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsDouble_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(double));
  return (il);
}
 
lsDouble_t * lsDouble_Tail(lsDouble_t *il)
{
  lsDouble_t *il_to = lsDouble_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsDouble_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsDouble_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(double));
  return (il_to);
}


lsDouble_t * lsDouble_take(lsDouble_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsDouble_t * lsDouble_Take(lsDouble_t *il, int n)
{
  lsDouble_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsDouble_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(double));
  LS_N(il_to) = m;
  return (il_to);
}

double lsDouble_delSwap(lsDouble_t *il, int i)
{
  double x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

double lsDouble_delete(lsDouble_t *il, int index, double undef)
{
  double x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(double));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsDouble_Free(lsDouble_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsDouble_free(lsDouble_t * il)
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

double lsDouble_get(lsDouble_t * il,int i)
{
  if (!il)
    rs_error("lsDouble_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsDouble_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

double lsDouble_getCheck(lsDouble_t * il,int i,double undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsDouble_length(lsDouble_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsDouble_t *  lsDouble_getRowPt(lsDouble_t * row, lsDouble_t * il, int i, int cols)
{
  row = lsDouble_take(lsDouble_dropPt(lsDouble_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsDouble_t *  lsDouble_getRow(lsDouble_t *row, lsDouble_t *il, int i, int cols)
{
  lsDouble_t rowPt;
  lsDouble_getRowPt(lsDouble_init(&rowPt), il, i, cols);

  lsDouble_cpy(row, &rowPt);

  return (row);
}

lsDouble_t *  lsDouble_getCol(lsDouble_t *col, lsDouble_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  double *item;

  if (n <= j)
    return (lsDouble_setNil(col));

  col = lsDouble_realloc(col, n / cols + 1);
  lsDouble_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsDouble_add(col,*item);

  return (col);
}

lsDouble_t *  lsDouble_setRow(lsDouble_t *il, lsDouble_t *row, int i, int cols)
{
  lsDouble_t rowPt;
  int n;
  lsDouble_getRowPt(lsDouble_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsDouble_cpy(&rowPt,lsDouble_take(row,n));
  return (il);
}

lsDouble_t *  lsDouble_setCol(lsDouble_t *il, lsDouble_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  double *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsDouble_t *  lsDouble_SetPt(int n, double * items)
{
  return (lsDouble_setPt(NULL,n,items));
}

lsDouble_t *  lsDouble_setPt(lsDouble_t * il_to, int n, double * items)
{
  if (!il_to)
    il_to = lsDouble_Nil();
  lsDouble_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsDouble_t *  lsDouble_CpyPt(lsDouble_t * il_from)
{
  return (lsDouble_cpyPt(NULL, il_from));
}

lsDouble_t *  lsDouble_cpyPt(lsDouble_t * il_to, lsDouble_t * il_from)
{
  if (!il_to)
    il_to = lsDouble_Nil();
  lsDouble_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsDouble_t * lsDouble_Cpy(const lsDouble_t * il_from)
{
  return (lsDouble_cpy(NULL,il_from));
}

lsDouble_t * lsDouble_cpy(lsDouble_t * il_to, const lsDouble_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (!il_from) return (il_to);

  lsDouble_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(double));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsDouble_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsDouble_t * lsDouble_Cat(lsDouble_t * il_1, lsDouble_t * il_2)
{
  return (lsDouble_cat(lsDouble_Cpy(il_1), il_2));
}

lsDouble_t * lsDouble_cat(lsDouble_t * il_to, lsDouble_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsDouble_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsDouble_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(double));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsDouble_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsDouble_t * lsDouble_addCat(lsDouble_t *il_to, lsDouble_t *il)
{
  return lsDouble_add(il_to, lsDouble_Cat(il_to,il));
}

lsDouble_t * lsDouble_AddCat(lsDouble_t *il_to, lsDouble_t *il)
{
  return lsDouble_Add(il_to, lsDouble_Cat(il_to,il));
}
#endif

lsDouble_t * lsDouble_drop(lsDouble_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsDouble_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(double));
  return (il);
}

lsDouble_t * lsDouble_Drop(lsDouble_t *il, int i)
{
  lsDouble_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsDouble_setPt(lsDouble_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsDouble_Cpy(&tmp);
  return (il_split);
}

lsDouble_t * lsDouble_dropPt(lsDouble_t *il, int i)
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

lsDouble_t * lsDouble_split(lsDouble_t *il, int i)
{
  lsDouble_t *il_drop = lsDouble_Drop(il,i);
  lsDouble_take(il,i);
  return (il_drop);
}

lsPt_t * lsDouble_nsplit(lsPt_t *il_split, lsDouble_t *il, lsInt_t *is)
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
      lsDouble_t *split = lsDouble_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsDouble_t *lsDouble_range(lsDouble_t *il, int i0, int iN)
{
  lsDouble_take(il,iN);
  if (i0 > 0) {
    lsDouble_drop(il,i0-1);
  }
  return (il);
}

lsDouble_t *lsDouble_Range(lsDouble_t *il, int i0, int iN)
{
  int n;
  lsDouble_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsDouble_Drop(il,i0);
  return (lsDouble_take(il_to,iN));
}

lsDouble_t *lsDouble_rangePt(lsDouble_t *il, int i0, int iN)
{
  return (lsDouble_dropPt(lsDouble_take(il,iN),i0));
}

lsDouble_t *lsDouble_cpyRange(lsDouble_t *il_to, lsDouble_t *il, int i0, int iN)
{
  lsDouble_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsDouble_rangePt(lsDouble_cpyPt(lsDouble_nil(&tmp),il),i0,iN);
  return (lsDouble_cpy(il_to,&tmp));
}

lsDouble_t *lsDouble_catRange(lsDouble_t *il_to, lsDouble_t *il, int i0, int iN)
{
  lsDouble_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsDouble_rangePt(lsDouble_cpyPt(lsDouble_nil(&tmp),il),i0,iN);
  return (lsDouble_cat(il_to,&tmp));
}
  
lsDouble_t *lsDouble_reverse(lsDouble_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    double tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsDouble_makeIndex(lsInt_t *index, lsDouble_t *il)
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
lsDouble_t * lsDouble_join(lsDouble_t *il_to, lsDouble_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsDouble_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    double x = LS_GET(il_from,i);
    if (lsDouble_elem(il_to,x)) continue;

    lsDouble_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsDouble_t * lsDouble_filterByValue(lsDouble_t *il, double undef, lsInt_t *indices)
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

lsDouble_t * lsDouble_CpyFilterByValue(lsDouble_t *il, double undef, lsInt_t *indices)
{
  return (lsDouble_cpyFilterByValue(NULL,il,undef,indices));
}

lsDouble_t * lsDouble_cpyFilterByValue(lsDouble_t *il_to, lsDouble_t *il_from, double undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsDouble_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsDouble_t * lsDouble_filterByIndex(lsDouble_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsDouble_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsDouble_t * lsDouble_CpyFilterByIndex(lsDouble_t *il, lsInt_t *indices)
{
  return (lsDouble_cpyFilterByIndex(NULL,il,indices));
}

lsDouble_t * lsDouble_cpyFilterByIndex(lsDouble_t *il_to, lsDouble_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsDouble_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsDouble_t * lsDouble_joinInts(lsDouble_t *il_to, lsDouble_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsDouble_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsDouble_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsDouble_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsDouble_elem(lsDouble_t * il, double item)
{
  return (lsDouble_getLastIndex(il,item) >= 0);
}

int lsDouble_getLastIndex(lsDouble_t *il, double item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsDouble_getFstIndex(lsDouble_t *il, double item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsDouble_getIndex(lsDouble_t * il, double item)
{
  return (lsDouble_getLastIndex(il,item));
}

int lsDouble_neqElem(lsDouble_t * il, double item)
{
  return (lsDouble_getLastNeqIndex(il,item) >= 0);
}

int lsDouble_getLastNeqIndex(lsDouble_t * il, double item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsDouble_getFstNeqIndex(lsDouble_t * il, double item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsDouble_disjoint(lsDouble_t *il1, lsDouble_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsDouble_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsDouble_t *lsDouble_subst(lsDouble_t *il, double i, double j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsDouble_subBag(lsDouble_t *il_sub, lsDouble_t *il_super, double undef)
{
  return lsDouble_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsDouble_subBagIndices(lsInt_t *indices,
		     lsDouble_t *il_sub, lsDouble_t *il_super, double undef)
{
  lsDouble_t _sub;
  lsDouble_t _super;
  int i;
  double last;
  double item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsDouble_qsortLt(lsDouble_cpy(lsDouble_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsDouble_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsDouble_take(&_super,last_index);
      if ((last_index = lsDouble_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsDouble_cpyPt(lsDouble_nil(&_super),il_super);

      if ((last_index = lsDouble_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsDouble_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsDouble_subBagLimitedIndices(lsInt_t *indices,
			    lsDouble_t *il_sub, lsDouble_t *il_super, 
			    lsInt_t *limit, double undef)
{
  lsDouble_t _sub;
  lsDouble_t _super;
  int i;
  double last;
  double item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsDouble_qsortLt(lsDouble_cpy(lsDouble_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsDouble_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsDouble_take(&_super,last_index);
      if ((last_index = lsDouble_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsDouble_cpyPt(lsDouble_nil(&_super),il_super);
      if (limit) lsDouble_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsDouble_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsDouble_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsDouble_elemFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func)
{
 if (!func) rs_error("lsDouble_elemFunc: cmp-function undefined.");
 return (lsDouble_getIndexFunc(il,item,func) >= 0);
}

int lsDouble_getIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsDouble_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsDouble_neqElemFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func)
{
  if (!func) rs_error("lsDouble_neqElemFunc: cmp-function undefined.");
  return (lsDouble_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsDouble_getLastNeqIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsDouble_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsDouble_getFstNeqIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsDouble_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsDouble_t * lsDouble_insert(lsDouble_t *il, int index, double item, double item0)
{
  int i;
  if (!il)
    il = lsDouble_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsDouble_insert: illegal index %d.",index);

    lsDouble_setIndex(il,index,item,item0);
    return (il);
  }
  lsDouble_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(double));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsDouble_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsDouble_t * lsDouble_insertN(lsDouble_t *il, int index, int n, double item, 
		    double item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsDouble_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsDouble_insert: illegal index %d.",index);

    lsDouble_setIndex(il,index,item,item0);
    lsDouble_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsDouble_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(double));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsDouble_t * lsDouble_insSwap(lsDouble_t *il, int index, double item, double item0)
{
  int i,n;
  double _item;

  if (!il)
    il = lsDouble_Nil();

  if (index < 0)
    rs_error("lsDouble_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsDouble_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsDouble_add(il,_item);
#else
  if (index < n)
    lsDouble_add(il,_item);
#endif

  return (il);
}

double lsDouble_getFlip(int i, lsDouble_t *il, double undef)
{
  double item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsDouble_t * lsDouble_map(lsDouble_t * il, lsDouble_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsDouble_t * lsDouble_map_2(lsDouble_t * il, lsDouble_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsDouble_t * lsDouble_map_3(lsDouble_t * il, lsDouble_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsDouble_t * lsDouble_mapSet(lsDouble_t * il, lsDouble_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsDouble_t * lsDouble_mapSet_2(lsDouble_t * il, lsDouble_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsDouble_t * lsDouble_mapSet_3(lsDouble_t * il, lsDouble_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsDouble_t * lsDouble_CpyMap(lsDouble_t * il_from, lsDouble_map_t *func)
{
  return (lsDouble_cpyMap(NULL,il_from,func));
}

lsDouble_t * lsDouble_cpyMap(lsDouble_t * il_to, lsDouble_t * il_from, lsDouble_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsDouble_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsDouble_t *  lsDouble_CpyMap_2(lsDouble_t * il_from, lsDouble_map_2_t *func, void *arg)
{
  return (lsDouble_cpyMap_2(NULL, il_from, func, arg));
}

lsDouble_t *  lsDouble_CpyMap_3(lsDouble_t * il_from, lsDouble_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsDouble_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsDouble_t *  lsDouble_cpyMap_2(lsDouble_t * il_to, lsDouble_t * il_from, 
		      lsDouble_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsDouble_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsDouble_t *  lsDouble_cpyMap_3(lsDouble_t * il_to, lsDouble_t * il_from, 
		      lsDouble_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsDouble_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsDouble_t * lsDouble_CartProd(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func)
{
  return (lsDouble_cpyCartProd(NULL,il1,il2,func));
}

lsDouble_t * lsDouble_cpyCartProd(lsDouble_t *il_to, 
			lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsDouble_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsDouble_cpy(il_to,il1);
  if (!il_to)
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsDouble_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsDouble_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsDouble_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsDouble_t * lsDouble_cartProd(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func)
{
  int i,j;
  lsDouble_t tmp;
  if (LS_isNIL(il1)) return lsDouble_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsDouble_cpy(lsDouble_nil(&tmp),il1);
  lsDouble_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsDouble_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsDouble_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsDouble_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsDouble_t * lsDouble_cartProd_2(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsDouble_t tmp;
  if (LS_isNIL(il1)) return lsDouble_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsDouble_cpy(lsDouble_nil(&tmp),il1);
  lsDouble_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsDouble_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsDouble_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsDouble_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsDouble_t * lsDouble_filter(lsDouble_t * il, lsDouble_filter_t *func)
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

lsDouble_t * lsDouble_CpyFilter(lsDouble_t *il_from, lsDouble_filter_t *func)
{
  return (lsDouble_cpyFilter(NULL,il_from,func));
}

lsDouble_t * lsDouble_cpyFilter(lsDouble_t *il_to, lsDouble_t *il_from, lsDouble_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsDouble_Nil();
  else
    lsDouble_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsDouble_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

double lsDouble_foldl(lsDouble_t *il, double item0, lsDouble_fold_t *func)
{
  int i;
  double result = item0;
  if (!func)
    rs_error("lsDouble_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

double lsDouble_foldr(lsDouble_t *il, double item0, lsDouble_fold_t *func)
{
  int i;
  double result = item0;
  if (!func)
    rs_error("lsDouble_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

double lsDouble_foldl_2(lsDouble_t *il, double item0, lsDouble_fold_2_t *func,
		     void *arg)
{
  int i;
  double result = item0;
  if (!func)
    rs_error("lsDouble_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

double lsDouble_foldr_2(lsDouble_t *il, double item0, lsDouble_fold_2_t *func,
		     void *arg)
{
  int i;
  double result = item0;
  if (!func)
    rs_error("lsDouble_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsDouble_t * lsDouble_sscan_chr(lsDouble_t *il, char t, char *s)
{
  char *p;
  double v;

  if (!il)
    il = lsDouble_Nil();
  else
    lsDouble_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsDouble_add(il,v);
    }
  }
  return (il);
}

char * lsDouble_sprint_chr(char *s, lsDouble_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    double x = LS_GET(il,i);
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

char * lsDouble_sprintf_chr(char *s, lsDouble_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    double x = LS_GET(il,i);
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
 
int lsDouble_fwrite(FILE *fp, lsDouble_t *il)
{
    double *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(double), m, fp)) != m) {
	    rs_error("lsDouble_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(double), l, fp)) != l) {
	    rs_error("lsDouble_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsDouble_fread(lsDouble_t *il, int k, FILE *fp)
{
    double *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsDouble_realloc(lsDouble_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(double), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(double), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsDouble_lsDouble_minmax(lsDouble_t *il, int mode, lsDouble_cmp_t *func)
{
  int i,iminmax;
  double minmax;
  if (LS_isNIL(il)) 
    rs_error("lsDouble_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    double tmp = LS_GET(il,i);
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
      rs_error("_lsDouble_lsDouble_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

double lsDouble_maxFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  int i = _lsDouble_lsDouble_minmax(il,3,func);
  return (LS_GET(il,i));
}

double lsDouble_minFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  int i = _lsDouble_lsDouble_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsDouble_maxIndexFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  return _lsDouble_lsDouble_minmax(il,3,func);
}

int lsDouble_minIndexFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  return _lsDouble_lsDouble_minmax(il,2,func);
}

lsDouble_t * lsDouble_sortByIndex(lsDouble_t *il, lsInt_t *index, double undef)
{
  lsDouble_t *tmp = NULL;
  tmp = lsDouble_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsDouble_cpy(il,tmp);
      lsDouble_Free(tmp);
  }
  return (il);
}

lsDouble_t * lsDouble_cpySortByIndex(lsDouble_t *il_to, lsDouble_t *il_from, 
			   lsInt_t *index, double undef)
{
  int i;
  lsDouble_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsDouble_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    double item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsDouble_setIndex(il_to,i,item,undef);
    }
#else
    lsDouble_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsDouble_qsortX(double *v, int left, int right, 
		int mode, lsDouble_cmp_t *func)
{
  double x = v[left]; /* bestimme Trennelement */
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
      double h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsDouble_qsortX(v,left,r,mode,func);
  if (l < right) _lsDouble_qsortX(v,l,right,mode,func);
}

lsDouble_t * lsDouble_qsortLtFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsDouble_t * lsDouble_qsortGtFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsDouble_qsortX_2(double *v, int left, int right, 
		  int mode, lsDouble_cmp_2_t *func, void *arg)
{
  double x = v[left]; /* bestimme Trennelement */
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
      double h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsDouble_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsDouble_qsortX_2(v,l,right,mode,func,arg);
}

lsDouble_t * lsDouble_qsortLtFunc_2(lsDouble_t *il, lsDouble_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsDouble_t * lsDouble_qsortGtFunc_2(lsDouble_t *il, lsDouble_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsDouble_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsDouble_t *il = tplFst(arg);
  lsDouble_cmp_t *func = (lsDouble_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      double x = LS_GET(il,i);
      double y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsDouble_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsDouble_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsDouble_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      double x = LS_GET(il,i);
      double y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsDouble_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsDouble_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsDouble_t *il = tplFst(arg);
  lsDouble_cmp_t *func = (lsDouble_cmp_t *) tplSnd(arg);
  double *value = (double *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      double x = *value;
      double y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsDouble_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsDouble_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsDouble_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  double *value = (double *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      double x = *value;
      double y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsDouble_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsDouble_qsortIndexLtFunc(lsInt_t *index, lsDouble_t *il, 
			      lsDouble_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsDouble_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsDouble_qsortIndexLt(lsInt_t *index, lsDouble_t *il)
{
  return (lsDouble_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsDouble_qsortIndexGtFunc(lsInt_t *index, lsDouble_t *il, 
			      lsDouble_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsDouble_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsDouble_qsortIndexGt(lsInt_t *index, lsDouble_t *il)
{
  return (lsDouble_qsortIndexGtFunc(index,il,NULL));
}

void _lsDouble_mergeX(double *v, double *w, int ll, int rl, int rr,
		int mode, lsDouble_cmp_t *func)
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
      rs_error("_lsDouble_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsDouble_msortX(double *v, double *w, int left, int right, 
		int mode, lsDouble_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsDouble_msortX(v,w,left,m,mode,func);
  _lsDouble_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsDouble_mergeX(w,v,left,m,right,mode,func);
}

lsDouble_t * lsDouble_msortLtFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  static lsDouble_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsDouble_realloc(_il,LS_N(il));
    _lsDouble_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsDouble_t * lsDouble_msortGtFunc(lsDouble_t *il, lsDouble_cmp_t *func)
{
  static lsDouble_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsDouble_realloc(_il,LS_N(il));
    _lsDouble_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

double lsDouble_sum(lsDouble_t *il)
{
  double sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

double lsDouble_prod(lsDouble_t *il)
{
  double sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsDouble_t * lsDouble_scale(lsDouble_t *il, double s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsDouble_t * lsDouble_delta(lsDouble_t *il_to, lsDouble_t *il_from, double base)
{
  int i;
  lsDouble_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsDouble_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

double lsDouble_max(lsDouble_t *il)
{
  int i = _lsDouble_lsDouble_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

double lsDouble_min(lsDouble_t *il)
{
  int i = _lsDouble_lsDouble_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsDouble_maxIndex(lsDouble_t *il)
{
  return _lsDouble_lsDouble_minmax(il,1,NULL);
}

int lsDouble_minIndex(lsDouble_t *il)
{
  return _lsDouble_lsDouble_minmax(il,0,NULL);
}

lsDouble_t *lsDouble_rmdup(lsDouble_t *il)
{
  int i,j;
  double item;
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
      
    
lsDouble_t * lsDouble_qsortLt(lsDouble_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsDouble_t * lsDouble_qsortGt(lsDouble_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsDouble_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsDouble_t * lsDouble_msortLt(lsDouble_t *il)
{
  static lsDouble_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsDouble_realloc(_il,LS_N(il));
    _lsDouble_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsDouble_t * lsDouble_msortGt(lsDouble_t *il)
{
  static lsDouble_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsDouble_realloc(_il,LS_N(il));
    _lsDouble_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsDouble_bsearchX(lsDouble_t *il, double i,
		 int mode, lsDouble_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    double x = LS_GET(il,m);
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
      rs_error("_lsDouble_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsDouble_bsearchLtFunc(lsDouble_t *il, double i, lsDouble_cmp_t *func)
{
  return (_lsDouble_bsearchX(il,i,2,func));
}

int lsDouble_bsearchGtFunc(lsDouble_t *il, double i, lsDouble_cmp_t *func)
{
  return (_lsDouble_bsearchX(il,i,3,func));
}

int _lsDouble_bsearchX_2(lsDouble_t *il, double i,
		   int mode, lsDouble_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    double x = LS_GET(il,m);
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
      rs_error("_lsDouble_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsDouble_bsearchLtFunc_2(lsDouble_t *il,double i,lsDouble_cmp_2_t *func,void *arg)
{
  return (_lsDouble_bsearchX_2(il,i,2,func,arg));
}

int lsDouble_bsearchGtFunc_2(lsDouble_t *il,double i,lsDouble_cmp_2_t *func,void *arg)
{
  return (_lsDouble_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsDouble_bsearchLt(lsDouble_t *il, double i)
{
  return (_lsDouble_bsearchX(il,i,0,NULL));
}

int lsDouble_bsearchGt(lsDouble_t *il, double i)
{
  return (_lsDouble_bsearchX(il,i,1,NULL));
}

#endif 

int lsDouble_cmpFunc(lsDouble_t *il1, lsDouble_t *il2, lsDouble_cmp_t *func)
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
      double x = LS_GET(il1,i);
      double y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsDouble_cmpFunc: no cmp-function defined");
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

int lsDouble_cmp(lsDouble_t *il1, lsDouble_t *il2)
{
  return (lsDouble_cmpFunc(il1,il2,NULL));
}

#endif
  

