
#ifndef TUPLE_Float_INCLUDED
#define TUPLE_Float_INCLUDED

#define TUPLE_IS_Float

typedef struct pairFloat_t {
  float first;
  float second;
} pairFloat_t;

typedef float (tupleFloat_map_t)(float);

extern pairFloat_t pairFloatNULL;

pairFloat_t * pairFloat(float fst, float snd);
/* allociert ein pairFloat_t und weist <fst> und <snd> zu */
pairFloat_t * pairFloat_set(pairFloat_t *p, float fst, float snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     pairFloat_destroy(pairFloat_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pairFloat_t * pairFloat_cpy(pairFloat_t *p_to, pairFloat_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairFloat_t * pairFloat_Cpy(pairFloat_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairFloat_t * pairFloat_map(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pairFloat_t * pairFloat_Map(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2);
/* allociert neuen pairFloat_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pairFloat_t * pairFloat_mapSet(pairFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2);
/* wie pairFloat_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

float pairFloat_getFst(pairFloat_t *p, float undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
float pairFloat_getSnd(pairFloat_t *p, float undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
float pairFloat_setFst(pairFloat_t *p, float fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
float pairFloat_setSnd(pairFloat_t *p, float snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void pairFloat_destroyFunc(pairFloat_t *p, tupleFloat_map_t *free1,tupleFloat_map_t *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pairFloat_t frei */
#endif

typedef struct tripleFloat_t {
  float first;
  float second;
  float third;
} tripleFloat_t;

extern tripleFloat_t tripleFloatNULL;

tripleFloat_t * tripleFloat(float fst, float snd, float trd);
/* allociert ein pairFloat_t und weist <fst>, <snd> und <trd> zu */
tripleFloat_t * tripleFloat_set(tripleFloat_t *p, float fst, float snd, 
		       float trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       tripleFloat_destroy(tripleFloat_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
tripleFloat_t * tripleFloat_cpy(tripleFloat_t *p_to, tripleFloat_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleFloat_t * tripleFloat_Cpy(tripleFloat_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleFloat_t * tripleFloat_map(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
		       tupleFloat_map_t *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
tripleFloat_t * tripleFloat_Map(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
		       tupleFloat_map_t *f3);
/* allociert neuen pairFloat_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
tripleFloat_t * tripleFloat_mapSet(tripleFloat_t *p, tupleFloat_map_t *f1, tupleFloat_map_t *f2,
			  tupleFloat_map_t *f3);
/* wie pairFloat_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

float pairFloat_getFst(pairFloat_t *p, float undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
float pairFloat_getSnd(pairFloat_t *p, float undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
float pairFloat_getTrd(pairFloat_t *p, float undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
float tripleFloat_setFst(tripleFloat_t *p, float fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
float tripleFloat_setSnd(tripleFloat_t *p, float snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
float tripleFloat_setTrd(tripleFloat_t *p, float trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void tripleFloat_destroyFunc(tripleFloat_t *p, tupleFloat_map_t *free1,
			 tupleFloat_map_t *free2, tupleFloat_map_t *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den tripleFloat_t frei */
#endif

#undef TUPLE_IS_Float

#endif






