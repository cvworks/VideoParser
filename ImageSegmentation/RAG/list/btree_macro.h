/*
 * list/btree_macro.h
 *
 * binary trees
 *
 * Sven Wachsmuth, 17.03.2004
 */
#ifndef _LS_BTREE_MACRO_H
#define _LS_BTREE_MACRO_H

#include "list_macro.h"
#include "tuple_macro.h"

#define BTREE_UNDEF (-1)
#define BTREE_GET(t,node) (LS_GET(&(t)->list,node))
#define BTREE_GET_PARENT(t,node) (LS_GET(&(t)->parent,node))
#define BTREE_GET_FST_CHILD(t,node) (tplFst(&LS_GET(&(t)->children,node)))
#define BTREE_GET_SND_CHILD(t,node) (tplSnd(&LS_GET(&(t)->children,node)))
#define BTREE_EXISTS(t,node) ((t) && LS_EXISTS(&(t)->list,node))
#define BTREE_N(t) (LS_N(&(t)->list))
#define BTREE_IS_LEAF(t,node) (BTREE_GET_FST_CHILD(t,node) == BTREE_UNDEF \
&& BTREE_GET_SND_CHILD(t,node) == BTREE_UNDEF)

#endif
