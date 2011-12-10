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

btreeInt_t *btreeInt_nil(btreeInt_t *t)
{
    if (!t) {
	t = (btreeInt_t *) rs_malloc(sizeof(btreeInt_t));
    }
    lsInt_nil(&t->list);
    lsPairInt_nil(&t->children);
    lsInt_nil(&t->parent);
    return (t);
}

btreeInt_t *btreeInt_Nil(void)
{
    return (btreeInt_nil(NULL));
}

void btreeInt_free(btreeInt_t *t)
{
    if (!t) return;
    lsInt_free(&t->list);
    lsPairInt_free(&t->children);
    lsInt_free(&t->parent);
}

void btreeInt_Free(btreeInt_t *t)
{
    if (!t) return;
    btreeInt_free(t);
    free(t);
}

btreeInt_t *btreeInt_setNil(btreeInt_t *t)
{
    if (!t) return (t);
    lsInt_setNil(&t->list);
    lsPairInt_setNil(&t->children);
    lsInt_setNil(&t->parent);
    return (t);
}

btreeInt_t *btreeInt_cpy(btreeInt_t *copy, const btreeInt_t *t)
{
    if (!t) return (copy);

    copy = btreeInt_realloc(copy, BTREE_N(t));
    lsInt_cpy(&copy->list, &t->list);
    lsPairInt_cpy(&copy->children, &t->children);
    lsInt_cpy(&copy->parent, &t->parent);

    return (copy);
}
    
btreeInt_t *btreeInt_Cpy(const btreeInt_t *t)
{
    return (btreeInt_cpy(NULL, t));
}

btreeInt_t *btreeInt_realloc(btreeInt_t *t, int max_nodes)
{
    if (!t) {
	t = btreeInt_Nil();
    }
    lsInt_realloc(&t->list, max_nodes);
    lsPairInt_realloc(&t->children, max_nodes);
    lsInt_realloc(&t->parent, max_nodes);

    return (t);
}

int btreeInt_newParent(btreeInt_t *t, int i, int node1, int node2)
{
    int node;
    if (!t) return (BTREE_UNDEF);
    node = LS_N(lsInt_add(&t->list,i))-1;
    pairInt_set(lsPairInt_getNewItem(&t->children), node1, node2);
    lsInt_add(&t->parent,BTREE_UNDEF);
    if (BTREE_EXISTS(t,node1)) 
	BTREE_GET_PARENT(t,node1) = node;
    if (BTREE_EXISTS(t,node2))
	BTREE_GET_PARENT(t,node2) = node;
    return (node);
}

int btreeInt_newNode(btreeInt_t *t, int i)
{
    return btreeInt_newParent(t,i,BTREE_UNDEF,BTREE_UNDEF);
}

btreeInt_t *btreeInt_addParent(btreeInt_t *t, int i, int node1, int node2)
{
    btreeInt_newParent(t,i,node1,node2);
    return (t);
}

btreeInt_t *btreeInt_addNode(btreeInt_t *t, int i)
{
    btreeInt_newNode(t,i);
    return (t);
}






