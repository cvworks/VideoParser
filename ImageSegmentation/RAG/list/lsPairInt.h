#define LS_IS_lsPairInt
#ifndef LS_lsPairInt_INCLUDED
#define LS_lsPairInt_INCLUDED

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

typedef struct lsPairInt_t {
  pairInt_t * list;
  int       n_list;
  int     max_list;
} lsPairInt_t;

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

typedef pairInt_t (lsPairInt_fold_t)(pairInt_t,pairInt_t);
typedef pairInt_t (lsPairInt_fold_2_t)(pairInt_t,pairInt_t,void *);
typedef pairInt_t (lsPairInt_map_t)(pairInt_t);
typedef pairInt_t (lsPairInt_map_2_t)(pairInt_t,void *);
typedef pairInt_t (lsPairInt_map_3_t)(pairInt_t,void *,void *);
typedef int (lsPairInt_filter_t)(pairInt_t);
typedef int (lsPairInt_cmp_t)(pairInt_t,pairInt_t);
typedef int (lsPairInt_cmp_2_t)(pairInt_t,pairInt_t,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsPairInt_t *  lsPairInt_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsPairInt_t *  lsPairInt_realloc(lsPairInt_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsPairInt_Nil neu allociert. */

lsPairInt_t *  lsPairInt_nil(lsPairInt_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsPairInt_t *  lsPairInt_ConsNil(pairInt_t i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsPairInt_t *  lsPairInt_setNil(lsPairInt_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairInt_t *  lsPairInt_setConsNil(lsPairInt_t * il, pairInt_t i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairInt_t *  lsPairInt_SetPt(int n, pairInt_t *items);
lsPairInt_t *  lsPairInt_setPt(lsPairInt_t * il_to, int n, pairInt_t *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairInt_t *  lsPairInt_CpyPt(lsPairInt_t * il_from);
lsPairInt_t *  lsPairInt_cpyPt(lsPairInt_t * il_to, lsPairInt_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairInt_t *  lsPairInt_Cpy(const lsPairInt_t * il_from);
lsPairInt_t *  lsPairInt_cpy(lsPairInt_t * il_to, const lsPairInt_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsPairInt_t *  lsPairInt_Cat(lsPairInt_t * il_1, lsPairInt_t * il_2);
lsPairInt_t *  lsPairInt_cat(lsPairInt_t * il_to, lsPairInt_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsPairInt_t * lsPairInt_addCat(lsPairInt_t *il_to, lsPairInt_t *il);
/* Rueckgabe: lsPairInt_add(il_to, lsPairInt_Cat(il_to,il)) */
lsPairInt_t * lsPairInt_AddCat(lsPairInt_t *il_to, lsPairInt_t *il);
/* Rueckgabe: lsPairInt_Add(il_to, lsPairInt_Cat(il_to,il)) */
#endif

void      lsPairInt_Free(lsPairInt_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsPairInt_free(lsPairInt_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

pairInt_t lsPairInt_setIndex(lsPairInt_t *il, int index, pairInt_t i, pairInt_t i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsPairInt_t * lsPairInt_setIndices(lsPairInt_t *il, lsInt_t *indices, pairInt_t x,
		       pairInt_t undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairInt_setIndex: index < 0] */

lsPairInt_t * lsPairInt_nsetIndex(lsPairInt_t *il, int index, int n, pairInt_t x, 
		      pairInt_t undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairInt_setIndex: index < 0] */

pairInt_t *lsPairInt_getNewItem(lsPairInt_t *il);
int        lsPairInt_getNewItemIndex(lsPairInt_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsPairInt_pushLast(il,i) (lsPairInt_add(il,i))
lsPairInt_t *  lsPairInt_add(lsPairInt_t * il, pairInt_t i);
lsPairInt_t *  lsPairInt_Add(lsPairInt_t * il, pairInt_t i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsPairInt_pushHead(il,i) (lsPairInt_cons(il,i)
lsPairInt_t *  lsPairInt_cons(lsPairInt_t * il, pairInt_t i);
lsPairInt_t *  lsPairInt_Cons(lsPairInt_t * il, pairInt_t i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

pairInt_t lsPairInt_popLast(lsPairInt_t *il, pairInt_t undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairInt_t lsPairInt_popHead(lsPairInt_t *il, pairInt_t undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsPairInt_t *  lsPairInt_init(lsPairInt_t * il);
lsPairInt_t *  lsPairInt_Init(lsPairInt_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairInt_t *  lsPairInt_tail(lsPairInt_t * il);
lsPairInt_t *  lsPairInt_tail(lsPairInt_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairInt_t * lsPairInt_take(lsPairInt_t *il, int n);
lsPairInt_t * lsPairInt_Take(lsPairInt_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairInt_t * lsPairInt_drop(lsPairInt_t *il, int i);
lsPairInt_t * lsPairInt_Drop(lsPairInt_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsPairInt_t * lsPairInt_dropPt(lsPairInt_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsPairInt_t * lsPairInt_split(lsPairInt_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsPairInt_take(<il>,i)
 * - Rueckgabe lsPairInt_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsPairInt_nsplit(lsPt_t *il_split, lsPairInt_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsPairInt_take(<il>,is[0])
 * - Rueckgabe[j] = lsPairInt_Drop(lsPairInt_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsPairInt_t * lsPairInt_range(lsPairInt_t *il, int i0, int iN);
lsPairInt_t * lsPairInt_Range(lsPairInt_t *il, int i0, int iN);
lsPairInt_t * lsPairInt_rangePt(lsPairInt_t *il, int i0, int iN);
lsPairInt_t * lsPairInt_cpyRange(lsPairInt_t *il_to, lsPairInt_t *il, int i0, int iN);
lsPairInt_t * lsPairInt_catRange(lsPairInt_t *il_to, lsPairInt_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsPairInt_t * lsPairInt_insSwap(lsPairInt_t *il, int index, pairInt_t i, pairInt_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

pairInt_t lsPairInt_delSwap(lsPairInt_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairInt_t * lsPairInt_insert(lsPairInt_t *il, int index, pairInt_t i, pairInt_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsPairInt_t * lsPairInt_insertN(lsPairInt_t *il, int index, int n, pairInt_t i, pairInt_t i0);
/* Wie lsPairInt_insert, fuegt aber <n>-mal das Element <i> ein */

pairInt_t lsPairInt_delete(lsPairInt_t *il, int index, pairInt_t undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsPairInt_t * lsPairInt_join(lsPairInt_t *il_to, lsPairInt_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsPairInt_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsPairInt_t * lsPairInt_joinInts(lsPairInt_t *il_to, lsPairInt_t *il_from, lsInt_t *value2index);
/* wie lsPairInt_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsPairInt_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

pairInt_t lsPairInt_last(lsPairInt_t *il, pairInt_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairInt_t lsPairInt_head(lsPairInt_t *il, pairInt_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairInt_t lsPairInt_get(lsPairInt_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

pairInt_t lsPairInt_getCheck(lsPairInt_t * il,int i, pairInt_t undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

pairInt_t lsPairInt_getFlip(int i, lsPairInt_t *il, pairInt_t undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsPairInt_t *  lsPairInt_getRowPt(lsPairInt_t * row, lsPairInt_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsPairInt_t *  lsPairInt_getRow(lsPairInt_t *row, lsPairInt_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsPairInt_t *  lsPairInt_getCol(lsPairInt_t *col, lsPairInt_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsPairInt_t *  lsPairInt_setRow(lsPairInt_t *il, lsPairInt_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsPairInt_t *  lsPairInt_setCol(lsPairInt_t *il, lsPairInt_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsPairInt_length(lsPairInt_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsPairInt_elem(lsPairInt_t * il, pairInt_t i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairInt_neqElem(lsPairInt_t *il, pairInt_t i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairInt_disjoint(lsPairInt_t *il1, lsPairInt_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsPairInt_t *lsPairInt_subst(lsPairInt_t *il, pairInt_t i, pairInt_t j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsPairInt_subBag(lsPairInt_t *il_sub, lsPairInt_t *il_super, pairInt_t undef);
int lsPairInt_subBagIndices(lsInt_t *indices,
		     lsPairInt_t *il_sub, lsPairInt_t *il_super, pairInt_t undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsPairInt_subBagLimitedIndices(lsInt_t *indices,
			    lsPairInt_t *il_sub, lsPairInt_t *il_super, 
			    lsInt_t *limit, pairInt_t undef);
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

int lsPairInt_getIndex(lsPairInt_t * il, pairInt_t item);
int lsPairInt_getFstIndex(lsPairInt_t *il, pairInt_t item);
int lsPairInt_getLastIndex(lsPairInt_t *il, pairInt_t item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairInt_getLastNeqIndex(lsPairInt_t * il, pairInt_t item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairInt_getFstNeqIndex(lsPairInt_t * il, pairInt_t item);
/* wie lsPairInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsPairInt_elemFunc(lsPairInt_t * il, pairInt_t i, lsPairInt_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairInt_neqElemFunc(lsPairInt_t *il, pairInt_t i, lsPairInt_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairInt_getIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsPairInt_getLastNeqIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsPairInt_getFstNeqIndexFunc(lsPairInt_t * il, pairInt_t item, lsPairInt_cmp_t *func);
/* wie lsPairInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */

pairInt_t lsPairInt_foldl(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_t *func);
pairInt_t 
lsPairInt_foldl_2(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsPairInt_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

pairInt_t lsPairInt_foldr(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_t *func);
pairInt_t 
lsPairInt_foldr_2(lsPairInt_t *il, pairInt_t item0, lsPairInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsPairInt_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

pairInt_t lsPairInt_maxFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
int       lsPairInt_maxIndexFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairInt_t lsPairInt_minFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
int       lsPairInt_minIndexFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsPairInt_bsearchLtFunc(lsPairInt_t *il, pairInt_t i, lsPairInt_cmp_t *func);
int lsPairInt_bsearchLtFunc_2(lsPairInt_t *il,pairInt_t i,lsPairInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairInt_bsearchGtFunc(lsPairInt_t *il, pairInt_t i, lsPairInt_cmp_t *func);
int lsPairInt_bsearchGtFunc_2(lsPairInt_t *il,pairInt_t i,lsPairInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairInt_cmpFunc(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsPairInt_cmpIndex(int i, int j, pairPt_t *arg);
int lsPairInt_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsPairInt_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsPairInt_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsPairInt_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsPairInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsPairInt_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsPairInt_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
pairInt_t lsPairInt_sum(lsPairInt_t *il);
/* liefert die Summe ueber alle Listenelemente */

pairInt_t lsPairInt_prod(lsPairInt_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsPairInt_t * lsPairInt_scale(lsPairInt_t *il, pairInt_t s);
/* skaliert jedes Element der List um Faktor s */

lsPairInt_t *  lsPairInt_delta(lsPairInt_t *il_to, lsPairInt_t *il_from, pairInt_t base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

pairInt_t lsPairInt_max(lsPairInt_t *il);
int       lsPairInt_maxIndex(lsPairInt_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairInt_t lsPairInt_min(lsPairInt_t *il);
int       lsPairInt_minIndex(lsPairInt_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsPairInt_t *lsPairInt_rmdup(lsPairInt_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsPairInt_bsearchLt(lsPairInt_t *il, pairInt_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairInt_bsearchGt(lsPairInt_t *il, pairInt_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairInt_cmp(lsPairInt_t *il1, lsPairInt_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsPairInt_makeIndex(lsInt_t *index, lsPairInt_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsPairInt_t *lsPairInt_reverse(lsPairInt_t *il);
/* Dreht die Elemente einer Liste um */

lsPairInt_t *  lsPairInt_CpyMap(lsPairInt_t * il_from, lsPairInt_map_t *func);
lsPairInt_t *  lsPairInt_cpyMap(lsPairInt_t * il_to, lsPairInt_t * il_from, lsPairInt_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairInt_t *  lsPairInt_map(lsPairInt_t * il, lsPairInt_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsPairInt_t *  lsPairInt_mapSet(lsPairInt_t * il, lsPairInt_map_t *func);
/* wie lsPairInt_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairInt_t *  lsPairInt_map_2(lsPairInt_t * il, lsPairInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsPairInt_t *  lsPairInt_map_3(lsPairInt_t * il, lsPairInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsPairInt_t *  lsPairInt_mapSet_2(lsPairInt_t * il, lsPairInt_map_2_t *func, void *arg);
/* wie lsPairInt_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairInt_t *  lsPairInt_mapSet_3(lsPairInt_t * il,lsPairInt_map_3_t *func,void *arg1,void *arg2);
/* wie lsPairInt_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsPairInt_t *  lsPairInt_CpyMap_2(lsPairInt_t * il_from, lsPairInt_map_2_t *func, void *arg);
lsPairInt_t *  lsPairInt_cpyMap_2(lsPairInt_t * il_to, lsPairInt_t * il_from, 
		      lsPairInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairInt_t *  lsPairInt_CpyMap_3(lsPairInt_t * il_from, lsPairInt_map_3_t *func, 
		      void *arg1, void *arg2);
lsPairInt_t *  lsPairInt_cpyMap_3(lsPairInt_t * il_to, lsPairInt_t * il_from, 
		      lsPairInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairInt_t * lsPairInt_CartProd(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func);
lsPairInt_t * lsPairInt_cartProd(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func);
lsPairInt_t * lsPairInt_cpyCartProd(lsPairInt_t *il_to, 
			lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_t *func);
lsPairInt_t * lsPairInt_cartProd_2(lsPairInt_t *il1, lsPairInt_t *il2, lsPairInt_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsPairInt_CartProd gibt eine neu allocierte List zurueck
 * lsPairInt_cartProd gibt il1 als Ergebnisliste zurueck
 * lsPairInt_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsPairInt_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsPairInt_t * lsPairInt_sortByIndex(lsPairInt_t *il, lsInt_t *index, pairInt_t undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsPairInt_t * lsPairInt_cpySortByIndex(lsPairInt_t *il_to, lsPairInt_t *il_from, 
			   lsInt_t *index, pairInt_t undef);
/* wie lsPairInt_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsPairInt_t * lsPairInt_qsortLtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_qsortGtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_qsortLtFunc_2(lsPairInt_t *il, lsPairInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_qsortGtFunc_2(lsPairInt_t *il, lsPairInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairInt_qsortIndexLtFunc(lsInt_t *index, lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairInt_qsortIndexGtFunc(lsInt_t *index, lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_msortLtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsPairInt_t * lsPairInt_msortGtFunc(lsPairInt_t *il, lsPairInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairInt_t * lsPairInt_qsortLt(lsPairInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_qsortGt(lsPairInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairInt_qsortIndexLt(lsInt_t *index, lsPairInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairInt_qsortIndexGt(lsInt_t *index, lsPairInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairInt_t * lsPairInt_msortLt(lsPairInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsPairInt_t * lsPairInt_msortGt(lsPairInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsPairInt_t * lsPairInt_filterByValue(lsPairInt_t *il, pairInt_t undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsPairInt_t * lsPairInt_CpyFilterByValue(lsPairInt_t *il, pairInt_t undef, lsInt_t *indices);
/* wie lsPairInt_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairInt_t * lsPairInt_cpyFilterByValue(lsPairInt_t *il_to, lsPairInt_t *il_from, pairInt_t undef, 
			     lsInt_t *indices);
/* wie lsPairInt_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsPairInt_t * lsPairInt_filterByIndex(lsPairInt_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsPairInt_t * lsPairInt_CpyFilterByIndex(lsPairInt_t *il, lsInt_t *indices);
/* wie lsPairInt_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairInt_t * lsPairInt_cpyFilterByIndex(lsPairInt_t *il_to, lsPairInt_t *il_from, lsInt_t *indices);
/* wie lsPairInt_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsPairInt_t *  lsPairInt_filter(lsPairInt_t *il, lsPairInt_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairInt_t *  lsPairInt_CpyFilter(lsPairInt_t *il_from, lsPairInt_filter_t *func);
lsPairInt_t *  lsPairInt_cpyFilter(lsPairInt_t *il_to, lsPairInt_t *il_from, lsPairInt_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairInt_t * lsPairInt_sscan_chr(lsPairInt_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsPairInt_sprint_chr(char *s, lsPairInt_t *il, char t);
char * lsPairInt_sprintf_chr(char *s, lsPairInt_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsPairInt_fwrite(FILE *fp, lsPairInt_t *il);
int lsPairInt_fread(lsPairInt_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




