
#ifndef TUPLE_Long_INCLUDED
#define TUPLE_Long_INCLUDED

#define TUPLE_IS_Long

typedef struct pairLong_t {
  long int first;
  long int second;
} pairLong_t;

typedef long int (tupleLong_map_t)(long int);

extern pairLong_t pairLongNULL;

pairLong_t * pairLong(long int fst, long int snd);
/* allociert ein pairLong_t und weist <fst> und <snd> zu */
pairLong_t * pairLong_set(pairLong_t *p, long int fst, long int snd);
/* weist <fst> und <snd> zu. Falls <p> Nullpointer, wird <p> allociert */
void     pairLong_destroy(pairLong_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
pairLong_t * pairLong_cpy(pairLong_t *p_to, pairLong_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairLong_t * pairLong_Cpy(pairLong_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
pairLong_t * pairLong_map(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2);
/* wendet Funktion <f1>(p->first) und <f2>(p->second) an, soweit
 * die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
pairLong_t * pairLong_Map(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2);
/* allociert neuen pairLong_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
pairLong_t * pairLong_mapSet(pairLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2);
/* wie pairLong_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

long int pairLong_getFst(pairLong_t *p, long int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
long int pairLong_getSnd(pairLong_t *p, long int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
long int pairLong_setFst(pairLong_t *p, long int fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
long int pairLong_setSnd(pairLong_t *p, long int snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */

#if defined(TUPLE_IS_Pt)
void pairLong_destroyFunc(pairLong_t *p, tupleLong_map_t *free1,tupleLong_map_t *free2);
/* ruft die Funktionen <free1>, <free2> zur Freigabe der Tupel-Eintraege auf
 * und gibt dann den pairLong_t frei */
#endif

typedef struct tripleLong_t {
  long int first;
  long int second;
  long int third;
} tripleLong_t;

extern tripleLong_t tripleLongNULL;

tripleLong_t * tripleLong(long int fst, long int snd, long int trd);
/* allociert ein pairLong_t und weist <fst>, <snd> und <trd> zu */
tripleLong_t * tripleLong_set(tripleLong_t *p, long int fst, long int snd, 
		       long int trd);
/* weist <fst>, <snd> und <trd> zu. Falls <p> Nullpointer, wird <p> 
 * allociert */
void       tripleLong_destroy(tripleLong_t *p);
/* gibt Speicherbereich von <p> frei.
 * [ACHTUNG: keine Freigabe von Pointerwerten] */ 
tripleLong_t * tripleLong_cpy(tripleLong_t *p_to, tripleLong_t *p_from);
/* kopiert Inhalt von p_from nach p_to. Falls <p_from> Nullpointer,
 * keine Aenderung von <p_to>. Falls <p_to> Nullpointer,
 * wird <p_to> allociert.
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleLong_t * tripleLong_Cpy(tripleLong_t *p);
/* dupliziert <p>. Falls <p> Nullpointer, wird NULL zurueckgegeben
 * [ACHTUNG: Pointer werden als Pointer kopiert] */
tripleLong_t * tripleLong_map(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
		       tupleLong_map_t *f3);
/* wendet Funktion <f1>(p->first), <f2>(p->second) und <f3>(p->third) an, 
 * soweit die Funktionspointer nicht NULL sind.
 * [ACHTUNG: keine Zuweisung der Funktionsresultate, macht nur Sinn bei 
 *           Pointerwerten] */
tripleLong_t * tripleLong_Map(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
		       tupleLong_map_t *f3);
/* allociert neuen pairLong_t mit den Funktionsresultaten als Werten.
 * Falls Funktionspointer NULL ist, wird der Tupel-Eintrag einfach
 * uebernommen. */
tripleLong_t * tripleLong_mapSet(tripleLong_t *p, tupleLong_map_t *f1, tupleLong_map_t *f2,
			  tupleLong_map_t *f3);
/* wie pairLong_map, weist die Funktionsresultate aber den Tupel-Eintraegen
 * wieder zu. */

long int pairLong_getFst(pairLong_t *p, long int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->first, sonst <undef> */
long int pairLong_getSnd(pairLong_t *p, long int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->second, sonst <undef> */
long int pairLong_getTrd(pairLong_t *p, long int undef);
/* Falls <p> nicht Nullpointer, Rueckgabe von p->third, sonst <undef> */
long int tripleLong_setFst(tripleLong_t *p, long int fst);
/* Setzt den ersten Wert des Tupels auf <fst> und gibt den alten zurueck.
 * Falls <p> Nullpointer, Rueckgabe von <fst>. */
long int tripleLong_setSnd(tripleLong_t *p, long int snd);
/* Setzt den zweiten Wert des Tupels auf <snd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <snd>. */
long int tripleLong_setTrd(tripleLong_t *p, long int trd);
/* Setzt den dritten Wert des Tupels auf <trd> und gibt den alten zurueck
 * Falls <p> Nullpointer, Rueckgabe von <trd>. */

#if defined(TUPLE_IS_Pt)
void tripleLong_destroyFunc(tripleLong_t *p, tupleLong_map_t *free1,
			 tupleLong_map_t *free2, tupleLong_map_t *free3);
/* ruft die Funktionen <free1>, <free2>, <free3> zur Freigabe der 
 * Tupel-Eintraege auf und gibt dann den tripleLong_t frei */
#endif

#undef TUPLE_IS_Long

#endif






