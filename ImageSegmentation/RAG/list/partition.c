/*
 * partition.c
 *
 * Klasse zur Partitionierung von Vektoren
 *
 * Sven Wachsmuth, 5.3.99
 * Sven Wachsmuth, 28.2.01, Adaptierung auf liblist.a
 */
#include <stdio.h>
#include <rs/memory.h>
#include <rs/messages.h>
#include "partition.h"

lsPart_t *lsPart_create(void)
{
  return (lsPart_init(NULL));
}

lsPart_t *lsPart_init(lsPart_t *p)
{
  if (!p) {
    p = rs_malloc(sizeof(lsPart_t *),"lsPart_t");
  }
  lsDouble_nil(&p->orig);
  lsDouble_nil(&p->sorted);
  lsInt_nil(&p->sorted2orig);
  lsInt_nil(&p->orig2sorted);
  lsPt_nil(&p->parts);

  return (p);
}
void lsPart_free(lsPart_t *p)
{
  if (!p) return;
  lsDouble_free(&p->orig);
  lsDouble_free(&p->sorted);
  lsInt_free(&p->sorted2orig);
  lsInt_free(&p->orig2sorted);
  lsPt_map(&p->parts, (lsPt_map_t *) lsInt_free);
  lsPt_free(&p->parts);
}

void lsPart_destroy(lsPart_t *p)
{
  if (!p) return;
  lsPart_free(p);
  rs_free(p);
}

lsPart_t *lsPart_reinit(lsPart_t *p)
{
  if (!p) return (NULL);
  lsDouble_setNil(&p->sorted);
  lsInt_setNil(&p->sorted2orig);
  lsInt_setNil(&p->orig2sorted);
  lsPt_map(&p->parts, (lsPt_map_t *) lsInt_free);
  lsPt_setNil(&p->parts);
  return (p);
}

double *lsPart_set(lsPart_t *p, int n, double *v)
{
  double *_items;
  if (!p)
      rs_error("lsPart_set: try to set a null-pointer partition.");
  _items = LS_ITEMS(&p->orig);
  LS_ITEMS(&p->orig) = NULL;
  lsDouble_setPt(&p->orig,n,v);
  lsPart_reinit(p);

  return (_items);
}

lsPart_t *lsPart_Set(lsPart_t *p, int n, double *v)
{
  lsDouble_t _v;
  lsDouble_nil(&_v);
  if (!p)
      p = lsPart_create();
  lsDouble_cpy(&p->orig, lsDouble_setPt(&_v, n, v));
  lsPart_reinit(p);
  return (p);
}

lsPart_t *lsPart_eval(lsPart_t *p, double base)
{
  lsPart_t p_delta;
  lsDouble_t _delta;
  
  if (!p || LS_isNIL(&p->orig)) return (p);

  lsPart_delta(lsDouble_nil(&_delta),p,base,1);
  if (LS_isNIL(&_delta)) return (p);
  
  lsPart_Set(lsPart_init(&p_delta), LS_N(&_delta), LS_ITEMS(&_delta));
  lsPart_delta(lsDouble_setNil(&_delta),&p_delta,1.0,0);
  { /** Partitionierung */
    int i_max = lsDouble_maxIndex(&_delta);
    lsInt_t *is_max;
    lsPart_split(&p_delta,i_max);
    is_max = lsPart_getMaxGrp(&p_delta);
    lsPart_nsplit(p, is_max);
  }
  return (p);
}

lsPart_t *lsPart_sort(lsPart_t *p)
{
  if (!p) return (NULL);
  lsDouble_qsortIndexLt(&p->sorted2orig, &p->orig);
  lsDouble_SortByIndex(&p->sorted, &p->orig, &p->sorted2orig, 0.0);
  lsInt_makeIndex(&p->orig2sorted,&p->sorted2orig);
  return (p);
}

lsDouble_t *lsPart_delta(lsDouble_t *d, lsPart_t *p, double base, int gteq_flag)
{
  lsDouble_t _p, _d;
  double _base;
  lsDouble_nil(&_p); lsDouble_nil(&_d);
  lsDouble_setNil(d);

  if (!p || LS_isNIL(&p->orig)) return (d);
  d = lsDouble_realloc(d, LS_N(&p->orig));
  if (LS_isNIL(&p->sorted)) lsPart_sort(p);
  
  if (gteq_flag) { /** Anfangsnullen ueberspringen */
    int i = lsDouble_getFstNeqIndex(&p->sorted,0.0);
    if (i > 0)
	lsDouble_SetIndex(d,i-1,0.0,0.0);
    if (i == LS_N(&p->sorted))
	return (d);

    /** <base>-Werte relativ zum niedrigsten neq-Element berechnen */
    _base = base * LS_GET(&p->sorted,i);

    /** Null-Eintraege abschneiden 
     *  [ACHTUNG: Listen duerfen NICHT reallociert werden] */
    lsDouble_setPt(&_p, LS_N(&p->sorted)-i, LS_ITEMS_DROP(&p->sorted,i));
    lsDouble_setPt(&_d, LS_N(&p->sorted)-i, LS_ITEMS_DROP(d,i));
  } else {
    _base = base * LS_GET(&p->sorted,0);
    lsDouble_cpyPt(&_p, &p->sorted);
    lsDouble_cpyPt(&_d, d);
  }
  lsDouble_delta(&_d,&_p,_base);
  /** korrekte Listenlaenge eintragen */
  LS_N(d) = LS_N(&p->sorted);

  return (d);
}

lsPart_t *lsPart_split(lsPart_t *p, int i)
{
  lsInt_t *is, *js;
  if (!p) return (p);
  is = lsInt_Cpy(&p->sorted2orig);
  js = lsInt_split(is,i);
  lsPt_Cons(lsPt_Cons(lsPt_setNil(&p->parts),is),js);
  return (p);
}

lsPart_t *lsPart_nsplit(lsPart_t *p, lsInt_t *is)
{
  lsInt_t *js;
  if (!p) return (p);
  js = lsInt_Cpy(&p->sorted2orig);
  lsInt_nsplit(&p->parts,js,is);
  return (p);
}

lsInt_t *lsPart_getMaxGrp(lsPart_t *p)
{
  if (!p) return (NULL);
  return (LS_LAST_CHECK(&p->parts,NULL));
}

void lsPart_fprint(FILE *fp, lsPart_t *p)
{
  char *s = NULL;
  fprintf(fp,"lsPart {\n");
  if (p) {
    int i;
    fprintf(fp,"\torig:\t%s.\n",s=lsDouble_sprint_chr(s,&p->orig,' '));
    fprintf(fp,"\tsorted:\t%s.\n",s=lsDouble_sprint_chr(s,&p->sorted,' '));
    fprintf(fp,"\torig2sorted:\t%s.\n",
	    s=lsInt_sprint_chr(s,&p->orig2sorted,' '));
    fprintf(fp,"\tsorted2orig:\t%s.\n",
	    s=lsInt_sprint_chr(s,&p->sorted2orig,' '));
    LS_FORALL_ITEMS(&p->parts,i) {
      lsInt_t *is = LS_GET(&p->parts,i);
      fprintf(fp,"\tpart[%d]:\t%s.\n",i,lsInt_sprint_chr(s,is,' '));
    }
  }
  fprintf(fp,"}\n");
}
    
