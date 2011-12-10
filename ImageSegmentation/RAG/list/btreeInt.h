/*
 * list/btree.h
 *
 * binary trees
 *
 * Sven Wachsmuth, 17.03.2004
 */
#include "list.h"
#include "btree_macro.h"

typedef struct btreeInt_t {
    lsInt_t      list; /* content of tree nodes, indexed by node-id */
    lsPairInt_t children; /* child-node-ids, indexed by parent-node-id */
    lsInt_t     parent; /* parent-node-id, indexed by child-node-id */
} btreeInt_t;

btreeInt_t *btreeInt_nil(btreeInt_t *t);
btreeInt_t *btreeInt_Nil(void);

void btreeInt_free(btreeInt_t *t);
void btreeInt_Free(btreeInt_t *t);

btreeInt_t *btreeInt_setNil(btreeInt_t *t);

btreeInt_t *btreeInt_cpy(btreeInt_t *copy, const btreeInt_t *t);
btreeInt_t *btreeInt_Cpy(const btreeInt_t *t);
btreeInt_t *btreeInt_realloc(btreeInt_t *t, int max_nodes);

int      btreeInt_newParent(btreeInt_t *t, int i, int node1, int node2);
int      btreeInt_newNode(btreeInt_t *t, int i);
btreeInt_t *btreeInt_addParent(btreeInt_t *t, int i, int node1, int node2);
btreeInt_t *btreeInt_addNode(btreeInt_t *t, int i);






