
#ifndef TUPLE_Pt_INCLUDED
#define TUPLE_Pt_INCLUDED

#define TUPLE_IS_Pt

typedef struct pairPt_t {
  void * first;
  void * second;
} pairPt_t;

typedef void * (tuplePt_map_t)(void *);

extern pairPt_t pairPtNULL;

pairPt_t * pairPt(void * fst, void * snd);
/* allociert ein pairPt_t und weist <fst> und <snd> zu */
pairPt_t * pairPt_set(pairPt_t *p, void * fst, void * snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     pairPt_destroy(pairPt_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pairPt_t * pairPt_cpy(pairPt_t *p_to, pairPt_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairPt_t * pairPt_Cpy(pairPt_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairPt_t * pairPt_map(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pairPt_t * pairPt_Map(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2);
/* allociert neuen pairPt_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pairPt_t * pairPt_mapSet(pairPt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2);
/* wie pairPt_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

void * pairPt_getFst(pairPt_t *p, void * undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
void * pairPt_getSnd(pairPt_t *p, void * undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
void * pairPt_setFst(pairPt_t *p, void * fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
void * pairPt_setSnd(pairPt_t *p, void * snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void pairPt_destroyFunc(pairPt_t *p, tuplePt_map_t *free1,tuplePt_map_t *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pairPt_t frei */
#endif

typedef struct triplePt_t {
  void * first;
  void * second;
  void * third;
} triplePt_t;

extern triplePt_t triplePtNULL;

triplePt_t * triplePt(void * fst, void * snd, void * trd);
/* allociert ein pairPt_t und weist <fst>, <snd> und <trd> zu */
triplePt_t * triplePt_set(triplePt_t *p, void * fst, void * snd, 
		       void * trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       triplePt_destroy(triplePt_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
triplePt_t * triplePt_cpy(triplePt_t *p_to, triplePt_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
triplePt_t * triplePt_Cpy(triplePt_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
triplePt_t * triplePt_map(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
		       tuplePt_map_t *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
triplePt_t * triplePt_Map(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
		       tuplePt_map_t *f3);
/* allociert neuen pairPt_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
triplePt_t * triplePt_mapSet(triplePt_t *p, tuplePt_map_t *f1, tuplePt_map_t *f2,
			  tuplePt_map_t *f3);
/* wie pairPt_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

void * pairPt_getFst(pairPt_t *p, void * undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
void * pairPt_getSnd(pairPt_t *p, void * undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
void * pairPt_getTrd(pairPt_t *p, void * undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
void * triplePt_setFst(triplePt_t *p, void * fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
void * triplePt_setSnd(triplePt_t *p, void * snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
void * triplePt_setTrd(triplePt_t *p, void * trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void triplePt_destroyFunc(triplePt_t *p, tuplePt_map_t *free1,
			 tuplePt_map_t *free2, tuplePt_map_t *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den triplePt_t frei */
#endif

#undef TUPLE_IS_Pt

#endif






