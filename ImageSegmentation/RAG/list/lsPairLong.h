#define LS_IS_lsPairLong
#ifndef LS_lsPairLong_INCLUDED
#define LS_lsPairLong_INCLUDED

#include <stdio.h>

#if defined(LS_IS_lsPairFloat)
#include "tupleFloat.h"
#endif
#if defined(LS_IS_lsPairInt) || defined(LS_IS_lsTripleInt)
#include "tupleInt.h"
#endif
#if defined(LS_IS_lsPairLong) || defined(LS_IS_lsTripleLong)
#include "tupleLong.h"
#endif

#include "tuplePt.h"

typedef struct lsPairLong_t {
  pairLong_t * list;
  int       n_list;
  int     max_list;
} lsPairLong_t;

#if !defined(LS_IS_lsInt)
#include "lsInt.h"
#undef LS_IS_lsInt
#endif

#if !defined(LS_IS_lsPt)
#include "lsPt.h"
#undef LS_IS_lsPt
#endif

#if defined(LS_IS_lsInt) \
    || defined(LS_IS_lsULInt) \
    || defined(LS_IS_lsLInt) \
    || defined(LS_IS_lsChar) \
    || defined(LS_IS_lsFloat) \
    || defined(LS_IS_lsDouble) \
    || defined(LS_IS_lsPt)
#define LS_IS_lvalue
#endif

typedef pairLong_t (lsPairLong_fold_t)(pairLong_t,pairLong_t);
typedef pairLong_t (lsPairLong_fold_2_t)(pairLong_t,pairLong_t,void *);
typedef pairLong_t (lsPairLong_map_t)(pairLong_t);
typedef pairLong_t (lsPairLong_map_2_t)(pairLong_t,void *);
typedef pairLong_t (lsPairLong_map_3_t)(pairLong_t,void *,void *);
typedef int (lsPairLong_filter_t)(pairLong_t);
typedef int (lsPairLong_cmp_t)(pairLong_t,pairLong_t);
typedef int (lsPairLong_cmp_2_t)(pairLong_t,pairLong_t,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsPairLong_t *  lsPairLong_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsPairLong_t *  lsPairLong_realloc(lsPairLong_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsPairLong_Nil neu allociert. */

lsPairLong_t *  lsPairLong_nil(lsPairLong_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsPairLong_t *  lsPairLong_ConsNil(pairLong_t i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsPairLong_t *  lsPairLong_setNil(lsPairLong_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairLong_t *  lsPairLong_setConsNil(lsPairLong_t * il, pairLong_t i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairLong_t *  lsPairLong_SetPt(int n, pairLong_t *items);
lsPairLong_t *  lsPairLong_setPt(lsPairLong_t * il_to, int n, pairLong_t *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairLong_t *  lsPairLong_CpyPt(lsPairLong_t * il_from);
lsPairLong_t *  lsPairLong_cpyPt(lsPairLong_t * il_to, lsPairLong_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairLong_t *  lsPairLong_Cpy(const lsPairLong_t * il_from);
lsPairLong_t *  lsPairLong_cpy(lsPairLong_t * il_to, const lsPairLong_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsPairLong_t *  lsPairLong_Cat(lsPairLong_t * il_1, lsPairLong_t * il_2);
lsPairLong_t *  lsPairLong_cat(lsPairLong_t * il_to, lsPairLong_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsPairLong_t * lsPairLong_addCat(lsPairLong_t *il_to, lsPairLong_t *il);
/* Rueckgabe: lsPairLong_add(il_to, lsPairLong_Cat(il_to,il)) */
lsPairLong_t * lsPairLong_AddCat(lsPairLong_t *il_to, lsPairLong_t *il);
/* Rueckgabe: lsPairLong_Add(il_to, lsPairLong_Cat(il_to,il)) */
#endif

void      lsPairLong_Free(lsPairLong_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsPairLong_free(lsPairLong_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

pairLong_t lsPairLong_setIndex(lsPairLong_t *il, int index, pairLong_t i, pairLong_t i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsPairLong_t * lsPairLong_setIndices(lsPairLong_t *il, lsInt_t *indices, pairLong_t x,
		       pairLong_t undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairLong_setIndex: index < 0] */

lsPairLong_t * lsPairLong_nsetIndex(lsPairLong_t *il, int index, int n, pairLong_t x, 
		      pairLong_t undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairLong_setIndex: index < 0] */

pairLong_t *lsPairLong_getNewItem(lsPairLong_t *il);
int        lsPairLong_getNewItemIndex(lsPairLong_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsPairLong_pushLast(il,i) (lsPairLong_add(il,i))
lsPairLong_t *  lsPairLong_add(lsPairLong_t * il, pairLong_t i);
lsPairLong_t *  lsPairLong_Add(lsPairLong_t * il, pairLong_t i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsPairLong_pushHead(il,i) (lsPairLong_cons(il,i)
lsPairLong_t *  lsPairLong_cons(lsPairLong_t * il, pairLong_t i);
lsPairLong_t *  lsPairLong_Cons(lsPairLong_t * il, pairLong_t i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

pairLong_t lsPairLong_popLast(lsPairLong_t *il, pairLong_t undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairLong_t lsPairLong_popHead(lsPairLong_t *il, pairLong_t undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsPairLong_t *  lsPairLong_init(lsPairLong_t * il);
lsPairLong_t *  lsPairLong_Init(lsPairLong_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairLong_t *  lsPairLong_tail(lsPairLong_t * il);
lsPairLong_t *  lsPairLong_tail(lsPairLong_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairLong_t * lsPairLong_take(lsPairLong_t *il, int n);
lsPairLong_t * lsPairLong_Take(lsPairLong_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairLong_t * lsPairLong_drop(lsPairLong_t *il, int i);
lsPairLong_t * lsPairLong_Drop(lsPairLong_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsPairLong_t * lsPairLong_dropPt(lsPairLong_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsPairLong_t * lsPairLong_split(lsPairLong_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsPairLong_take(<il>,i)
 * - Rueckgabe lsPairLong_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsPairLong_nsplit(lsPt_t *il_split, lsPairLong_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsPairLong_take(<il>,is[0])
 * - Rueckgabe[j] = lsPairLong_Drop(lsPairLong_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsPairLong_t * lsPairLong_range(lsPairLong_t *il, int i0, int iN);
lsPairLong_t * lsPairLong_Range(lsPairLong_t *il, int i0, int iN);
lsPairLong_t * lsPairLong_rangePt(lsPairLong_t *il, int i0, int iN);
lsPairLong_t * lsPairLong_cpyRange(lsPairLong_t *il_to, lsPairLong_t *il, int i0, int iN);
lsPairLong_t * lsPairLong_catRange(lsPairLong_t *il_to, lsPairLong_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsPairLong_t * lsPairLong_insSwap(lsPairLong_t *il, int index, pairLong_t i, pairLong_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

pairLong_t lsPairLong_delSwap(lsPairLong_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairLong_t * lsPairLong_insert(lsPairLong_t *il, int index, pairLong_t i, pairLong_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsPairLong_t * lsPairLong_insertN(lsPairLong_t *il, int index, int n, pairLong_t i, pairLong_t i0);
/* Wie lsPairLong_insert, fuegt aber <n>-mal das Element <i> ein */

pairLong_t lsPairLong_delete(lsPairLong_t *il, int index, pairLong_t undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsPairLong_t * lsPairLong_join(lsPairLong_t *il_to, lsPairLong_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsPairLong_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsPairLong_t * lsPairLong_joinInts(lsPairLong_t *il_to, lsPairLong_t *il_from, lsInt_t *value2index);
/* wie lsPairLong_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsPairLong_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

pairLong_t lsPairLong_last(lsPairLong_t *il, pairLong_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairLong_t lsPairLong_head(lsPairLong_t *il, pairLong_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairLong_t lsPairLong_get(lsPairLong_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

pairLong_t lsPairLong_getCheck(lsPairLong_t * il,int i, pairLong_t undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

pairLong_t lsPairLong_getFlip(int i, lsPairLong_t *il, pairLong_t undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsPairLong_t *  lsPairLong_getRowPt(lsPairLong_t * row, lsPairLong_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsPairLong_t *  lsPairLong_getRow(lsPairLong_t *row, lsPairLong_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsPairLong_t *  lsPairLong_getCol(lsPairLong_t *col, lsPairLong_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsPairLong_t *  lsPairLong_setRow(lsPairLong_t *il, lsPairLong_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsPairLong_t *  lsPairLong_setCol(lsPairLong_t *il, lsPairLong_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsPairLong_length(lsPairLong_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsPairLong_elem(lsPairLong_t * il, pairLong_t i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairLong_neqElem(lsPairLong_t *il, pairLong_t i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairLong_disjoint(lsPairLong_t *il1, lsPairLong_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsPairLong_t *lsPairLong_subst(lsPairLong_t *il, pairLong_t i, pairLong_t j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsPairLong_subBag(lsPairLong_t *il_sub, lsPairLong_t *il_super, pairLong_t undef);
int lsPairLong_subBagIndices(lsInt_t *indices,
		     lsPairLong_t *il_sub, lsPairLong_t *il_super, pairLong_t undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsPairLong_subBagLimitedIndices(lsInt_t *indices,
			    lsPairLong_t *il_sub, lsPairLong_t *il_super, 
			    lsInt_t *limit, pairLong_t undef);
#endif
/* Testet ob sub-bag <il_sub> in super-bag <il_super> enthalten ist.
 * Die Reihenfolge der Elemente spielt dabei keine Rolle, jedoch
 * die Multiplizitaet der Elemente wird beruecksichtigt.
 * <limit> begrenzt den moeglichen Index von Element il_sub[i] auf
 *         0..limit[il_sub[i]]-1
 * Rueckgabe ist -1, falls <il_sub> keine sub-bag von <il_super> ist
 *          und <n>, sonst mit drop(<il_super>,<n>) enthaelt kein Element
 *                   aus <il_sub> 
 * Rueckgabe <indices> ist die sortierte Liste von ausgewaehlten Indices */
#endif

int lsPairLong_getIndex(lsPairLong_t * il, pairLong_t item);
int lsPairLong_getFstIndex(lsPairLong_t *il, pairLong_t item);
int lsPairLong_getLastIndex(lsPairLong_t *il, pairLong_t item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairLong_getLastNeqIndex(lsPairLong_t * il, pairLong_t item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairLong_getFstNeqIndex(lsPairLong_t * il, pairLong_t item);
/* wie lsPairLong_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsPairLong_elemFunc(lsPairLong_t * il, pairLong_t i, lsPairLong_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairLong_neqElemFunc(lsPairLong_t *il, pairLong_t i, lsPairLong_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairLong_getIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsPairLong_getLastNeqIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsPairLong_getFstNeqIndexFunc(lsPairLong_t * il, pairLong_t item, lsPairLong_cmp_t *func);
/* wie lsPairLong_getLastNeqIndex, Suche beginnt hier beim 1. Element */

pairLong_t lsPairLong_foldl(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_t *func);
pairLong_t 
lsPairLong_foldl_2(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsPairLong_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

pairLong_t lsPairLong_foldr(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_t *func);
pairLong_t 
lsPairLong_foldr_2(lsPairLong_t *il, pairLong_t item0, lsPairLong_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsPairLong_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

pairLong_t lsPairLong_maxFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
int       lsPairLong_maxIndexFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairLong_t lsPairLong_minFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
int       lsPairLong_minIndexFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsPairLong_bsearchLtFunc(lsPairLong_t *il, pairLong_t i, lsPairLong_cmp_t *func);
int lsPairLong_bsearchLtFunc_2(lsPairLong_t *il,pairLong_t i,lsPairLong_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairLong_bsearchGtFunc(lsPairLong_t *il, pairLong_t i, lsPairLong_cmp_t *func);
int lsPairLong_bsearchGtFunc_2(lsPairLong_t *il,pairLong_t i,lsPairLong_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairLong_cmpFunc(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsPairLong_cmpIndex(int i, int j, pairPt_t *arg);
int lsPairLong_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsPairLong_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsPairLong_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsPairLong_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsPairLong_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsPairLong_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsPairLong_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
pairLong_t lsPairLong_sum(lsPairLong_t *il);
/* liefert die Summe ueber alle Listenelemente */

pairLong_t lsPairLong_prod(lsPairLong_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsPairLong_t * lsPairLong_scale(lsPairLong_t *il, pairLong_t s);
/* skaliert jedes Element der List um Faktor s */

lsPairLong_t *  lsPairLong_delta(lsPairLong_t *il_to, lsPairLong_t *il_from, pairLong_t base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

pairLong_t lsPairLong_max(lsPairLong_t *il);
int       lsPairLong_maxIndex(lsPairLong_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairLong_t lsPairLong_min(lsPairLong_t *il);
int       lsPairLong_minIndex(lsPairLong_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsPairLong_t *lsPairLong_rmdup(lsPairLong_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsPairLong_bsearchLt(lsPairLong_t *il, pairLong_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairLong_bsearchGt(lsPairLong_t *il, pairLong_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairLong_cmp(lsPairLong_t *il1, lsPairLong_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsPairLong_makeIndex(lsInt_t *index, lsPairLong_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsPairLong_t *lsPairLong_reverse(lsPairLong_t *il);
/* Dreht die Elemente einer Liste um */

lsPairLong_t *  lsPairLong_CpyMap(lsPairLong_t * il_from, lsPairLong_map_t *func);
lsPairLong_t *  lsPairLong_cpyMap(lsPairLong_t * il_to, lsPairLong_t * il_from, lsPairLong_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairLong_t *  lsPairLong_map(lsPairLong_t * il, lsPairLong_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsPairLong_t *  lsPairLong_mapSet(lsPairLong_t * il, lsPairLong_map_t *func);
/* wie lsPairLong_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairLong_t *  lsPairLong_map_2(lsPairLong_t * il, lsPairLong_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsPairLong_t *  lsPairLong_map_3(lsPairLong_t * il, lsPairLong_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsPairLong_t *  lsPairLong_mapSet_2(lsPairLong_t * il, lsPairLong_map_2_t *func, void *arg);
/* wie lsPairLong_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairLong_t *  lsPairLong_mapSet_3(lsPairLong_t * il,lsPairLong_map_3_t *func,void *arg1,void *arg2);
/* wie lsPairLong_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsPairLong_t *  lsPairLong_CpyMap_2(lsPairLong_t * il_from, lsPairLong_map_2_t *func, void *arg);
lsPairLong_t *  lsPairLong_cpyMap_2(lsPairLong_t * il_to, lsPairLong_t * il_from, 
		      lsPairLong_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairLong_t *  lsPairLong_CpyMap_3(lsPairLong_t * il_from, lsPairLong_map_3_t *func, 
		      void *arg1, void *arg2);
lsPairLong_t *  lsPairLong_cpyMap_3(lsPairLong_t * il_to, lsPairLong_t * il_from, 
		      lsPairLong_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairLong_t * lsPairLong_CartProd(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func);
lsPairLong_t * lsPairLong_cartProd(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func);
lsPairLong_t * lsPairLong_cpyCartProd(lsPairLong_t *il_to, 
			lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_t *func);
lsPairLong_t * lsPairLong_cartProd_2(lsPairLong_t *il1, lsPairLong_t *il2, lsPairLong_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsPairLong_CartProd gibt eine neu allocierte List zurueck
 * lsPairLong_cartProd gibt il1 als Ergebnisliste zurueck
 * lsPairLong_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsPairLong_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsPairLong_t * lsPairLong_sortByIndex(lsPairLong_t *il, lsInt_t *index, pairLong_t undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsPairLong_t * lsPairLong_cpySortByIndex(lsPairLong_t *il_to, lsPairLong_t *il_from, 
			   lsInt_t *index, pairLong_t undef);
/* wie lsPairLong_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsPairLong_t * lsPairLong_qsortLtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_qsortGtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_qsortLtFunc_2(lsPairLong_t *il, lsPairLong_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_qsortGtFunc_2(lsPairLong_t *il, lsPairLong_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairLong_qsortIndexLtFunc(lsInt_t *index, lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairLong_qsortIndexGtFunc(lsInt_t *index, lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_msortLtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsPairLong_t * lsPairLong_msortGtFunc(lsPairLong_t *il, lsPairLong_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairLong_t * lsPairLong_qsortLt(lsPairLong_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_qsortGt(lsPairLong_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairLong_qsortIndexLt(lsInt_t *index, lsPairLong_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairLong_qsortIndexGt(lsInt_t *index, lsPairLong_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairLong_t * lsPairLong_msortLt(lsPairLong_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsPairLong_t * lsPairLong_msortGt(lsPairLong_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsPairLong_t * lsPairLong_filterByValue(lsPairLong_t *il, pairLong_t undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsPairLong_t * lsPairLong_CpyFilterByValue(lsPairLong_t *il, pairLong_t undef, lsInt_t *indices);
/* wie lsPairLong_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairLong_t * lsPairLong_cpyFilterByValue(lsPairLong_t *il_to, lsPairLong_t *il_from, pairLong_t undef, 
			     lsInt_t *indices);
/* wie lsPairLong_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsPairLong_t * lsPairLong_filterByIndex(lsPairLong_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsPairLong_t * lsPairLong_CpyFilterByIndex(lsPairLong_t *il, lsInt_t *indices);
/* wie lsPairLong_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairLong_t * lsPairLong_cpyFilterByIndex(lsPairLong_t *il_to, lsPairLong_t *il_from, lsInt_t *indices);
/* wie lsPairLong_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsPairLong_t *  lsPairLong_filter(lsPairLong_t *il, lsPairLong_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairLong_t *  lsPairLong_CpyFilter(lsPairLong_t *il_from, lsPairLong_filter_t *func);
lsPairLong_t *  lsPairLong_cpyFilter(lsPairLong_t *il_to, lsPairLong_t *il_from, lsPairLong_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairLong_t * lsPairLong_sscan_chr(lsPairLong_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsPairLong_sprint_chr(char *s, lsPairLong_t *il, char t);
char * lsPairLong_sprintf_chr(char *s, lsPairLong_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsPairLong_fwrite(FILE *fp, lsPairLong_t *il);
int lsPairLong_fread(lsPairLong_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




