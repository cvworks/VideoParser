#ifndef ALL_LIST_H
#define ALL_LIST_H

#define LS_FORALL_ITEMS(l,i) for ((i)=0;(i)<(l)->n_list;(i)++)
#define LS_NEXT_ITEM(l,i) ((((++(i)) < 0) ? (i)=0 : (i)) < (l)->n_list)
#define LS_FORALL_ITEMS_REV(l,i) for ((i)=(l)->n_list-1; (i)>=0; (i)--)
#define LS_NEXT_ITEM_REV(l,i) \
        ((((--(i)) >= (l)->n_list) ? (i)=(l)->n_list-1 : (i)) >= 0)
#define LS_EXISTS(l,i) ((l)&&(i)>=0 &&(i)<(l)->n_list)
#define LS_isNIL(l) (!(l)||(l)->n_list==0)
#define LS_GET(l,i) ((l)->list[i])
#define LS_GET_CHECK(l,i,i0) (LS_EXISTS(l,i) ? LS_GET(l,i):(i0))
#define LS_N(l) ((l)->n_list)
#define LS_N_CHECK(l) (LS_isNIL(l) ? 0 : (l)->n_list)
#define LS_ITEMS(l) ((l)->list)
#define LS_ITEMS_DROP(l,n) \
        (((n)>= 0 && (n)<(l)->max_list) ? ((l)->list+(n)):NULL)
#define LS_LAST(l) ((l)->list[LS_N(l)-1])
#define LS_LAST_CHECK(l,i0) (LS_isNIL(l) ? (i0) : LS_LAST(l))
#define LS_FIRST(l) ((l)->list[0])
#define LS_FIRST_CHECK(l,i0) (LS_isNIL(l) ? (i0) : LS_FIRST(l))
#define LS_HEAD(l) ((l)->list[0])
#define LS_HEAD_CHECK(l,i0) (LS_isNIL(l) ? (i0) : LS_FIRST(l))
#define LS_MAX_ITEMS(l) ((l)->max_list)

#define LS_bsearchIndex(i) (-((i)+1))
#define LS_bsearchTest(i)  ((i) >= 0)
#endif
