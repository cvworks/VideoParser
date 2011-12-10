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
#include "lsChar.h"
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

lsChar_t * lsChar_Nil(void)
{
  return (lsChar_nil(NULL));
}

lsChar_t * lsChar_realloc(lsChar_t *il, int n)
{
  if (!il) il = lsChar_nil(NULL);
  if (n > il->max_list) {
    il->list = rs_realloc(il->list,
			  (il->max_list = n) * sizeof(char), 
			  "list items");
  }
  return (il);
}

lsChar_t *  lsChar_nil(lsChar_t *il)
{
  if (!il) il = rs_malloc(sizeof(lsChar_t), "list");
  il->list = NULL;
  il->n_list = 0;
  il->max_list = 0;

  return (il);
}

lsChar_t * lsChar_ConsNil(char i)
{
  lsChar_t * il = rs_malloc(sizeof(lsChar_t), "list");

  il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(char), "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

char lsChar_setIndex(lsChar_t *il, int index, char i, char i0)
{
  char ret;
  int j;

  if (!il)
    rs_error("lsChar_setIndex: got NULL-pointer list");
  if (index < 0)
    rs_error("lsChar_setIndex: illegal index %d",index);

  if (index >= il->max_list) {
    int d = (index - il->max_list) / LIST_BUF + 1;
    il->list = rs_realloc(il->list,
			  (il->max_list+= d*LIST_BUF) * sizeof(char), 
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


lsChar_t * lsChar_setIndices(lsChar_t *il, lsInt_t *indices, char x,
		       char undef)
{
  int i;
  if (LS_isNIL(indices)) return (il);
  if (!il)
    il = lsChar_Nil();

  LS_FORALL_ITEMS(indices,i)
    lsChar_setIndex(il,LS_GET(indices,i),x,undef);
  
  return (il);
}

lsChar_t * lsChar_setNil(lsChar_t * il)
{
  if (il) il->n_list = 0;

  return (il);
}

lsChar_t * lsChar_nsetIndex(lsChar_t *il, int index, int n, char x, 
		      char undef)
{
  if (n <= 0) return;
  if (!il)
    il = lsChar_Nil();

  lsChar_setIndex(il, index+=n, x, undef);
  while (--n > 0)
    LS_GET(il,--index) = x;
  
  return (il);
}

lsChar_t * lsChar_setConsNil(lsChar_t * il, char i)
{
  if (!il)
    return lsChar_ConsNil(i);

  if (!il->list)
    il->list = rs_malloc((il->max_list=LIST_BUF) * sizeof(char), 
			 "list item");
  il->list[0] = i;
  il->n_list = 1;
  return (il);
}

int lsChar_getNewItemIndex(lsChar_t *il)
{
  int index;
  if (!il) return (-1);
  index = LS_N(il);
  lsChar_getNewItem(il);
  return (index);
}

char *lsChar_getNewItem(lsChar_t *il)
{
  char *item;
  if (!il) return (NULL);
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(char), "list item");
  item = &il->list[il->n_list++];
  return (item);
}

lsChar_t * lsChar_add(lsChar_t * il, char i)
{
  if (!il)
    il = lsChar_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(char), "list item");
  il->list[il->n_list] = i;
  il->n_list++;
  return (il);
}

lsChar_t * lsChar_Add(lsChar_t * il, char i)
{
  lsChar_t *il_to = lsChar_Nil();
  if (!il)
    return (lsChar_setConsNil(il_to,i));

  lsChar_realloc(il_to, il->n_list+LIST_BUF);
  lsChar_cpy(il_to,il);
  il_to->list[il_to->n_list] = i;
  il_to->n_list++;
  return (il_to);
}
  
lsChar_t * lsChar_Cons(lsChar_t * il, char i)
{
  lsChar_t *il_to = lsChar_Nil();
  if (!il)
    return (lsChar_setConsNil(il_to,i));

  lsChar_realloc(il_to, il->n_list+LIST_BUF);
  memcpy(il_to->list+1,il->list,il->n_list * sizeof(char)); 
  il_to->list[0] = i;
  il_to->n_list = il->n_list + 1;
  return (il_to);
}

lsChar_t * lsChar_cons(lsChar_t *il, char i)
{
  if (!il)
    il = lsChar_Nil();
  if (il->n_list>=il->max_list)
    il->list = rs_realloc(il->list,(il->max_list+=LIST_BUF) * 
			  sizeof(char), "list item");
  memmove(il->list+1, il->list, il->n_list * sizeof(char));
  il->list[0] = i;
  il->n_list++;
  return (il);
}

char lsChar_last(lsChar_t *il, char undef)
{
  return (LS_LAST_CHECK(il,undef));
}

char lsChar_head(lsChar_t *il, char undef)
{
  return (LS_FIRST_CHECK(il,undef));
}

char lsChar_popLast(lsChar_t *il, char undef)
{
  char x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_LAST(il);
  lsChar_init(il);
  return (x);
}

char lsChar_popHead(lsChar_t *il, char undef)
{
  char x;
  if (LS_isNIL(il))
    return (undef);
  x = LS_HEAD(il);
  lsChar_tail(il);
  return (x);
}

lsChar_t * lsChar_init(lsChar_t * il)
{
  if (LS_isNIL(il)) {
    rs_error("lsChar_init: got empty list");
  }
  LS_N(il)--;
  return (il);
}

lsChar_t * lsChar_Init(lsChar_t * il)
{
  lsChar_t *il_to = lsChar_Nil();

  if (LS_isNIL(il)) {
    rs_error("lsChar_Init: got empty list");
  }
  lsChar_cpy(il_to,il);
  LS_N(il_to)--;
  return (il_to);
}

lsChar_t * lsChar_tail(lsChar_t *il)
{
  if (LS_isNIL(il)) {
    rs_error("lsChar_tail: got empty list");
  }
  if ((--LS_N(il)) == 0)
    return (il);
  memmove(il->list, il->list+1, LS_N(il) * sizeof(char));
  return (il);
}
 
lsChar_t * lsChar_Tail(lsChar_t *il)
{
  lsChar_t *il_to = lsChar_Nil();
  if (LS_isNIL(il)) {
    rs_error("lsChar_tail: got empty list");
  }
  if (LS_N(il) == 1)
    return (il_to);
  lsChar_realloc(il_to, LS_N(il)-1);
  memcpy(il_to->list, il->list+1, (LS_N(il)-1) * sizeof(char));
  return (il_to);
}


lsChar_t * lsChar_take(lsChar_t *il, int n)
{
  if (LS_N(il) < n) {
    return (il);
  }
  LS_N(il) = n;
  return (il);
}

lsChar_t * lsChar_Take(lsChar_t *il, int n)
{
  lsChar_t *il_to;
  int m;
  if (!il)
    return (NULL);

  m = (LS_N(il) < n) ? LS_N(il) : n;
  il_to = lsChar_realloc(NULL,m);
  memcpy(il_to->list,il->list,m * sizeof(char));
  LS_N(il_to) = m;
  return (il_to);
}

char lsChar_delSwap(lsChar_t *il, int i)
{
  char x;

  if (LS_isNIL(il))
    rs_error("ls_delIndex: got empty list");
  if (!LS_EXISTS(il,i))
    rs_error("ls_delIndex: illegal index %d",i);

  x = LS_GET(il,i);
  LS_GET(il,i) = LS_LAST(il);
  LS_N(il)--;
  return (x);
}

char lsChar_delete(lsChar_t *il, int index, char undef)
{
  char x;
  if (!LS_EXISTS(il,index))
    return (undef);
  x = LS_GET(il,index);

  LS_N(il)--;
  memmove(il->list+index,il->list+index+1,(LS_N(il)-index)*sizeof(char));
  /*
   *  while ((++index) < LS_N(il))
   *    LS_GET(il,index-1) = LS_GET(il,index);
   *  LS_N(il)--;
   */

  return (x);
}

void lsChar_Free(lsChar_t * il)
{
  if (!il)
    return;
  if (il->list) {
    rs_free(il->list);
  }
  rs_free(il);
}

void lsChar_free(lsChar_t * il)
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

char lsChar_get(lsChar_t * il,int i)
{
  if (!il)
    rs_error("lsChar_get: got NULL-pointer list");
  if (i < 0 || i >= il->n_list)
    rs_error("lsChar_get: index [%d] out of range [%d..%d]",i,0,il->n_list);
  
  return (il->list[i]);
}

char lsChar_getCheck(lsChar_t * il,int i,char undef)
{
  return (LS_GET_CHECK(il,i,undef));
}

int lsChar_length(lsChar_t * il)
{
  return (il) ? (il->n_list) : 0;
}

lsChar_t *  lsChar_getRowPt(lsChar_t * row, lsChar_t * il, int i, int cols)
{
  row = lsChar_take(lsChar_dropPt(lsChar_cpyPt(row,il),i*cols),cols);
  
  return (row);
}

lsChar_t *  lsChar_getRow(lsChar_t *row, lsChar_t *il, int i, int cols)
{
  lsChar_t rowPt;
  lsChar_getRowPt(lsChar_init(&rowPt), il, i, cols);

  lsChar_cpy(row, &rowPt);

  return (row);
}

lsChar_t *  lsChar_getCol(lsChar_t *col, lsChar_t *il, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  char *item;

  if (n <= j)
    return (lsChar_setNil(col));

  col = lsChar_realloc(col, n / cols + 1);
  lsChar_setNil(col);

  for (i=j, item = &LS_GET(il,j); i < n; i+=cols, item+=cols)
    lsChar_add(col,*item);

  return (col);
}

lsChar_t *  lsChar_setRow(lsChar_t *il, lsChar_t *row, int i, int cols)
{
  lsChar_t rowPt;
  int n;
  lsChar_getRowPt(lsChar_nil(&rowPt),il,i,cols);
  n = LS_N(&rowPt);
  if (n > 0)
    lsChar_cpy(&rowPt,lsChar_take(row,n));
  return (il);
}

lsChar_t *  lsChar_setCol(lsChar_t *il, lsChar_t *col, int j, int cols)
{
  int i, n = LS_N_CHECK(il);
  int k, m = LS_N_CHECK(col);
  char *item;

  if (n <= j)
    return (il);

  for (i=j, k=0, item = &LS_GET(il,j); i < n && k < m; i+=cols,item+=cols,k++)
    *item = LS_GET(col,k);

  return (il);
}

lsChar_t *  lsChar_SetPt(int n, char * items)
{
  return (lsChar_setPt(NULL,n,items));
}

lsChar_t *  lsChar_setPt(lsChar_t * il_to, int n, char * items)
{
  if (!il_to)
    il_to = lsChar_Nil();
  lsChar_free(il_to);
  LS_ITEMS(il_to) = items;
  LS_N(il_to) = n;
  il_to->max_list = n;

  return (il_to);
}

lsChar_t *  lsChar_CpyPt(lsChar_t * il_from)
{
  return (lsChar_cpyPt(NULL, il_from));
}

lsChar_t *  lsChar_cpyPt(lsChar_t * il_to, lsChar_t * il_from)
{
  if (!il_to)
    il_to = lsChar_Nil();
  lsChar_free(il_to);
  LS_ITEMS(il_to) = LS_ITEMS(il_from);
  LS_N(il_to) = LS_N(il_from);
  il_to->max_list = il_from->max_list;

  return (il_to);
}

lsChar_t * lsChar_Cpy(const lsChar_t * il_from)
{
  return (lsChar_cpy(NULL,il_from));
}

lsChar_t * lsChar_cpy(lsChar_t * il_to, const lsChar_t * il_from)
{
  int i;

  if (!il_to) 
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (!il_from) return (il_to);

  lsChar_realloc(il_to, LS_N(il_from));

  memcpy(LS_ITEMS(il_to),LS_ITEMS(il_from),LS_N(il_from)*sizeof(char));
  LS_N(il_to) = LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsChar_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

lsChar_t * lsChar_Cat(lsChar_t * il_1, lsChar_t * il_2)
{
  return (lsChar_cat(lsChar_Cpy(il_1), il_2));
}

lsChar_t * lsChar_cat(lsChar_t * il_to, lsChar_t * il_from)
{
  int i;

  if (!il_to) 
    rs_error("lsChar_cat: got NULL-pointer list");

  if (!il_from) return (il_to);

  lsChar_realloc(il_to,LS_N(il_to)+LS_N(il_from));

  memcpy(LS_ITEMS(il_to)+LS_N(il_to),LS_ITEMS(il_from),
	 LS_N(il_from)*sizeof(char));
  LS_N(il_to) += LS_N(il_from);
  /*
   *LS_FORALL_ITEMS(il_from,i) {
   *  lsChar_Cons(il_to,LS_GET(il_from,i));
   *}
   */
  return (il_to);
}

#if defined(LS_IS_lsPt)
lsChar_t * lsChar_addCat(lsChar_t *il_to, lsChar_t *il)
{
  return lsChar_add(il_to, lsChar_Cat(il_to,il));
}

lsChar_t * lsChar_AddCat(lsChar_t *il_to, lsChar_t *il)
{
  return lsChar_Add(il_to, lsChar_Cat(il_to,il));
}
#endif

lsChar_t * lsChar_drop(lsChar_t *il, int i)
{
  if (!LS_EXISTS(il,i))
    return (lsChar_setNil(il));
  LS_N(il) -= i;
  memmove(il->list,il->list+i,LS_N(il) * sizeof(char));
  return (il);
}

lsChar_t * lsChar_Drop(lsChar_t *il, int i)
{
  lsChar_t tmp, *il_split;
  if (!LS_EXISTS(il,i))
      return (NULL);
  lsChar_setPt(lsChar_nil(&tmp),LS_N(il)-i,LS_ITEMS_DROP(il,i));
  il_split = lsChar_Cpy(&tmp);
  return (il_split);
}

lsChar_t * lsChar_dropPt(lsChar_t *il, int i)
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

lsChar_t * lsChar_split(lsChar_t *il, int i)
{
  lsChar_t *il_drop = lsChar_Drop(il,i);
  lsChar_take(il,i);
  return (il_drop);
}

lsPt_t * lsChar_nsplit(lsPt_t *il_split, lsChar_t *il, lsInt_t *is)
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
      lsChar_t *split = lsChar_split(il,index);
      if (!split) continue;
      lsPt_setIndex(il_split,i+1,split,NULL);
  }
  lsPt_setIndex(il_split,0,il,NULL);
  return (il_split);
}

lsChar_t *lsChar_range(lsChar_t *il, int i0, int iN)
{
  lsChar_take(il,iN);
  if (i0 > 0) {
    lsChar_drop(il,i0-1);
  }
  return (il);
}

lsChar_t *lsChar_Range(lsChar_t *il, int i0, int iN)
{
  int n;
  lsChar_t *il_to = NULL;
  n = iN - i0;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (NULL);
  il_to = lsChar_Drop(il,i0);
  return (lsChar_take(il_to,iN));
}

lsChar_t *lsChar_rangePt(lsChar_t *il, int i0, int iN)
{
  return (lsChar_dropPt(lsChar_take(il,iN),i0));
}

lsChar_t *lsChar_cpyRange(lsChar_t *il_to, lsChar_t *il, int i0, int iN)
{
  lsChar_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsChar_rangePt(lsChar_cpyPt(lsChar_nil(&tmp),il),i0,iN);
  return (lsChar_cpy(il_to,&tmp));
}

lsChar_t *lsChar_catRange(lsChar_t *il_to, lsChar_t *il, int i0, int iN)
{
  lsChar_t tmp;
  if (!LS_EXISTS(il,i0) || iN < i0)
    return (il_to);
  lsChar_rangePt(lsChar_cpyPt(lsChar_nil(&tmp),il),i0,iN);
  return (lsChar_cat(il_to,&tmp));
}
  
lsChar_t *lsChar_reverse(lsChar_t *il)
{
  int i,n;
  if (LS_isNIL(il)) return (il);
  
  for (n = LS_N(il)-1, i = 0;
       i < n;
       i++, n--) {
    char tmp;
    tmp = LS_GET(il,i);
    LS_GET(il,i) = LS_GET(il,n);
    LS_GET(il,n) = tmp;
  }
  return (il);
}
  
#if defined(LS_IS_lsInt)
lsInt_t *lsChar_makeIndex(lsInt_t *index, lsChar_t *il)
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
lsChar_t * lsChar_join(lsChar_t *il_to, lsChar_t *il_from)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsChar_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    char x = LS_GET(il_from,i);
    if (lsChar_elem(il_to,x)) continue;

    lsChar_add(il_to,x);
  }
  return (il_to);
}
#endif
#if defined(LS_IS_lvalue)
lsChar_t * lsChar_filterByValue(lsChar_t *il, char undef, lsInt_t *indices)
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

lsChar_t * lsChar_CpyFilterByValue(lsChar_t *il, char undef, lsInt_t *indices)
{
  return (lsChar_cpyFilterByValue(NULL,il,undef,indices));
}

lsChar_t * lsChar_cpyFilterByValue(lsChar_t *il_to, lsChar_t *il_from, char undef, 
			     lsInt_t *indices)
{
  int i;
  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (indices) lsInt_setNil(indices);
  LS_FORALL_ITEMS(il_from,i) {
    if (LS_GET(il_from,i) != undef) {
      lsChar_add(il_to,LS_GET(il_from,i));
      if (indices) lsInt_add(indices,i);
    }
  }
  return (il_to);
}
#endif

lsChar_t * lsChar_filterByIndex(lsChar_t *il, lsInt_t *indices)
{
  int i,j;
  if (!il) return (NULL);
  if (!indices) 
    return (lsChar_setNil(il));

  j=0; LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il,index))
      LS_GET(il,j++) = LS_GET(il,index);
  }
  LS_N(il) = j;

  return (il);
}

lsChar_t * lsChar_CpyFilterByIndex(lsChar_t *il, lsInt_t *indices)
{
  return (lsChar_cpyFilterByIndex(NULL,il,indices));
}

lsChar_t * lsChar_cpyFilterByIndex(lsChar_t *il_to, lsChar_t *il_from, lsInt_t *indices)
{
  int i;
  if (!il_from || !indices) return (il_to);
  if (!il_to)
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  LS_FORALL_ITEMS(indices,i) {
    int index = LS_GET(indices,i);
    if (LS_EXISTS(il_from,index))
      lsChar_add(il_to,LS_GET(il_from,index));
  }
  return (il_to);
}
 
#if defined(LS_IS_lsInt)

lsChar_t * lsChar_joinInts(lsChar_t *il_to, lsChar_t *il_from, lsInt_t *value2index)
{
  int i;
  if (!value2index)
    return (lsChar_join(il_to,il_from));

  if (!il_from) return (il_to);
  if (!il_to)
    il_to = lsChar_Nil();
  
  LS_FORALL_ITEMS(il_from,i) {
    int x = LS_GET(il_from,i);
    if (LS_GET_CHECK(value2index,x,-1) != -1)
      continue;

    lsInt_setIndex(value2index,x,LS_N(il_to),-1);
    lsChar_add(il_to,x);
  }
  return (il_to);
}

#endif

#if defined(LS_IS_lvalue)
int lsChar_elem(lsChar_t * il, char item)
{
  return (lsChar_getLastIndex(il,item) >= 0);
}

int lsChar_getLastIndex(lsChar_t *il, char item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return (i);
}

int lsChar_getFstIndex(lsChar_t *il, char item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) == item) break;
  }
  return ((i < LS_N(il)) ? (i) : (-1));
}

int lsChar_getIndex(lsChar_t * il, char item)
{
  return (lsChar_getLastIndex(il,item));
}

int lsChar_neqElem(lsChar_t * il, char item)
{
  return (lsChar_getLastNeqIndex(il,item) >= 0);
}

int lsChar_getLastNeqIndex(lsChar_t * il, char item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS_REV(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i);
}

int lsChar_getFstNeqIndex(lsChar_t * il, char item)
{
  int i;
  if (!il) return (-1);
  
  LS_FORALL_ITEMS(il,i) {
    if (LS_GET(il,i) != item) break;
  }
  return (i < LS_N(il) ? (i) : (-1));
}

int lsChar_disjoint(lsChar_t *il1, lsChar_t *il2)
{
  int i;
  if (!il1 || !il2) return (1);

  LS_FORALL_ITEMS(il1,i) {
    if (lsChar_elem(il2,LS_GET(il1,i)))
      return (0);
  }
  return (1);
}

lsChar_t *lsChar_subst(lsChar_t *il, char i, char j)
{
  int h;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,h) {
    if (LS_GET(il,h) == i) LS_GET(il,h) = j;
  }
  return (il);
}

#ifndef LS_IS_lsPt

int lsChar_subBag(lsChar_t *il_sub, lsChar_t *il_super, char undef)
{
  return lsChar_subBagIndices(NULL, il_sub, il_super, undef);
}

int lsChar_subBagIndices(lsInt_t *indices,
		     lsChar_t *il_sub, lsChar_t *il_super, char undef)
{
  lsChar_t _sub;
  lsChar_t _super;
  int i;
  char last;
  char item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsChar_qsortLt(lsChar_cpy(lsChar_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsChar_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsChar_take(&_super,last_index);
      if ((last_index = lsChar_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsChar_cpyPt(lsChar_nil(&_super),il_super);

      if ((last_index = lsChar_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsChar_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}

#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)

int lsChar_subBagLimitedIndices(lsInt_t *indices,
			    lsChar_t *il_sub, lsChar_t *il_super, 
			    lsInt_t *limit, char undef)
{
  lsChar_t _sub;
  lsChar_t _super;
  int i;
  char last;
  char item;
  int last_index;
  int max_index = -1;

  if (LS_isNIL(il_sub)) return (0);
  if (LS_isNIL(il_super)) return (-1);

  lsChar_qsortLt(lsChar_cpy(lsChar_nil(&_sub),il_sub));
  if (indices)
    lsInt_setNil(indices);

  last = undef;
  while ((item = lsChar_popLast(&_sub,undef)) != undef) {
    if (item == last) {
      lsChar_take(&_super,last_index);
      if ((last_index = lsChar_getLastIndex(&_super,item)) < 0)
	break;
    } else {
      lsChar_cpyPt(lsChar_nil(&_super),il_super);
      if (limit) lsChar_take(&_super, LS_GET_CHECK(limit,item,LS_N(&_super)));

      if ((last_index = lsChar_getLastIndex(&_super,item)) < 0)
	break;
      last = item;
      if (last_index > max_index) max_index = last_index;
    }
    if (indices) 
      lsInt_add(indices,last_index);
  }
  if (indices)
    lsInt_qsortLt(indices);

  lsChar_free(&_sub);
  return ((last_index >= 0) ? max_index+1 : -1);
}
#endif

#endif

#endif

int lsChar_elemFunc(lsChar_t * il, char item, lsChar_cmp_t *func)
{
 if (!func) rs_error("lsChar_elemFunc: cmp-function undefined.");
 return (lsChar_getIndexFunc(il,item,func) >= 0);
}

int lsChar_getIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsChar_getIndexFunc: cmp-function undefined.");

  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) == 0) break;
  }
  return (i);
}

int lsChar_neqElemFunc(lsChar_t * il, char item, lsChar_cmp_t *func)
{
  if (!func) rs_error("lsChar_neqElemFunc: cmp-function undefined.");
  return (lsChar_getLastNeqIndexFunc(il,item,func) >= 0);
}

int lsChar_getLastNeqIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsChar_getLastNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

int lsChar_getFstNeqIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func)
{
  int i;
  if (!il) return (-1);
  if (!func) rs_error("lsChar_getFstNeqIndexFunc: cmp-function undefined.");
  
  LS_FORALL_ITEMS(il,i) {
    if ((*func)(LS_GET(il,i),item) != 0) break;
  }
  return (i);
}

lsChar_t * lsChar_insert(lsChar_t *il, int index, char item, char item0)
{
  int i;
  if (!il)
    il = lsChar_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsChar_insert: illegal index %d.",index);

    lsChar_setIndex(il,index,item,item0);
    return (il);
  }
  lsChar_realloc(il,LS_N(il)+1);
  memmove(il->list+index+1,il->list+index,(LS_N(il)-index)*sizeof(char));
  LS_N(il)++;
  /*
   *i = LS_N(il); lsChar_add(il,item0);
   *while ((--i) >= index)
   *  LS_GET(il,i+1) = LS_GET(il,i);
   */
  LS_GET(il,index) = item;

  return (il);
}

lsChar_t * lsChar_insertN(lsChar_t *il, int index, int n, char item, 
		    char item0)
{
  int i;
  if (n <= 0) return (il);
  if (!il)
    il = lsChar_Nil();
  if (!LS_EXISTS(il,index)) {
    if (index < 0)
      rs_error("lsChar_insert: illegal index %d.",index);

    lsChar_setIndex(il,index,item,item0);
    lsChar_setIndex(il,index+n-1,item,item);
    return (il);
  }
  i = LS_N(il); lsChar_realloc(il,i+n);
  memmove(il->list+index+n,il->list+index,(i-index) * sizeof(char));
  /*while ((--i) >= index)
   *  LS_GET(il,i+n) = LS_GET(il,i);
   */
  while (n--)
    LS_GET(il,index++) = item;
  LS_N(il) += n;

  return (il);
}


lsChar_t * lsChar_insSwap(lsChar_t *il, int index, char item, char item0)
{
  int i,n;
  char _item;

  if (!il)
    il = lsChar_Nil();

  if (index < 0)
    rs_error("lsChar_insSwap: illegal index %d.",index);

  n = LS_N(il);
  _item = lsChar_setIndex(il,index,item,item0);
#if defined(LS_IS_lvalue)
  if (_item != item0) 
    lsChar_add(il,_item);
#else
  if (index < n)
    lsChar_add(il,_item);
#endif

  return (il);
}

char lsChar_getFlip(int i, lsChar_t *il, char undef)
{
  char item = LS_GET_CHECK(il,i,undef);
  return item;
}

lsChar_t * lsChar_map(lsChar_t * il, lsChar_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i));
  }
  return (il);
}

lsChar_t * lsChar_map_2(lsChar_t * il, lsChar_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsChar_t * lsChar_map_3(lsChar_t * il, lsChar_map_3_t *func, void *arg1, void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsChar_t * lsChar_mapSet(lsChar_t * il, lsChar_map_t *func)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i));
  }
  return (il);
}

lsChar_t * lsChar_mapSet_2(lsChar_t * il, lsChar_map_2_t *func, void *arg)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg);
  }
  return (il);
}

lsChar_t * lsChar_mapSet_3(lsChar_t * il, lsChar_map_3_t *func, void *arg1,void *arg2)
{
  int i;
  if (!il) return (il);

  LS_FORALL_ITEMS(il,i) {
    LS_GET(il,i) = (*func)(LS_GET(il,i),arg1,arg2);
  }
  return (il);
}

lsChar_t * lsChar_CpyMap(lsChar_t * il_from, lsChar_map_t *func)
{
  return (lsChar_cpyMap(NULL,il_from,func));
}

lsChar_t * lsChar_cpyMap(lsChar_t * il_to, lsChar_t * il_from, lsChar_map_t *func)
{
  int i;

  if (!il_to) 
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsChar_add(il_to,(*func)(LS_GET(il_from,i)));
  }
  return (il_to);
}

lsChar_t *  lsChar_CpyMap_2(lsChar_t * il_from, lsChar_map_2_t *func, void *arg)
{
  return (lsChar_cpyMap_2(NULL, il_from, func, arg));
}

lsChar_t *  lsChar_CpyMap_3(lsChar_t * il_from, lsChar_map_3_t *func, 
		      void *arg1, void *arg2)
{
  return (lsChar_cpyMap_3(NULL, il_from, func, arg1, arg2));
}

lsChar_t *  lsChar_cpyMap_2(lsChar_t * il_to, lsChar_t * il_from, 
		      lsChar_map_2_t *func, void *arg)
{
  int i;

  if (!il_to) 
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsChar_add(il_to,(*func)(LS_GET(il_from,i),arg));
  }
  return (il_to);
}

lsChar_t *  lsChar_cpyMap_3(lsChar_t * il_to, lsChar_t * il_from, 
		      lsChar_map_3_t *func, void *arg1, void *arg2)
{
  int i;

  if (!il_to) 
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    lsChar_add(il_to,(*func)(LS_GET(il_from,i),arg1,arg2));
  }
  return (il_to);
}

lsChar_t * lsChar_CartProd(lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func)
{
  return (lsChar_cpyCartProd(NULL,il1,il2,func));
}

lsChar_t * lsChar_cpyCartProd(lsChar_t *il_to, 
			lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func)
{
  int i,j;
  if (LS_isNIL(il1)) return lsChar_cpy(il_to,il2);
  if (LS_isNIL(il2)) return lsChar_cpy(il_to,il1);
  if (!il_to)
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);
  
  LS_FORALL_ITEMS(il1,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsChar_add(il_to, (*func)(LS_GET(il1,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsChar_add(il_to, LS_GET(il1,i) * LS_GET(il2,j));
#else
	rs_error("lsChar_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il_to);
}

lsChar_t * lsChar_cartProd(lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func)
{
  int i,j;
  lsChar_t tmp;
  if (LS_isNIL(il1)) return lsChar_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsChar_cpy(lsChar_nil(&tmp),il1);
  lsChar_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsChar_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j)));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsChar_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsChar_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}

lsChar_t * lsChar_cartProd_2(lsChar_t *il1, lsChar_t *il2, lsChar_fold_2_t *func,
		       void *arg)
{
  int i,j;
  lsChar_t tmp;
  if (LS_isNIL(il1)) return lsChar_cpy(il1,il2);
  if (LS_isNIL(il2)) return il1;

  lsChar_cpy(lsChar_nil(&tmp),il1);
  lsChar_setNil(il1);
  
  LS_FORALL_ITEMS(&tmp,i) {
    LS_FORALL_ITEMS(il2,j) {
      if (func) lsChar_add(il1, (*func)(LS_GET(&tmp,i),LS_GET(il2,j), arg));
      else {
#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)
	lsChar_add(il1, LS_GET(&tmp,i) * LS_GET(il2,j));
#else
	rs_error("lsChar_cartProd: undefined function.");
#endif
      }
    }
  }
  return (il1);
}
  
lsChar_t * lsChar_filter(lsChar_t * il, lsChar_filter_t *func)
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

lsChar_t * lsChar_CpyFilter(lsChar_t *il_from, lsChar_filter_t *func)
{
  return (lsChar_cpyFilter(NULL,il_from,func));
}

lsChar_t * lsChar_cpyFilter(lsChar_t *il_to, lsChar_t *il_from, lsChar_filter_t *func)
{
  int i,j;
  
  if (!il_to)
    il_to = lsChar_Nil();
  else
    lsChar_setNil(il_to);

  if (!il_from) return (il_to);

  LS_FORALL_ITEMS(il_from,i) {
    if ((*func)(LS_GET(il_from,i)))
      lsChar_add(il_to,LS_GET(il_from,i));
  }
  return (il_to);
}

char lsChar_foldl(lsChar_t *il, char item0, lsChar_fold_t *func)
{
  int i;
  char result = item0;
  if (!func)
    rs_error("lsChar_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i));
  }
  return (result);
}

char lsChar_foldr(lsChar_t *il, char item0, lsChar_fold_t *func)
{
  int i;
  char result = item0;
  if (!func)
    rs_error("lsChar_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result);
  }
  return (result);
}

char lsChar_foldl_2(lsChar_t *il, char item0, lsChar_fold_2_t *func,
		     void *arg)
{
  int i;
  char result = item0;
  if (!func)
    rs_error("lsChar_foldl: illegal null-pointer function.");
  
  LS_FORALL_ITEMS(il,i) {
    result = (*func)(result,LS_GET(il,i),arg);
  }
  return (result);
}

char lsChar_foldr_2(lsChar_t *il, char item0, lsChar_fold_2_t *func,
		     void *arg)
{
  int i;
  char result = item0;
  if (!func)
    rs_error("lsChar_foldr: illegal null-pointer function.");
  
  LS_FORALL_ITEMS_REV(il,i) {
    result = (*func)(LS_GET(il,i),result,arg);
  }
  return (result);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

lsChar_t * lsChar_sscan_chr(lsChar_t *il, char t, char *s)
{
  char *p;
  char v;

  if (!il)
    il = lsChar_Nil();
  else
    lsChar_setNil(il);

  while (p = strchrsplit(&s,t)) {
    if (*p) {
#if defined(LS_IS_lsInt) || defined(LS_IS_lsULInt)
      if (!isdigit(*p) && *p != '-') continue; v  = atoi(p);
#elif defined(LS_IS_lsFloat) || defined(LS_IS_lsDouble)
      if (!isdigit(*p) && !strchr("-.",*p)) continue; v  = atof(p);
#elif defined(LS_IS_lsChar)
      if (iscntrl(*p)) continue; v  = *p;
#endif
      lsChar_add(il,v);
    }
  }
  return (il);
}

char * lsChar_sprint_chr(char *s, lsChar_t *il, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    char x = LS_GET(il,i);
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

char * lsChar_sprintf_chr(char *s, lsChar_t *il, const char *format, char t)
{
  char tmp[256];
  int  i, n = 0, max = (s) ? strlen(s) : 0;
  if (s) *s = '\0';

  if (!il) return (s);

  LS_FORALL_ITEMS(il,i) {
    int n_tmp;
    char x = LS_GET(il,i);
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
 
int lsChar_fwrite(FILE *fp, lsChar_t *il)
{
    char *items;
    int i,l,n,m;
    if (!fp || LS_isNIL(il)) return (0);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : LS_N(il);
    n = LS_N(il) / m;

    for (items=LS_ITEMS(il), i=0; i < n; i++, items+=m) {
	int s;
	if ((s = fwrite(items, sizeof(char), m, fp)) != m) {
	    rs_error("lsChar_fwrite: write error [%d/%d] at %d.\n",s,m,i);
	}
    }
    if ((l = LS_N(il) % m) > 0) {
	int s;
	if ((s = fwrite(items, sizeof(char), l, fp)) != l) {
	    rs_error("lsChar_fwrite: write error [%d/%d] at %d.\n",s,l,i);
	}
    }
    return (LS_N(il));
}    

int lsChar_fread(lsChar_t *il, int k, FILE *fp)
{
    char *items;
    int i,l,n,m;
    if (!fp || !il) return (0);

    /* if k==0 read until EOF; in this case k must be > 0 */
    if (LS_FILE_BUFSIZE <= 0 && k <= 0) return (-1);

    m = (LS_FILE_BUFSIZE > 0) ? LS_FILE_BUFSIZE : k;
    n = (k > 0) ? k / m : -1;

    lsChar_realloc(lsChar_setNil(il),(k > 0) ? k : LS_FILE_BUFSIZE);

    for (items=LS_ITEMS(il), i=n; i != 0; i--, items+=m, LS_N(il)+=m) {
	int s;
	if ((s = fread(items, sizeof(char), m, fp)) != m) {
	    LS_N(il)+=s;
	    return (LS_N(il));
	}
    }
    if ((l = k % m) > 0) {
	int s;
	s = fread(items, sizeof(char), l, fp);
	LS_N(il)+=s;
    }
    return (LS_N(il));
}

#endif

int _lsChar_lsChar_minmax(lsChar_t *il, int mode, lsChar_cmp_t *func)
{
  int i,iminmax;
  char minmax;
  if (LS_isNIL(il)) 
    rs_error("lsChar_maxFunc: empty list");
  minmax = LS_GET(il,iminmax=0);
  for (i=1; i < LS_N(il); i++) {
    char tmp = LS_GET(il,i);
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
      rs_error("_lsChar_lsChar_minmax: undefined mode %d.",mode);
    }
    if (cmp < 0) {
      minmax = tmp; iminmax = i;
    } 
  }
  return (iminmax);
}

char lsChar_maxFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  int i = _lsChar_lsChar_minmax(il,3,func);
  return (LS_GET(il,i));
}

char lsChar_minFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  int i = _lsChar_lsChar_minmax(il,2,func);
  return (LS_GET(il,i));
}

int lsChar_maxIndexFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  return _lsChar_lsChar_minmax(il,3,func);
}

int lsChar_minIndexFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  return _lsChar_lsChar_minmax(il,2,func);
}

lsChar_t * lsChar_sortByIndex(lsChar_t *il, lsInt_t *index, char undef)
{
  lsChar_t *tmp = NULL;
  tmp = lsChar_cpySortByIndex(tmp, il, index, undef);
  if (tmp) {
      lsChar_cpy(il,tmp);
      lsChar_Free(tmp);
  }
  return (il);
}

lsChar_t * lsChar_cpySortByIndex(lsChar_t *il_to, lsChar_t *il_from, 
			   lsInt_t *index, char undef)
{
  int i;
  lsChar_setNil(il_to);
  if (!index) return (il_to);
  if (!il_from) return (il_to);
  if (!il_to) 
      il_to = lsChar_Nil();

  LS_FORALL_ITEMS(index,i) {
    int j = LS_GET(index,i);
    char item = LS_GET_CHECK(il_from,j,undef);
#if defined(LS_IS_lvalue)    
    if (item != undef) {
	lsChar_setIndex(il_to,i,item,undef);
    }
#else
    lsChar_setIndex(il_to,i,item,undef);    
#endif
  }
  return (il_to);
}

void _lsChar_qsortX(char *v, int left, int right, 
		int mode, lsChar_cmp_t *func)
{
  char x = v[left]; /* bestimme Trennelement */
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
      char h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsChar_qsortX(v,left,r,mode,func);
  if (l < right) _lsChar_qsortX(v,l,right,mode,func);
}

lsChar_t * lsChar_qsortLtFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX(LS_ITEMS(il),0,LS_N(il)-1,2,func);
  }
  return (il);
}

lsChar_t * lsChar_qsortGtFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX(LS_ITEMS(il),0,LS_N(il)-1,3,func);
  }
  return (il);
}

void _lsChar_qsortX_2(char *v, int left, int right, 
		  int mode, lsChar_cmp_2_t *func, void *arg)
{
  char x = v[left]; /* bestimme Trennelement */
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
      char h = v[r];
      v[r] = v[l];
      v[l] = h;
      l++;
      r--;
    }
  } while (l <= r);
  /* Sortiere Teilfolgen */
  if (left < r) _lsChar_qsortX_2(v,left,r,mode,func,arg);
  if (l < right) _lsChar_qsortX_2(v,l,right,mode,func,arg);
}

lsChar_t * lsChar_qsortLtFunc_2(lsChar_t *il, lsChar_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,2,func,arg);
  }
  return (il);
}

lsChar_t * lsChar_qsortGtFunc_2(lsChar_t *il, lsChar_cmp_2_t *func, void *arg)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX_2(LS_ITEMS(il),0,LS_N(il)-1,3,func,arg);
  }
  return (il);
}

int lsChar_cmpIndex(int i, int j, pairPt_t *arg)
{
  lsChar_t *il = tplFst(arg);
  lsChar_cmp_t *func = (lsChar_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(LS_GET(il,i),LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      char x = LS_GET(il,i);
      char y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsChar_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsChar_cmpIndexPt(int i, int j, pairPt_t *arg)
{
  lsChar_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);

  if (func) {
      return ((*func)(&LS_GET(il,i),&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      char x = LS_GET(il,i);
      char y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsChar_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsChar_cmpValue2Index(int dummy, int j, triplePt_t *arg)
{
  lsChar_t *il = tplFst(arg);
  lsChar_cmp_t *func = (lsChar_cmp_t *) tplSnd(arg);
  char *value = (char *) tplTrd(arg);

  if (func) {
      return ((*func)(*value,LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      char x = *value;
      char y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsChar_cmpIndex: no cmp-function defined");
#endif
  }
}

int lsChar_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg)
{
  lsChar_t *il = tplFst(arg);
  lsPt_cmp_t *func = (lsPt_cmp_t *) tplSnd(arg);
  char *value = (char *) tplTrd(arg);

  if (func) {
      return ((*func)(value,&LS_GET(il,j)));
  } else {
#if defined(LS_IS_lvalue)      
      char x = *value;
      char y = LS_GET(il,j);
      if (x < y) return (-1);
      if (x > y) return (1);
      return (0);
#else
      rs_error("lsChar_cmpIndex: no cmp-function defined");
#endif
  }
}
  
lsInt_t * lsChar_qsortIndexLtFunc(lsInt_t *index, lsChar_t *il, 
			      lsChar_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsChar_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsChar_qsortIndexLt(lsInt_t *index, lsChar_t *il)
{
  return (lsChar_qsortIndexLtFunc(index,il,NULL));
}

lsInt_t * lsChar_qsortIndexGtFunc(lsInt_t *index, lsChar_t *il, 
			      lsChar_cmp_t *func)
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
		    (lsInt_cmp_2_t *) lsChar_cmpIndex, 
		    pairPt_set(&arg,il,(void *) func));
  }
  return (index);
}

lsInt_t * lsChar_qsortIndexGt(lsInt_t *index, lsChar_t *il)
{
  return (lsChar_qsortIndexGtFunc(index,il,NULL));
}

void _lsChar_mergeX(char *v, char *w, int ll, int rl, int rr,
		int mode, lsChar_cmp_t *func)
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
      rs_error("_lsChar_mergeX: undefined mode %d.",mode);
    }
  }
  while (ll < lr)
    v[i++] = w[ll++];
  while (rl < rr)
    v[i++] = w[rl++];
}

void _lsChar_msortX(char *v, char *w, int left, int right, 
		int mode, lsChar_cmp_t *func)
{
  int m;
  if (right - left <= 1) return;
  /* split / devide */
  m = (left + right) / 2;
  /* loese Teilprobleme */
  _lsChar_msortX(v,w,left,m,mode,func);
  _lsChar_msortX(v,w,m,right,mode,func);
  /* merge / conquer */
  _lsChar_mergeX(w,v,left,m,right,mode,func);
}

lsChar_t * lsChar_msortLtFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  static lsChar_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsChar_realloc(_il,LS_N(il));
    _lsChar_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),2,func);
  }
  return (il);
}

lsChar_t * lsChar_msortGtFunc(lsChar_t *il, lsChar_cmp_t *func)
{
  static lsChar_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsChar_realloc(_il,LS_N(il));
    _lsChar_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),3,func);
  }
  return (il);
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

char lsChar_sum(lsChar_t *il)
{
  char sum = 0;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum += LS_GET(il,i);
  }
  return (sum);
}

char lsChar_prod(lsChar_t *il)
{
  char sum = 1;
  int i;
  if (!il) return (sum);
  LS_FORALL_ITEMS(il,i) {
      sum *= LS_GET(il,i);
  }
  return (sum);
}

lsChar_t * lsChar_scale(lsChar_t *il, char s)
{
  int i;
  if (!il) return (il);
  LS_FORALL_ITEMS(il,i) {
      LS_GET(il,i) *= s;
  }
  return (il);
}

lsChar_t * lsChar_delta(lsChar_t *il_to, lsChar_t *il_from, char base)
{
  int i;
  lsChar_setNil(il_to);
  if (!il_from) return (il_to);
  il_to = lsChar_realloc(il_to, LS_N(il_from));

  LS_FORALL_ITEMS(il_from,i) {
    LS_GET(il_to,i) = LS_GET(il_from,i) - base;
    base = LS_GET(il_from,i);
  }
  LS_N(il_to) = LS_N(il_from);

  return (il_to);
}

char lsChar_max(lsChar_t *il)
{
  int i = _lsChar_lsChar_minmax(il,1,NULL);
  return (LS_GET(il,i));
}

char lsChar_min(lsChar_t *il)
{
  int i = _lsChar_lsChar_minmax(il,0,NULL);
  return (LS_GET(il,i));
}

int lsChar_maxIndex(lsChar_t *il)
{
  return _lsChar_lsChar_minmax(il,1,NULL);
}

int lsChar_minIndex(lsChar_t *il)
{
  return _lsChar_lsChar_minmax(il,0,NULL);
}

lsChar_t *lsChar_rmdup(lsChar_t *il)
{
  int i,j;
  char item;
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
      
    
lsChar_t * lsChar_qsortLt(lsChar_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX(LS_ITEMS(il),0,LS_N(il)-1,0,NULL);
  }
  return (il);
}

lsChar_t * lsChar_qsortGt(lsChar_t *il)
{
  if (il && LS_N(il) > 0) {
    _lsChar_qsortX(LS_ITEMS(il),0,LS_N(il)-1,1,NULL);
  }
  return (il);
}

lsChar_t * lsChar_msortLt(lsChar_t *il)
{
  static lsChar_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsChar_realloc(_il,LS_N(il));
    _lsChar_msortX(LS_ITEMS(_il),LS_ITEMS(il),0,LS_N(il),0,NULL);
  }
  return (il);
}

lsChar_t * lsChar_msortGt(lsChar_t *il)
{
  static lsChar_t *_il = NULL;
  if (il && LS_N(il) > 0) {
    _il = lsChar_realloc(_il,LS_N(il));
    _lsChar_msortX(LS_ITEMS(_il),LS_ITEMS(il),1,LS_N(il),0,NULL);
  }
  return (il);
}

#endif

int _lsChar_bsearchX(lsChar_t *il, char i,
		 int mode, lsChar_cmp_t *func)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    char x = LS_GET(il,m);
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
      rs_error("_lsChar_bsearchX: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsChar_bsearchLtFunc(lsChar_t *il, char i, lsChar_cmp_t *func)
{
  return (_lsChar_bsearchX(il,i,2,func));
}

int lsChar_bsearchGtFunc(lsChar_t *il, char i, lsChar_cmp_t *func)
{
  return (_lsChar_bsearchX(il,i,3,func));
}

int _lsChar_bsearchX_2(lsChar_t *il, char i,
		   int mode, lsChar_cmp_2_t *func, void *arg)
{
  int l = 0;
  int r = LS_N_CHECK(il)-1;
  while (r >= l) {
    int m = (r + l) / 2;
    char x = LS_GET(il,m);
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
      rs_error("_lsChar_bsearchX_2: undefined mode %d.",mode);
    }
  }
  return (-(l+1)); /* Kodierung des Einfuege-Index */ 
}

int lsChar_bsearchLtFunc_2(lsChar_t *il,char i,lsChar_cmp_2_t *func,void *arg)
{
  return (_lsChar_bsearchX_2(il,i,2,func,arg));
}

int lsChar_bsearchGtFunc_2(lsChar_t *il,char i,lsChar_cmp_2_t *func,void *arg)
{
  return (_lsChar_bsearchX_2(il,i,3,func,arg));
}

#if defined(LS_IS_lvalue) && !defined(LS_IS_lsPt)

int lsChar_bsearchLt(lsChar_t *il, char i)
{
  return (_lsChar_bsearchX(il,i,0,NULL));
}

int lsChar_bsearchGt(lsChar_t *il, char i)
{
  return (_lsChar_bsearchX(il,i,1,NULL));
}

#endif 

int lsChar_cmpFunc(lsChar_t *il1, lsChar_t *il2, lsChar_cmp_t *func)
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
      char x = LS_GET(il1,i);
      char y = LS_GET(il2,i);
      if (x == y)
	continue;
      else if (x < y)
	return (-1);
      else
	return (1);
#else
      rs_error("lsChar_cmpFunc: no cmp-function defined");
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

int lsChar_cmp(lsChar_t *il1, lsChar_t *il2)
{
  return (lsChar_cmpFunc(il1,il2,NULL));
}

#endif
  

