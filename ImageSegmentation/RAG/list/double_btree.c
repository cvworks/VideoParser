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

btreeDouble_t *btreeDouble_nil(btreeDouble_t *t)
{
    if (!t) {
	t = (btreeDouble_t *) rs_malloc(sizeof(btreeDouble_t));
    }
    lsDouble_nil(&t->list);
    lsPairInt_nil(&t->children);
    lsInt_nil(&t->parent);
    return (t);
}

btreeDouble_t *btreeDouble_Nil(void)
{
    return (btreeDouble_nil(NULL));
}

void btreeDouble_free(btreeDouble_t *t)
{
    if (!t) return;
    lsDouble_free(&t->list);
    lsPairInt_free(&t->children);
    lsInt_free(&t->parent);
}

void btreeDouble_Free(btreeDouble_t *t)
{
    if (!t) return;
    btreeDouble_free(t);
    free(t);
}

btreeDouble_t *btreeDouble_setNil(btreeDouble_t *t)
{
    if (!t) return (t);
    lsDouble_setNil(&t->list);
    lsPairInt_setNil(&t->children);
    lsInt_setNil(&t->parent);
    return (t);
}

btreeDouble_t *btreeDouble_cpy(btreeDouble_t *copy, const btreeDouble_t *t)
{
    if (!t) return (copy);

    copy = btreeDouble_realloc(copy, BTREE_N(t));
    lsDouble_cpy(&copy->list, &t->list);
    lsPairInt_cpy(&copy->children, &t->children);
    lsInt_cpy(&copy->parent, &t->parent);

    return (copy);
}
    
btreeDouble_t *btreeDouble_Cpy(const btreeDouble_t *t)
{
    return (btreeDouble_cpy(NULL, t));
}

btreeDouble_t *btreeDouble_realloc(btreeDouble_t *t, int max_nodes)
{
    if (!t) {
	t = btreeDouble_Nil();
    }
    lsDouble_realloc(&t->list, max_nodes);
    lsPairInt_realloc(&t->children, max_nodes);
    lsInt_realloc(&t->parent, max_nodes);

    return (t);
}

int btreeDouble_newParent(btreeDouble_t *t, double i, int node1, int node2)
{
    int node;
    if (!t) return (BTREE_UNDEF);
    node = LS_N(lsDouble_add(&t->list,i))-1;
    pairInt_set(lsPairInt_getNewItem(&t->children), node1, node2);
    lsInt_add(&t->parent,BTREE_UNDEF);
    if (BTREE_EXISTS(t,node1)) 
	BTREE_GET_PARENT(t,node1) = node;
    if (BTREE_EXISTS(t,node2))
	BTREE_GET_PARENT(t,node2) = node;
    return (node);
}

int btreeDouble_newNode(btreeDouble_t *t, double i)
{
    return btreeDouble_newParent(t,i,BTREE_UNDEF,BTREE_UNDEF);
}

btreeDouble_t *btreeDouble_addParent(btreeDouble_t *t, double i, int node1, int node2)
{
    btreeDouble_newParent(t,i,node1,node2);
    return (t);
}

btreeDouble_t *btreeDouble_addNode(btreeDouble_t *t, double i)
{
    btreeDouble_newNode(t,i);
    return (t);
}






