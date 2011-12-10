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
#include "_list.h"
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

list_t * ls_Nil(void)
{
  return (ls_nil(NULL));
}

list_t * ls_realloc(list_t *il, int n)
{
  if (!il) il = ls_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(LIST_TYPE), 
			  "list items");
  }
  return (il);
}

list_t *  ls_nil(list_t *il)
{
  if (!il) il = rs_malloc(sizeof(list_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

list_t * ls_ConsNil(LIST_TYPE i)
{
  list_t * il = rs_malloc(sizeof(list_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(LIST_TYPE), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

LIST_TYPE ls_setIndex(list_t *il, int index, LIST_TYPE i, LIST_TYPE i0)
{
  LIST_TYPE ret;
  int j;

  if (!il)
    rs_error("ls_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("ls_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(LIST_TYPE), 
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


list_t * ls_setIndices(list_t *il, lsInt_t *indices, LIST_TYPE x,
		       LIST_TYPE undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = ls_Nil();

  LS_FORALL_ITEMS(indices,i)
    ls_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

list_t * ls_setNil(list_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

list_t * ls_nsetIndex(list_t *il, int index, int n, LIST_TYPE x, 
		      LIST_TYPE undef)
{
  if (n <= 0) return;
  if (!il)
    il = ls_Nil();

  ls_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

list_t * ls_setConsNil(list_t * il, LIST_TYPE i)
{
  if (!il)
    return ls_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(LIST_TYPE), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int ls_getNewItemIndex(list_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  ls_getNewItem(il);
  return (index);
}

LIST_TYPE *ls_getNewItem(list_t *il)
{
  LIST_TYPE *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(LIST_TYPE), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

list_t * ls_add(list_t * il, LIST_TYPE i)
{
  if (!il)
    il = ls_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(LIST_TYPE), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

list_t * ls_Add(list_t * il, LIST_TYPE i)
{
  list_t *il_to = ls_Nil();
  if (!il)
    return (ls_setConsNil(il_to,i));

  ls_realloc(il_to, il->n_list+LIST_BUF);
  ls_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
list_t * ls_Cons(list_t * il, LIST_TYPE i)
{
  list_t *il_to = ls_Nil();
  if (!il)
    return (ls_setConsNil(il_to,i));

  ls_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(LIST_TYPE)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

list_t * ls_cons(list_t *il, LIST_TYPE i)
{
  if (!il)
    il = ls_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(LIST_TYPE), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(LIST_TYPE));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

LIST_TYPE ls_last(list_t *il, LIST_TYPE undef)
{
  return (LS_LAST_CHECK(il,undef));
}

LIST_TYPE ls_head(list_t *il, LIST_TYPE undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

LIST_TYPE ls_popLast(list_t *il, LIST_TYPE undef)
{
  LIST_TYPE x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  ls_init(il);
  return (x);
}

LIST_TYPE ls_popHead(list_t *il, LIST_TYPE undef)
{
  LIST_TYPE x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  ls_tail(il);
  return (x);
}

list_t * ls_init(list_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("ls_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

list_t * ls_Init(list_t * il)
{
  list_t *il_to = ls_Nil();

  if (LS_isNIL(il)) {
    rs_error("ls_Init: got empty list");
  }
  ls_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

list_t * ls_tail(list_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("ls_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(LIST_TYPE));
  return (il);
}
 
list_t * ls_Tail(list_t *il)
{
  list_t *il_to = ls_Nil();
  if (LS_isNIL(il)) {
    rs_error("ls_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  ls_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(LIST_TYPE));
  return (il_to);
}


list_t * ls_take(list_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

list_t * ls_Take(list_t *il, int n)
{
  list_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = ls_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(LIST_TYPE));
  LS_N(il_to) = m;
  return (il_to);
}

LIST_TYPE ls_delSwap(list_t *il, int i)
{
  LIST_TYPE x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

LIST_TYPE ls_delete(list_t *il, int index, LIST_TYPE undef)
{
  LIST_TYPE x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(LIST_TYPE));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void ls_Free(list_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void ls_free(list_t * il)
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

LIST_TYPE ls_get(list_t * il,int i)
{
  if (!il)
    rs_error("ls_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("ls_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

LIST_TYPE ls_getCheck(list_t * il,int i,LIST_TYPE undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int ls_length(list_t * il)
{
  return (il) ? (il->n_list) : 0;
}

list_t *  ls_getRowPt(list_t * row, list_t * il, int i, int cols)
{
  row = ls_take(ls_dropPt(ls_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

list_t *  ls_getRow(list_t *row, list_t *il, int i, int cols)
{
  list_t rowPt;
  ls_getRowPt(ls_init(&rowPt), il, i, cols);

  ls_cpy(row, &rowPt);

  return (row);
}

list_t *  ls_getCol(list_t *col, list_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  LIST_TYPE *item;

  if (n <= j)
    return (ls_setNil(col));

  col = ls_realloc(col, n / cols + 1);
  ls_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    ls_add(col,*item);

  return (col);
}

list_t *  ls_setRow(list_t *il, list_t *row, int i, int cols)
{
  list_t rowPt;
  int n;
  ls_getRowPt(ls_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    ls_cpy(&rowPt,ls_take(row,n));
  return (il);
}

list_t *  ls_setCol(list_t *il, list_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  LIST_TYPE *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

list_t *  ls_SetPt(int n, LIST_TYPE * items)
{
  return (ls_setPt(NULL,n,items));
}

list_t *  ls_setPt(list_t * il_to, int n, LIST_TYPE * items)
{
  if (!il_to)
    il_to = ls_Nil();
  ls_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

list_t *  ls_CpyPt(list_t * il_from)
{
  return (ls_cpyPt(NULL, il_from));
}

list_t *  ls_cpyPt(list_t * il_to, list_t * il_from)
{
  if (!il_to)
    il_to = ls_Nil();
  ls_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

list_t * ls_Cpy(const list_t * il_from)
{
  return (ls_cpy(NULL,il_from));
}

list_t * ls_cpy(list_t * il_to, const list_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (!il_from) return (il_to);

  ls_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(LIST_TYPE));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  ls_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

list_t * ls_Cat(list_t * il_1, list_t * il_2)
{
  return (ls_cat(ls_Cpy(il_1), il_2));
}

list_t * ls_cat(list_t * il_to, list_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("ls_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  ls_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(LIST_TYPE));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  ls_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
list_t * ls_addCat(list_t *il_to, list_t *il)
{
  return ls_add(il_to, ls_Cat(il_to,il));
}

list_t * ls_AddCat(list_t *il_to, list_t *il)
{
  return ls_Add(il_to, ls_Cat(il_to,il));
}
#endif

list_t * ls_drop(list_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (ls_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(LIST_TYPE));
  return (il);
}

list_t * ls_Drop(list_t *il, int i)
{
  list_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  ls_setPt(ls_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = ls_Cpy(&tmp);
  return (il_split);
}

list_t * ls_dropPt(list_t *il, int i)
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

list_t * ls_split(list_t *il, int i)
{
  list_t *il_drop = ls_Drop(il,i);
  ls_take(il,i);
  return (il_drop);
}

lsPt_t * ls_nsplit(lsPt_t *il_split, list_t *il, lsInt_t *is)
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
      list_t *split = ls_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

list_t *ls_range(list_t *il, int i0, int iN)
{
  ls_take(il,iN);
  if (i0 > 0) {
    ls_drop(il,i0-1);
  }
  return (il);
}

list_t *ls_Range(list_t *il, int i0, int iN)
{
  int n;
  list_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = ls_Drop(il,i0);
  return (ls_take(il_to,iN));
}

list_t *ls_rangePt(list_t *il, int i0, int iN)
{
  return (ls_dropPt(ls_take(il,iN),i0));
}

list_t *ls_cpyRange(list_t *il_to, list_t *il, int i0, int iN)
{
  list_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  ls_rangePt(ls_cpyPt(ls_nil(&tmp),il),i0,iN);
  return (ls_cpy(il_to,&tmp));
}

list_t *ls_catRange(list_t *il_to, list_t *il, int i0, int iN)
{
  list_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  ls_rangePt(ls_cpyPt(ls_nil(&tmp),il),i0,iN);
  return (ls_cat(il_to,&tmp));
}
  
list_t *ls_reverse(list_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    LIST_TYPE tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *ls_makeIndex(lsInt_t *index, list_t *il)
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
list_t * ls_join(list_t *il_to, list_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = ls_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    LIST_TYPE x = LS_GET(il_from,i);
    if (ls_elem(il_to,x)) continue;

    ls_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
list_t * ls_filterByValue(list_t *il, LIST_TYPE undef, lsInt_t *indices)
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

list_t * ls_CpyFilterByValue(list_t *il, LIST_TYPE undef, lsInt_t *indices)
{
  return (ls_cpyFilterByValue(NULL,il,undef,indices));
}

list_t * ls_cpyFilterByValue(list_t *il_to, list_t *il_from, LIST_TYPE undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      ls_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

list_t * ls_filterByIndex(list_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (ls_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

list_t * ls_CpyFilterByIndex(list_t *il, lsInt_t *indices)
{
  return (ls_cpyFilterByIndex(NULL,il,indices));
}

list_t * ls_cpyFilterByIndex(list_t *il_to, list_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      ls_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

list_t * ls_joinInts(list_t *il_to, list_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (ls_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = ls_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    ls_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int ls_elem(list_t * il, LIST_TYPE item)
{
  return (ls_getLastIndex(il,item) >= 0);
}

int ls_getLastIndex(list_t *il, LIST_TYPE item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int ls_getFstIndex(list_t *il, LIST_TYPE item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int ls_getIndex(list_t * il, LIST_TYPE item)
{
  return (ls_getLastIndex(il,item));
}

int ls_neqElem(list_t * il, LIST_TYPE item)
{
  return (ls_getLastNeqIndex(il,item) >= 0);
}

int ls_getLastNeqIndex(list_t * il, LIST_TYPE item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int ls_getFstNeqIndex(list_t * il, LIST_TYPE item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int ls_disjoint(list_t *il1, list_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (ls_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

list_t *ls_subst(list_t *il, LIST_TYPE i, LIST_TYPE j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int ls_subBag(list_t *il_sub, list_t *il_super, LIST_TYPE undef)
{
  return ls_subBagIndices(NULL, il_sub, il_super, undef);
}

int ls_subBagIndices(lsInt_t *indices,
		     list_t *il_sub, list_t *il_super, LIST_TYPE undef)
{
  list_t _sub;
  list_t _super;
  int i;
  LIST_TYPE last;
  LIST_TYPE item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  ls_qsortLt(ls_cpy(ls_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = ls_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      ls_take(&_super,last_index);
      if ((last_index = ls_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      ls_cpyPt(ls_nil(&_super),il_super);

      if ((last_index = ls_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  ls_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int ls_subBagLimitedIndices(lsInt_t *indices,
			    list_t *il_sub, list_t *il_super, 
			    lsInt_t *limit, LIST_TYPE undef)
{
  list_t _sub;
  list_t _super;
  int i;
  LIST_TYPE last;
  LIST_TYPE item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  ls_qsortLt(ls_cpy(ls_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = ls_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      ls_take(&_super,last_index);
      if ((last_index = ls_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      ls_cpyPt(ls_nil(&_super),il_super);
      if (limit) ls_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = ls_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  ls_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int ls_elemFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func)
{
 if (!func) rs_error("ls_elemFunc: cmp-function undefined.");
 return (ls_getIndexFunc(il,item,func) >= 0);
}

int ls_getIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("ls_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int ls_neqElemFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func)
{
  if (!func) rs_error("ls_neqElemFunc: cmp-function undefined.");
  return (ls_getLastNeqIndexFunc(il,item,func) >= 0);
}

int ls_getLastNeqIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("ls_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int ls_getFstNeqIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("ls_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

list_t * ls_insert(list_t *il, int index, LIST_TYPE item, LIST_TYPE item0)
{
  int i;
  if (!il)
    il = ls_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("ls_insert: illegal index %d.",index);

    ls_setIndex(il,index,item,item0);
    return (il);
  }
  ls_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(LIST_TYPE));
  LS_N(il)++;
  /*
   *i = LS_N(il); ls_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

list_t * ls_insertN(list_t *il, int index, int n, LIST_TYPE item, 
		    LIST_TYPE item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = ls_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("ls_insert: illegal index %d.",index);

    ls_setIndex(il,index,item,item0);
    ls_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); ls_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(LIST_TYPE));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


list_t * ls_insSwap(list_t *il, int index, LIST_TYPE item, LIST_TYPE item0)
{
  int i,n;
  LIST_TYPE _item;

  if (!il)
    il = ls_Nil();

  if (index < 0)
    rs_error("ls_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = ls_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    ls_add(il,_item);
#else
  if (index < n)
    ls_add(il,_item);
#endif

  return (il);
}

LIST_TYPE ls_getFlip(int i, list_t *il, LIST_TYPE undef)
{
  LIST_TYPE item = LS_GET_CHECK(il,i,undef);
  return item;
}

list_t * ls_map(list_t * il, LIST_MAP_TYPE *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

list_t * ls_map_2(list_t * il, LIST_MAP_2_TYPE *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

list_t * ls_map_3(list_t * il, LIST_MAP_3_TYPE *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

list_t * ls_mapSet(list_t * il, LIST_MAP_TYPE *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

list_t * ls_mapSet_2(list_t * il, LIST_MAP_2_TYPE *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

list_t * ls_mapSet_3(list_t * il, LIST_MAP_3_TYPE *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

list_t * ls_CpyMap(list_t * il_from, LIST_MAP_TYPE *func)
{
  return (ls_cpyMap(NULL,il_from,func));
}

list_t * ls_cpyMap(list_t * il_to, list_t * il_from, LIST_MAP_TYPE *func)
{
  int i;

  if (!il_to) 
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    ls_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

list_t *  ls_CpyMap_2(list_t * il_from, LIST_MAP_2_TYPE *func, void *arg)
{
  return (ls_cpyMap_2(NULL, il_from, func, arg));
}

list_t *  ls_CpyMap_3(list_t * il_from, LIST_MAP_3_TYPE *func, 
		      void *arg1, void *arg2)
{
  return (ls_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

list_t *  ls_cpyMap_2(list_t * il_to, list_t * il_from, 
		      LIST_MAP_2_TYPE *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    ls_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

list_t *  ls_cpyMap_3(list_t * il_to, list_t * il_from, 
		      LIST_MAP_3_TYPE *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    ls_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

list_t * ls_CartProd(list_t *il1, list_t *il2, LIST_FOLD_TYPE *func)
{
  return (ls_cpyCartProd(NULL,il1,il2,func));
}

list_t * ls_cpyCartProd(list_t *il_to, 
			list_t *il1, list_t *il2, LIST_FOLD_TYPE *func)
{
  int i,j;
  if (LS_isNIL(il1)) return ls_cpy(il_to,il2);
  if (LS_isNIL(il2)) return ls_cpy(il_to,il1);
  if (!il_to)
    il_to = ls_Nil();
  else
    ls_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) ls_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	ls_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("ls_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

list_t * ls_cartProd(list_t *il1, list_t *il2, LIST_FOLD_TYPE *func)
{
  int i,j;
  list_t tmp;
  if (LS_isNIL(il1)) return ls_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  ls_cpy(ls_nil(&tmp),il1);
  ls_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) ls_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	ls_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("ls_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

list_t * ls_cartProd_2(list_t *il1, list_t *il2, LIST_FOLD_2_TYPE *func,
		       void *arg)
{
  int i,j;
  list_t tmp;
  if (LS_isNIL(il1)) return ls_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  ls_cpy(ls_nil(&tmp),il1);
  ls_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) ls_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	ls_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("ls_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
list_t * ls_filter(list_t * il, LIST_FILTER_TYPE *func)
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

list_t * ls_CpyFilter(list_t *il_from, LIST_FILTER_TYPE *func)
{
  return (ls_cpyFilter(NULL,il_from,func));
}

list_t * ls_cpyFilter(list_t *il_to, list_t *il_from, LIST_FILTER_TYPE *func)
{
  int i,j;
  
  if (!il_to)
    il_to = ls_Nil();
  else
    ls_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      ls_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

LIST_TYPE ls_foldl(list_t *il, LIST_TYPE item0, LIST_FOLD_TYPE *func)
{
  int i;
  LIST_TYPE result = item0;
  if (!func)
    rs_error("ls_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

LIST_TYPE ls_foldr(list_t *il, LIST_TYPE item0, LIST_FOLD_TYPE *func)
{
  int i;
  LIST_TYPE result = item0;
  if (!func)
    rs_error("ls_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

LIST_TYPE ls_foldl_2(list_t *il, LIST_TYPE item0, LIST_FOLD_2_TYPE *func,
		     void *arg)
{
  int i;
  LIST_TYPE result = item0;
  if (!func)
    rs_error("ls_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

LIST_TYPE ls_foldr_2(list_t *il, LIST_TYPE item0, LIST_FOLD_2_TYPE *func,
		     void *arg)
{
  int i;
  LIST_TYPE result = item0;
  if (!func)
    rs_error("ls_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

list_t * ls_sscan_chr(list_t *il, char t, char *s)
{
  char *p;
  LIST_TYPE v;

  if (!il)
    il = ls_Nil();
  else
    ls_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      ls_add(il,v);
    }
  }
  return (il);
}

char * ls_sprint_chr(char *s, list_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    LIST_TYPE x = LS_GET(il,i);
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

char * ls_sprintf_chr(char *s, list_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    LIST_TYPE x = LS_GET(il,i);
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
 
int ls_fwrite(FILE *fp, list_t *il)
{
    LIST_TYPE *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(LIST_TYPE), m, fp)) != m) {
	    rs_error("ls_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(LIST_TYPE), l, fp)) != l) {
	    rs_error("ls_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int ls_fread(list_t *il, int k, FILE *fp)
{
    LIST_TYPE *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    ls_realloc(ls_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(LIST_TYPE), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(LIST_TYPE), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _ls_minmax(list_t *il, int mode, LIST_CMP_TYPE *func)
{
  int i,iminmax;
  LIST_TYPE minmax;
  if (LS_isNIL(il)) 
    rs_error("ls_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    LIST_TYPE tmp = LS_GET(il,i);
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
      rs_error("_ls_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

LIST_TYPE ls_maxFunc(list_t *il, LIST_CMP_TYPE *func)
{
  int i = _ls_minmax(il,3,func);
  return (LS_GET(il,i));
}

LIST_TYPE ls_minFunc(list_t *il, LIST_CMP_TYPE *func)
{
  int i = _ls_minmax(il,2,func);
  return (LS_GET(il,i));
}

int ls_maxIndexFunc(list_t *il, LIST_CMP_TYPE *func)
{
  return _ls_minmax(il,3,func);
}

int ls_minIndexFunc(list_t *il, LIST_CMP_TYPE *func)
{
  return _ls_minmax(il,2,func);
}

list_t * ls_sortByIndex(list_t *il, lsInt_t *index, LIST_TYPE undef)
{
  list_t *tmp = NULL;
  tmp = ls_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      ls_cpy(il,tmp);
      ls_Free(tmp);
  }
  return (il);
}

list_t * ls_cpySortByIndex(list_t *il_to, list_t *il_from, 
			   lsInt_t *index, LIST_TYPE undef)
{
  int i;
  ls_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = ls_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    LIST_TYPE item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	ls_setIndex(il_to,i,item,undef);
    }
#else
    ls_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _ls_qsortX(LIST_TYPE *v, int left, int right, 
		int mode, LIST_CMP_TYPE *func)
{
  LIST_TYPE x = v[left]; /* bestimme Trennelement */
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
      LIST_TYPE h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _ls_qsortX(v,left,r,mode,func);
  if (l < right) _ls_qsortX(v,l,right,mode,func);
}

list_t * ls_qsortLtFunc(list_t *il, LIST_CMP_TYPE *func)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

list_t * ls_qsortGtFunc(list_t *il, LIST_CMP_TYPE *func)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _ls_qsortX_2(LIST_TYPE *v, int left, int right, 
		  int mode, LIST_CMP_2_TYPE *func, void *arg)
{
  LIST_TYPE x = v[left]; /* bestimme Trennelement */
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
      LIST_TYPE h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _ls_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _ls_qsortX_2(v,l,right,mode,func,arg);
}

list_t * ls_qsortLtFunc_2(list_t *il, LIST_CMP_2_TYPE *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

list_t * ls_qsortGtFunc_2(list_t *il, LIST_CMP_2_TYPE *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int ls_cmpIndex(int i, int j, pairPt_t *arg)
{
  list_t *il = tplFst(arg);
  LIST_CMP_TYPE *func = (LIST_CMP_TYPE *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      LIST_TYPE x = LS_GET(il,i);
      LIST_TYPE y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("ls_cmpIndex: no cmp-function defined");
#endif
  }
}

int ls_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  list_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      LIST_TYPE x = LS_GET(il,i);
      LIST_TYPE y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("ls_cmpIndex: no cmp-function defined");
#endif
  }
}

int ls_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  list_t *il = tplFst(arg);
  LIST_CMP_TYPE *func = (LIST_CMP_TYPE *) tplSnd(arg);
  LIST_TYPE *value = (LIST_TYPE *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      LIST_TYPE x = *value;
      LIST_TYPE y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("ls_cmpIndex: no cmp-function defined");
#endif
  }
}

int ls_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  list_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  LIST_TYPE *value = (LIST_TYPE *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      LIST_TYPE x = *value;
      LIST_TYPE y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("ls_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * ls_qsortIndexLtFunc(lsInt_t *index, list_t *il, 
			      LIST_CMP_TYPE *func)
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
		    (lsInt_cmp_2_t *) ls_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * ls_qsortIndexLt(lsInt_t *index, list_t *il)
{
  return (ls_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * ls_qsortIndexGtFunc(lsInt_t *index, list_t *il, 
			      LIST_CMP_TYPE *func)
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
		    (lsInt_cmp_2_t *) ls_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * ls_qsortIndexGt(lsInt_t *index, list_t *il)
{
  return (ls_qsortIndexGtFunc(index,il,NULL));
}

void _ls_mergeX(LIST_TYPE *v, LIST_TYPE *w, int ll, int rl, int rr,
		int mode, LIST_CMP_TYPE *func)
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
      rs_error("_ls_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _ls_msortX(LIST_TYPE *v, LIST_TYPE *w, int left, int right, 
		int mode, LIST_CMP_TYPE *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _ls_msortX(v,w,left,m,mode,func);
  _ls_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _ls_mergeX(w,v,left,m,right,mode,func);
}

list_t * ls_msortLtFunc(list_t *il, LIST_CMP_TYPE *func)
{
  static list_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = ls_realloc(_il,LS_N(il));
    _ls_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

list_t * ls_msortGtFunc(list_t *il, LIST_CMP_TYPE *func)
{
  static list_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = ls_realloc(_il,LS_N(il));
    _ls_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

LIST_TYPE ls_sum(list_t *il)
{
  LIST_TYPE sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

LIST_TYPE ls_prod(list_t *il)
{
  LIST_TYPE sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

list_t * ls_scale(list_t *il, LIST_TYPE s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

list_t * ls_delta(list_t *il_to, list_t *il_from, LIST_TYPE base)
{
  int i;
  ls_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = ls_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

LIST_TYPE ls_max(list_t *il)
{
  int i = _ls_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

LIST_TYPE ls_min(list_t *il)
{
  int i = _ls_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int ls_maxIndex(list_t *il)
{
  return _ls_minmax(il,1,NULL);
}

int ls_minIndex(list_t *il)
{
  return _ls_minmax(il,0,NULL);
}

list_t *ls_rmdup(list_t *il)
{
  int i,j;
  LIST_TYPE item;
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
      
    
list_t * ls_qsortLt(list_t *il)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

list_t * ls_qsortGt(list_t *il)
{
  if (il && LS_N(il) > 0) {
    _ls_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

list_t * ls_msortLt(list_t *il)
{
  static list_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = ls_realloc(_il,LS_N(il));
    _ls_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

list_t * ls_msortGt(list_t *il)
{
  static list_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = ls_realloc(_il,LS_N(il));
    _ls_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _ls_bsearchX(list_t *il, LIST_TYPE i,
		 int mode, LIST_CMP_TYPE *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    LIST_TYPE x = LS_GET(il,m);
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
      rs_error("_ls_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int ls_bsearchLtFunc(list_t *il, LIST_TYPE i, LIST_CMP_TYPE *func)
{
  return (_ls_bsearchX(il,i,2,func));
}

int ls_bsearchGtFunc(list_t *il, LIST_TYPE i, LIST_CMP_TYPE *func)
{
  return (_ls_bsearchX(il,i,3,func));
}

int _ls_bsearchX_2(list_t *il, LIST_TYPE i,
		   int mode, LIST_CMP_2_TYPE *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    LIST_TYPE x = LS_GET(il,m);
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
      rs_error("_ls_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int ls_bsearchLtFunc_2(list_t *il,LIST_TYPE i,LIST_CMP_2_TYPE *func,void *arg)
{
  return (_ls_bsearchX_2(il,i,2,func,arg));
}

int ls_bsearchGtFunc_2(list_t *il,LIST_TYPE i,LIST_CMP_2_TYPE *func,void *arg)
{
  return (_ls_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int ls_bsearchLt(list_t *il, LIST_TYPE i)
{
  return (_ls_bsearchX(il,i,0,NULL));
}

int ls_bsearchGt(list_t *il, LIST_TYPE i)
{
  return (_ls_bsearchX(il,i,1,NULL));
}

#endif 

int ls_cmpFunc(list_t *il1, list_t *il2, LIST_CMP_TYPE *func)
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
      LIST_TYPE x = LS_GET(il1,i);
      LIST_TYPE y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("ls_cmpFunc: no cmp-function defined");
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

int ls_cmp(list_t *il1, list_t *il2)
{
  return (ls_cmpFunc(il1,il2,NULL));
}

#endif
  

