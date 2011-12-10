/*
 * list/btree.c
 *
 * binary trees
 *
 * Sven Wachsmuth, 17.03.2004
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "btree.h"

btree_t *btree_nil(btree_t *t)
{
    if (!t) {
	t = (btree_t *) rs_malloc(sizeof(btree_t));
    }
    ls_nil(&t->list);
    lsPairInt_nil(&t->children);
    lsInt_nil(&t->parent);
    return (t);
}

btree_t *btree_Nil(void)
{
    return (btree_nil(NULL));
}

void btree_free(btree_t *t)
{
    if (!t) return;
    ls_free(&t->list);
    lsPairInt_free(&t->children);
    lsInt_free(&t->parent);
}

void btree_Free(btree_t *t)
{
    if (!t) return;
    btree_free(t);
    free(t);
}

btree_t *btree_setNil(btree_t *t)
{
    if (!t) return (t);
    ls_setNil(&t->list);
    lsPairInt_setNil(&t->children);
    lsInt_setNil(&t->parent);
    return (t);
}

btree_t *btree_cpy(btree_t *copy, const btree_t *t)
{
    if (!t) return (copy);

    copy = btree_realloc(copy, BTREE_N(t));
    ls_cpy(&copy->list, &t->list);
    lsPairInt_cpy(&copy->children, &t->children);
    lsInt_cpy(&copy->parent, &t->parent);

    return (copy);
}
    
btree_t *btree_Cpy(const btree_t *t)
{
    return (btree_cpy(NULL, t));
}

btree_t *btree_realloc(btree_t *t, int max_nodes)
{
    if (!t) {
	t = btree_Nil();
    }
    ls_realloc(&t->list, max_nodes);
    lsPairInt_realloc(&t->children, max_nodes);
    lsInt_realloc(&t->parent, max_nodes);

    return (t);
}

int btree_newParent(btree_t *t, LIST_TYPE i, int node1, int node2)
{
    int node;
    if (!t) return (BTREE_UNDEF);
    node = LS_N(ls_add(&t->list,i))-1;
    pairInt_set(lsPairInt_getNewItem(&t->children), node1, node2);
    lsInt_add(&t->parent,BTREE_UNDEF);
    if (BTREE_EXISTS(t,node1)) 
	BTREE_GET_PARENT(t,node1) = node;
    if (BTREE_EXISTS(t,node2))
	BTREE_GET_PARENT(t,node2) = node;
    return (node);
}

int btree_newNode(btree_t *t, LIST_TYPE i)
{
    return btree_newParent(t,i,BTREE_UNDEF,BTREE_UNDEF);
}

btree_t *btree_addParent(btree_t *t, LIST_TYPE i, int node1, int node2)
{
    btree_newParent(t,i,node1,node2);
    return (t);
}

btree_t *btree_addNode(btree_t *t, LIST_TYPE i)
{
    btree_newNode(t,i);
    return (t);
}






