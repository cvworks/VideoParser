#ifndef _LIST_RANDOM_H
#define _LIST_RANDOM_H

#include "list.h"

int ls_randomInt(int min, int max);

lsInt_t *lsInt_setRandom(lsInt_t *list, int min, int max,
			 int Nmin, int Nmax, lsInt_t *Nfrequencies,
			 int exclusive);

#endif
