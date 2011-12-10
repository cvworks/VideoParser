
#ifndef TUPLE_Int_INCLUDED
#define TUPLE_Int_INCLUDED

#define TUPLE_IS_Int

typedef struct pairInt_t {
  int first;
  int second;
} pairInt_t;

typedef int (tupleInt_map_t)(int);

extern pairInt_t pairIntNULL;

pairInt_t * pairInt(int fst, int snd);
/* allociert ein pairInt_t und weist <fst> und <snd> zu */
pairInt_t * pairInt_set(pairInt_t *p, int fst, int snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     pairInt_destroy(pairInt_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pairInt_t * pairInt_cpy(pairInt_t *p_to, pairInt_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairInt_t * pairInt_Cpy(pairInt_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairInt_t * pairInt_map(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pairInt_t * pairInt_Map(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2);
/* allociert neuen pairInt_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pairInt_t * pairInt_mapSet(pairInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2);
/* wie pairInt_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

int pairInt_getFst(pairInt_t *p, int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
int pairInt_getSnd(pairInt_t *p, int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
int pairInt_setFst(pairInt_t *p, int fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
int pairInt_setSnd(pairInt_t *p, int snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void pairInt_destroyFunc(pairInt_t *p, tupleInt_map_t *free1,tupleInt_map_t *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pairInt_t frei */
#endif

typedef struct tripleInt_t {
  int first;
  int second;
  int third;
} tripleInt_t;

extern tripleInt_t tripleIntNULL;

tripleInt_t * tripleInt(int fst, int snd, int trd);
/* allociert ein pairInt_t und weist <fst>, <snd> und <trd> zu */
tripleInt_t * tripleInt_set(tripleInt_t *p, int fst, int snd, 
		       int trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       tripleInt_destroy(tripleInt_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
tripleInt_t * tripleInt_cpy(tripleInt_t *p_to, tripleInt_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleInt_t * tripleInt_Cpy(tripleInt_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleInt_t * tripleInt_map(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
		       tupleInt_map_t *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
tripleInt_t * tripleInt_Map(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
		       tupleInt_map_t *f3);
/* allociert neuen pairInt_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
tripleInt_t * tripleInt_mapSet(tripleInt_t *p, tupleInt_map_t *f1, tupleInt_map_t *f2,
			  tupleInt_map_t *f3);
/* wie pairInt_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

int pairInt_getFst(pairInt_t *p, int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
int pairInt_getSnd(pairInt_t *p, int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
int pairInt_getTrd(pairInt_t *p, int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
int tripleInt_setFst(tripleInt_t *p, int fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
int tripleInt_setSnd(tripleInt_t *p, int snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
int tripleInt_setTrd(tripleInt_t *p, int trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void tripleInt_destroyFunc(tripleInt_t *p, tupleInt_map_t *free1,
			 tupleInt_map_t *free2, tupleInt_map_t *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den tripleInt_t frei */
#endif

#undef TUPLE_IS_Int

#endif






