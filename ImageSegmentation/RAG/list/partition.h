/*
 * partition.h
 *
 * Klasse zur Partitionierung von Vektoren
 *
 * Sven Wachsmuth, 5.3.99
 * Sven Wachsmuth, 28.2.01, Adaptierung auf liblist.a
 */
#ifndef _LS_PARTITION_H
#define _LS_PARTITION_H

#include "list.h"

typedef struct lsPart_t {
    lsDouble_t orig;
    lsDouble_t sorted;
    lsInt_t   sorted2orig;
    lsInt_t   orig2sorted;
    lsPt_t    parts;
} lsPart_t;

lsPart_t *lsPart_create(void);
lsPart_t *lsPart_init(lsPart_t *p);
void      lsPart_free(lsPart_t *p);
void      lsPart_destroy(lsPart_t *p);

double    *lsPart_set(lsPart_t *p, int n, double *v);
lsPart_t *lsPart_Set(lsPart_t *p, int n, double *v);

lsInt_t *lsPart_getMaxGrp(lsPart_t *p);

lsDouble_t *lsPart_delta(lsDouble_t *d, lsPart_t *p, double base, int gteq_flag);

lsPart_t *lsPart_sort(lsPart_t *p);
lsPart_t *lsPart_split(lsPart_t *p, int i);
lsPart_t *lsPart_nsplit(lsPart_t *p, lsInt_t *is);
lsPart_t *lsPart_eval(lsPart_t *p, double base);

void lsPart_fprint(FILE *fp, lsPart_t *p);

#endif

