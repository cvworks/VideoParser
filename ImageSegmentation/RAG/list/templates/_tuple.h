
#ifndef TUPLE_TYPE_INCLUDED
#define TUPLE_TYPE_INCLUDED

#define TUPLE_IS_TYPE

typedef struct pair_t {
  TUPLE_TYPE first;
  TUPLE_TYPE second;
} pair_t;

typedef TUPLE_TYPE (TUPLE_MAP_TYPE)(TUPLE_TYPE);

extern pair_t _pairNULL;

pair_t * _pair(TUPLE_TYPE fst, TUPLE_TYPE snd);
/* allociert ein pair_t und weist <fst> und <snd> zu */
pair_t * _pair_set(pair_t *p, TUPLE_TYPE fst, TUPLE_TYPE snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     _pair_destroy(pair_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pair_t * _pair_cpy(pair_t *p_to, pair_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pair_t * _pair_Cpy(pair_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pair_t * _pair_map(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pair_t * _pair_Map(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2);
/* allociert neuen pair_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pair_t * _pair_mapSet(pair_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2);
/* wie _pair_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

TUPLE_TYPE _pair_getFst(pair_t *p, TUPLE_TYPE undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
TUPLE_TYPE _pair_getSnd(pair_t *p, TUPLE_TYPE undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
TUPLE_TYPE _pair_setFst(pair_t *p, TUPLE_TYPE fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
TUPLE_TYPE _pair_setSnd(pair_t *p, TUPLE_TYPE snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void _pair_destroyFunc(pair_t *p, TUPLE_MAP_TYPE *free1,TUPLE_MAP_TYPE *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pair_t frei */
#endif

typedef struct triple_t {
  TUPLE_TYPE first;
  TUPLE_TYPE second;
  TUPLE_TYPE third;
} triple_t;

extern triple_t _tripleNULL;

triple_t * _triple(TUPLE_TYPE fst, TUPLE_TYPE snd, TUPLE_TYPE trd);
/* allociert ein pair_t und weist <fst>, <snd> und <trd> zu */
triple_t * _triple_set(triple_t *p, TUPLE_TYPE fst, TUPLE_TYPE snd, 
		       TUPLE_TYPE trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       _triple_destroy(triple_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
triple_t * _triple_cpy(triple_t *p_to, triple_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
triple_t * _triple_Cpy(triple_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
triple_t * _triple_map(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
		       TUPLE_MAP_TYPE *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
triple_t * _triple_Map(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
		       TUPLE_MAP_TYPE *f3);
/* allociert neuen pair_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
triple_t * _triple_mapSet(triple_t *p, TUPLE_MAP_TYPE *f1, TUPLE_MAP_TYPE *f2,
			  TUPLE_MAP_TYPE *f3);
/* wie _pair_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

TUPLE_TYPE _pair_getFst(pair_t *p, TUPLE_TYPE undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
TUPLE_TYPE _pair_getSnd(pair_t *p, TUPLE_TYPE undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
TUPLE_TYPE _pair_getTrd(pair_t *p, TUPLE_TYPE undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
TUPLE_TYPE _triple_setFst(triple_t *p, TUPLE_TYPE fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
TUPLE_TYPE _triple_setSnd(triple_t *p, TUPLE_TYPE snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
TUPLE_TYPE _triple_setTrd(triple_t *p, TUPLE_TYPE trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void _triple_destroyFunc(triple_t *p, TUPLE_MAP_TYPE *free1,
			 TUPLE_MAP_TYPE *free2, TUPLE_MAP_TYPE *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den triple_t frei */
#endif

#undef TUPLE_IS_TYPE

#endif






