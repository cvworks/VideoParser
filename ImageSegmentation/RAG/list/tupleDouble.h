
#ifndef TUPLE_Double_INCLUDED
#define TUPLE_Double_INCLUDED

#define TUPLE_IS_Double

typedef struct pairDouble_t {
  double first;
  double second;
} pairDouble_t;

typedef double (tupleDouble_map_t)(double);

extern pairDouble_t pairDoubleNULL;

pairDouble_t * pairDouble(double fst, double snd);
/* allociert ein pairDouble_t und weist <fst> und <snd> zu */
pairDouble_t * pairDouble_set(pairDouble_t *p, double fst, double snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     pairDouble_destroy(pairDouble_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pairDouble_t * pairDouble_cpy(pairDouble_t *p_to, pairDouble_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairDouble_t * pairDouble_Cpy(pairDouble_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairDouble_t * pairDouble_map(pairDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pairDouble_t * pairDouble_Map(pairDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2);
/* allociert neuen pairDouble_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pairDouble_t * pairDouble_mapSet(pairDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2);
/* wie pairDouble_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

double pairDouble_getFst(pairDouble_t *p, double undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
double pairDouble_getSnd(pairDouble_t *p, double undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
double pairDouble_setFst(pairDouble_t *p, double fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
double pairDouble_setSnd(pairDouble_t *p, double snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void pairDouble_destroyFunc(pairDouble_t *p, tupleDouble_map_t *free1,tupleDouble_map_t *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pairDouble_t frei */
#endif

typedef struct tripleDouble_t {
  double first;
  double second;
  double third;
} tripleDouble_t;

extern tripleDouble_t tripleDoubleNULL;

tripleDouble_t * tripleDouble(double fst, double snd, double trd);
/* allociert ein pairDouble_t und weist <fst>, <snd> und <trd> zu */
tripleDouble_t * tripleDouble_set(tripleDouble_t *p, double fst, double snd, 
		       double trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       tripleDouble_destroy(tripleDouble_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
tripleDouble_t * tripleDouble_cpy(tripleDouble_t *p_to, tripleDouble_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleDouble_t * tripleDouble_Cpy(tripleDouble_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleDouble_t * tripleDouble_map(tripleDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2,
		       tupleDouble_map_t *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
tripleDouble_t * tripleDouble_Map(tripleDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2,
		       tupleDouble_map_t *f3);
/* allociert neuen pairDouble_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
tripleDouble_t * tripleDouble_mapSet(tripleDouble_t *p, tupleDouble_map_t *f1, tupleDouble_map_t *f2,
			  tupleDouble_map_t *f3);
/* wie pairDouble_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

double pairDouble_getFst(pairDouble_t *p, double undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
double pairDouble_getSnd(pairDouble_t *p, double undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
double pairDouble_getTrd(pairDouble_t *p, double undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
double tripleDouble_setFst(tripleDouble_t *p, double fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
double tripleDouble_setSnd(tripleDouble_t *p, double snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
double tripleDouble_setTrd(tripleDouble_t *p, double trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void tripleDouble_destroyFunc(tripleDouble_t *p, tupleDouble_map_t *free1,
			 tupleDouble_map_t *free2, tupleDouble_map_t *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den tripleDouble_t frei */
#endif

#undef TUPLE_IS_Double

#endif






