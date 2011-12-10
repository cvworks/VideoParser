/*
 * list/btree.h
 *
 * binary trees
 *
 * Sven Wachsmuth, 17.03.2004
 */
#include "list.h"
#include "btree_macro.h"

typedef struct btree_t {
    list_t      list; /* content of tree nodes, indexed by node-id */
    lsPairInt_t children; /* child-node-ids, indexed by parent-node-id */
    lsInt_t     parent; /* parent-node-id, indexed by child-node-id */
} btree_t;

btree_t *btree_nil(btree_t *t);
btree_t *btree_Nil(void);

void btree_free(btree_t *t);
void btree_Free(btree_t *t);

btree_t *btree_setNil(btree_t *t);

btree_t *btree_cpy(btree_t *copy, const btree_t *t);
btree_t *btree_Cpy(const btree_t *t);
btree_t *btree_realloc(btree_t *t, int max_nodes);

int      btree_newParent(btree_t *t, LIST_TYPE i, int node1, int node2);
int      btree_newNode(btree_t *t, LIST_TYPE i);
btree_t *btree_addParent(btree_t *t, LIST_TYPE i, int node1, int node2);
btree_t *btree_addNode(btree_t *t, LIST_TYPE i);






