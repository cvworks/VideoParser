/*
 * list/btree.h
 *
 * binary trees
 *
 * Sven Wachsmuth, 17.03.2004
 */
#include "list.h"
#include "btree_macro.h"

typedef struct btreeDouble_t {
    lsDouble_t      list; /* content of tree nodes, indexed by node-id */
    lsPairInt_t children; /* child-node-ids, indexed by parent-node-id */
    lsInt_t     parent; /* parent-node-id, indexed by child-node-id */
} btreeDouble_t;

btreeDouble_t *btreeDouble_nil(btreeDouble_t *t);
btreeDouble_t *btreeDouble_Nil(void);

void btreeDouble_free(btreeDouble_t *t);
void btreeDouble_Free(btreeDouble_t *t);

btreeDouble_t *btreeDouble_setNil(btreeDouble_t *t);

btreeDouble_t *btreeDouble_cpy(btreeDouble_t *copy, const btreeDouble_t *t);
btreeDouble_t *btreeDouble_Cpy(const btreeDouble_t *t);
btreeDouble_t *btreeDouble_realloc(btreeDouble_t *t, int max_nodes);

int      btreeDouble_newParent(btreeDouble_t *t, double i, int node1, int node2);
int      btreeDouble_newNode(btreeDouble_t *t, double i);
btreeDouble_t *btreeDouble_addParent(btreeDouble_t *t, double i, int node1, int node2);
btreeDouble_t *btreeDouble_addNode(btreeDouble_t *t, double i);






