/*
 * iterate.h
 *
 * Iterierungsfunktionen fuer Indexkombinationen
 *
 * Sven Wachsmuth, 27.03.00
 */
#include <rs/memory.h>
#include "iterate.h"

void *_ls_permFold(void *result, int k, int n, 
		   ls_fold_t *func, void *arg)
{
  static lsInt_t *_val = NULL;
  static int *val = NULL;
  static int id;
  int t;
  
  /** Falls Start ... */
  if (k == 0) {
    /** ... initialisiere Index-Werte mit 0 */
    if (_val) 
      lsInt_setNil(_val);
    else
      _val = lsInt_Nil();
    lsInt_setIndex(_val,n,0,0);
    val = LS_ITEMS(_val);
    /** ... initialisiere neuen Index-Wert */
    id = -1;
  }
  /** Setze neuen Index-Wert */
  val[k] = ++id;
  /** Falls letzten Index-Wert verteilt, Funktionsaufruf ... */
  if (id == n) result = (*func)(result, _val, arg);
  /** Sonst suche noch nicht belegte Index-Stelle ... */
  else for (t=1; t <= n; t++)
    /** ... und belege diese Stelle mit dem naechsten Index-Wert */
    if (val[t] == 0) result = _ls_permFold(result,t,n,func,arg);
  /** Setze naechsten Index-Wert zurueck ...
      ... und definiere Index-Stelle als unbelegt */
  id--; val[k] = 0;

  return (result);
}

void *ls_ordFold(void *result, int *min, int *max, int n, 
		 ls_fold_t *func, void *arg)
{
  static lsInt_t *_val = NULL;
  int *val;
  int i;
  int x = min[0] - 1;
  _val = lsInt_realloc(_val,n);
  val  = LS_ITEMS(_val);

  /** Initialisiere Index-Belegung <val> mit aufsteigend geordneten Werten */
  for (i=0; i < n; i++) {
    val[i] = (x < min[i]) ? (x=min[i]) : (++x);
    /** ... Falls ausserhalb von Wertebereich, Abbruch */
    if (val[i] > max[i])
      return (result);
  }
  /** Wiederhole fuer alle geordneten Kombinationen ... */
  do {
    int i,j,x;
    /** ... rufe Funktion auf */
    result = (*func)(result,_val,arg);

    /** ... Suche naechste Index-Kombination */
    do {
      /** - Erhoehe Index-Stelle <i>=n-1..0 bis ...
	  ... aktuelle Stelle <i> ein erlaubter Werte ist */ 
      for (i=n-1; i >= 0 && ((++val[i]) > max[i]); i--);
      /** - Falls keine weitere erlaubte Kombination, Abbruch */
      if (i < 0) return (result);
      /** - Initialisiere die Index-Stellen i+1..n-1 mit aufsteigend ...
	  ... sortierten Werten */
      x = val[i];
      for (j=i+1; j < n; j++) {
	val[j] = (x < min[j]) ? (x=min[j]) : (++x);
	/** ... Falls kein erlaubter Wert vorhanden, ...
	    ... neue Kombination probieren (j < n) */
	if (val[j] > max[j])
	  break;
      }
    } while (j < n);
  } while (1);
  return (result);
}

void *__ls_excl(void *result, lsInt_t *val, ls_exclArgs_t *args)
{
  /* Hilfsfunktion fuer ls_exclFold:
   * nimmt vorher durchgefuehrte Index-Permutation 
   * fuer Funktionsaufruf zurueck */
  int *perm = args->perm;
  int i;
  for (i=0; i < args->n; i++)
    LS_GET(args->val,i) = LS_GET(val,perm[i+1]-1);
  LS_N(args->val) = args->n;
  
  return (*args->func)(result,args->val,args->args);
}
  
void *_ls_excl(void *result, lsInt_t *val, ls_exclArgs_t *args)
{
  /* Hilfsfunktion fuer ls_exclFold:
   * Iteriert alle geordneten Wertkombinationen 
   * fuer die aktuell permutierten Index-Stellen */
  int i;

  args->perm = LS_ITEMS(val);

  for (i = 1; i <= args->n; i++) {
    args->permMin[LS_GET(val,i)-1] = args->min[i-1];
    args->permMax[LS_GET(val,i)-1] = args->max[i-1];
  }
  return (ls_ordFold(result, args->permMin, args->permMax, args->n, 
		     (ls_fold_t *) __ls_excl, args));
}

void *ls_exclFold(void *result, int *min, int *max, int n, 
		  ls_fold_t *func, void *args)
{
  ls_exclArgs_t _args;
  _args.min  = min;
  _args.max  = max;
  _args.n    = n;
  _args.func = func;
  _args.args = args;
  _args.permMin = rs_malloc(n * sizeof(int),"permMin");
  _args.permMax = rs_malloc(n * sizeof(int),"permMax");
  _args.val     = lsInt_realloc(NULL,n);
  _args.perm    = NULL;

  result = ls_permFold(result, n, (ls_fold_t *) _ls_excl, &_args);

  rs_free(_args.permMin);
  rs_free(_args.permMax);
  rs_free(_args.val);

  return (result);
}

