#include <stdlib.h>
#include "random.h"

int ls_randomInt(int min, int max)
{
  return min + (int) ((float) (max-min+1)*rand() / (RAND_MAX+1.0));
}

lsInt_t *lsInt_setRandom(lsInt_t *list, int min, int max,
			 int Nmin, int Nmax, lsInt_t *Nfrequencies,
			 int exclusive)
{
  int max_random, i_random, i, n_items;
  if (!list)
    list = lsInt_Nil();
  else
    lsInt_setNil(list);

  if (Nfrequencies)
    max_random = lsInt_sum(Nfrequencies);
  else
    max_random = Nmax-Nmin+1;

  /* select size of subset */
  i_random = (int) ((float) max_random*rand() / (RAND_MAX+1.0));

  if (Nfrequencies) {
    n_items = 0;
    LS_FORALL_ITEMS(Nfrequencies,n_items) {
      i_random -= LS_GET(Nfrequencies,n_items);
      if (i_random < 0) break;
    }
    n_items += Nmin;
  } else {
    n_items = i_random + Nmin;
  }
  lsInt_realloc(list, n_items);

  if (!exclusive) {
    for (i=0; i < n_items; i++) {
      i_random = (int) ((float) (max-min+1)*rand() / (RAND_MAX+1.0));
      lsInt_add(list,i_random+min);
    }
  } else {
    int n = max-min+1;
    for (i=0; i < n_items; i++,n--) {
      i_random = (int) ((float) n*rand() / (RAND_MAX+1.0));
      {
	int j,offset = 0;
	LS_FORALL_ITEMS(list,j) {
	  if (LS_GET(list,j) == i_random+min) break;
	  if (LS_GET(list,j) < n+min) offset++;
	}
	if (j < LS_N(list)) i_random = n+offset;
      }
      lsInt_add(list,i_random + min);
    }
  }
  return (list);
} 
