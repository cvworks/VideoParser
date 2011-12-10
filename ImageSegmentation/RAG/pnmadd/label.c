/*
 * pnmadd/label.c
 *
 * Sven Wachsmuth, 7.10.2004
 */
#include <stdlib.h>
#include "label.h"


int *pnm_label_createLookup(int *labelMax, int n_labels, const int *labels)
{
    int i, *labelFlags = NULL;
    if (!labelMax || !labels) return (NULL);
    
    *labelMax = -1;

    /* generate lookup-table for labels */
    for (i=0; i < n_labels; i++)
	if (labels[i] > (*labelMax)) (*labelMax) = labels[i];
    if ((*labelMax) < 0) return (NULL);

    labelFlags = (int *) calloc((*labelMax)+1,sizeof(int));
    for (i=0; i < n_labels; i++)
	labelFlags[labels[i]] = 1;

    return (labelFlags);
}

int pnm_label_lookup(int labelMax, const int *labelFlags, int label)
{
    return (label > 0 && label <= labelMax) ? labelFlags[label] : 0;
}
