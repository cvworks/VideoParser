/*
 * iterate.h
 *
 * Iterierungsfunktionen fuer Indexkombinationen
 *
 * Sven Wachsmuth, 27.03.00
 */
#ifndef _LS_ITERATE_H
#define _LS_ITERATE_H

#include "list.h"

typedef void *(ls_fold_t)(void *,lsInt_t *,void *);

typedef struct {
  int       *min;
  int       *max;
  int        n;
  ls_fold_t *func;
  void      *args;
  int       *permMin;
  int       *permMax;
  int       *perm;
  lsInt_t   *val;
} ls_exclArgs_t;

#define ls_permFold(result,n,func,arg) _ls_permFold(result,0,n,func,arg)
void *_ls_permFold(void *result, int k, int n, ls_fold_t *func, void *arg);
/* ruft die Funktion result = (*func)(result,index,arg) fuer alle
 * Permutationen der Zahlen 1...n auf (index=[1,2,...,n],[2,1,...,n],...) 
 */ 
void *ls_ordFold(void *result, int *min, int *max, int n, ls_fold_t *func, 
		 void *arg);
/* ruft die Funktion result = (*func)(result,index,arg) fuer alle geordneten
 * n-Tupel auf (index=[0,1,2,...],[0,1,3,...],[0,2,3,...],[1,2,3,...],...)
 * Die einzelnen Werte an Stelle i laufen dabei jeweils von min[i]..max[i] 
 * (2-dimensional wird z.B. die obere Dreiecksmatrix abgelaufen) 
 */ 
void *ls_exclFold(void *result, int *min, int *max, int n, ls_fold_t *func, 
		  void *args);
/* ruft die Funktion result = (*func)(result,index,arg) fuer alle n-Tupel
 * auf, bei denen keine zwei Stellen denselben Wert haben.
 * (index=[0,1,2...],[0,1,3...],[0,2,3...],[0,3,2...],[1,2,3...],[2,1,3...]...)
 */

#endif
