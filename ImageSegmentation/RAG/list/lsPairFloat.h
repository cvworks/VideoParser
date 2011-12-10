#define LS_IS_lsPairFloat
#ifndef LS_lsPairFloat_INCLUDED
#define LS_lsPairFloat_INCLUDED

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

typedef struct lsPairFloat_t {
  pairFloat_t * list;
  int       n_list;
  int     max_list;
} lsPairFloat_t;

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

typedef pairFloat_t (lsPairFloat_fold_t)(pairFloat_t,pairFloat_t);
typedef pairFloat_t (lsPairFloat_fold_2_t)(pairFloat_t,pairFloat_t,void *);
typedef pairFloat_t (lsPairFloat_map_t)(pairFloat_t);
typedef pairFloat_t (lsPairFloat_map_2_t)(pairFloat_t,void *);
typedef pairFloat_t (lsPairFloat_map_3_t)(pairFloat_t,void *,void *);
typedef int (lsPairFloat_filter_t)(pairFloat_t);
typedef int (lsPairFloat_cmp_t)(pairFloat_t,pairFloat_t);
typedef int (lsPairFloat_cmp_2_t)(pairFloat_t,pairFloat_t,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsPairFloat_t *  lsPairFloat_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsPairFloat_t *  lsPairFloat_realloc(lsPairFloat_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsPairFloat_Nil neu allociert. */

lsPairFloat_t *  lsPairFloat_nil(lsPairFloat_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsPairFloat_t *  lsPairFloat_ConsNil(pairFloat_t i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsPairFloat_t *  lsPairFloat_setNil(lsPairFloat_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairFloat_t *  lsPairFloat_setConsNil(lsPairFloat_t * il, pairFloat_t i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairFloat_t *  lsPairFloat_SetPt(int n, pairFloat_t *items);
lsPairFloat_t *  lsPairFloat_setPt(lsPairFloat_t * il_to, int n, pairFloat_t *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairFloat_t *  lsPairFloat_CpyPt(lsPairFloat_t * il_from);
lsPairFloat_t *  lsPairFloat_cpyPt(lsPairFloat_t * il_to, lsPairFloat_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairFloat_t *  lsPairFloat_Cpy(const lsPairFloat_t * il_from);
lsPairFloat_t *  lsPairFloat_cpy(lsPairFloat_t * il_to, const lsPairFloat_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsPairFloat_t *  lsPairFloat_Cat(lsPairFloat_t * il_1, lsPairFloat_t * il_2);
lsPairFloat_t *  lsPairFloat_cat(lsPairFloat_t * il_to, lsPairFloat_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsPairFloat_t * lsPairFloat_addCat(lsPairFloat_t *il_to, lsPairFloat_t *il);
/* Rueckgabe: lsPairFloat_add(il_to, lsPairFloat_Cat(il_to,il)) */
lsPairFloat_t * lsPairFloat_AddCat(lsPairFloat_t *il_to, lsPairFloat_t *il);
/* Rueckgabe: lsPairFloat_Add(il_to, lsPairFloat_Cat(il_to,il)) */
#endif

void      lsPairFloat_Free(lsPairFloat_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsPairFloat_free(lsPairFloat_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

pairFloat_t lsPairFloat_setIndex(lsPairFloat_t *il, int index, pairFloat_t i, pairFloat_t i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsPairFloat_t * lsPairFloat_setIndices(lsPairFloat_t *il, lsInt_t *indices, pairFloat_t x,
		       pairFloat_t undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairFloat_setIndex: index < 0] */

lsPairFloat_t * lsPairFloat_nsetIndex(lsPairFloat_t *il, int index, int n, pairFloat_t x, 
		      pairFloat_t undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPairFloat_setIndex: index < 0] */

pairFloat_t *lsPairFloat_getNewItem(lsPairFloat_t *il);
int        lsPairFloat_getNewItemIndex(lsPairFloat_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsPairFloat_pushLast(il,i) (lsPairFloat_add(il,i))
lsPairFloat_t *  lsPairFloat_add(lsPairFloat_t * il, pairFloat_t i);
lsPairFloat_t *  lsPairFloat_Add(lsPairFloat_t * il, pairFloat_t i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsPairFloat_pushHead(il,i) (lsPairFloat_cons(il,i)
lsPairFloat_t *  lsPairFloat_cons(lsPairFloat_t * il, pairFloat_t i);
lsPairFloat_t *  lsPairFloat_Cons(lsPairFloat_t * il, pairFloat_t i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

pairFloat_t lsPairFloat_popLast(lsPairFloat_t *il, pairFloat_t undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairFloat_t lsPairFloat_popHead(lsPairFloat_t *il, pairFloat_t undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsPairFloat_t *  lsPairFloat_init(lsPairFloat_t * il);
lsPairFloat_t *  lsPairFloat_Init(lsPairFloat_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairFloat_t *  lsPairFloat_tail(lsPairFloat_t * il);
lsPairFloat_t *  lsPairFloat_tail(lsPairFloat_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairFloat_t * lsPairFloat_take(lsPairFloat_t *il, int n);
lsPairFloat_t * lsPairFloat_Take(lsPairFloat_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPairFloat_t * lsPairFloat_drop(lsPairFloat_t *il, int i);
lsPairFloat_t * lsPairFloat_Drop(lsPairFloat_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsPairFloat_t * lsPairFloat_dropPt(lsPairFloat_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsPairFloat_t * lsPairFloat_split(lsPairFloat_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsPairFloat_take(<il>,i)
 * - Rueckgabe lsPairFloat_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsPairFloat_nsplit(lsPt_t *il_split, lsPairFloat_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsPairFloat_take(<il>,is[0])
 * - Rueckgabe[j] = lsPairFloat_Drop(lsPairFloat_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsPairFloat_t * lsPairFloat_range(lsPairFloat_t *il, int i0, int iN);
lsPairFloat_t * lsPairFloat_Range(lsPairFloat_t *il, int i0, int iN);
lsPairFloat_t * lsPairFloat_rangePt(lsPairFloat_t *il, int i0, int iN);
lsPairFloat_t * lsPairFloat_cpyRange(lsPairFloat_t *il_to, lsPairFloat_t *il, int i0, int iN);
lsPairFloat_t * lsPairFloat_catRange(lsPairFloat_t *il_to, lsPairFloat_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsPairFloat_t * lsPairFloat_insSwap(lsPairFloat_t *il, int index, pairFloat_t i, pairFloat_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

pairFloat_t lsPairFloat_delSwap(lsPairFloat_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPairFloat_t * lsPairFloat_insert(lsPairFloat_t *il, int index, pairFloat_t i, pairFloat_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsPairFloat_t * lsPairFloat_insertN(lsPairFloat_t *il, int index, int n, pairFloat_t i, pairFloat_t i0);
/* Wie lsPairFloat_insert, fuegt aber <n>-mal das Element <i> ein */

pairFloat_t lsPairFloat_delete(lsPairFloat_t *il, int index, pairFloat_t undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsPairFloat_t * lsPairFloat_join(lsPairFloat_t *il_to, lsPairFloat_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsPairFloat_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsPairFloat_t * lsPairFloat_joinInts(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsInt_t *value2index);
/* wie lsPairFloat_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsPairFloat_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

pairFloat_t lsPairFloat_last(lsPairFloat_t *il, pairFloat_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairFloat_t lsPairFloat_head(lsPairFloat_t *il, pairFloat_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

pairFloat_t lsPairFloat_get(lsPairFloat_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

pairFloat_t lsPairFloat_getCheck(lsPairFloat_t * il,int i, pairFloat_t undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

pairFloat_t lsPairFloat_getFlip(int i, lsPairFloat_t *il, pairFloat_t undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsPairFloat_t *  lsPairFloat_getRowPt(lsPairFloat_t * row, lsPairFloat_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsPairFloat_t *  lsPairFloat_getRow(lsPairFloat_t *row, lsPairFloat_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsPairFloat_t *  lsPairFloat_getCol(lsPairFloat_t *col, lsPairFloat_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsPairFloat_t *  lsPairFloat_setRow(lsPairFloat_t *il, lsPairFloat_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsPairFloat_t *  lsPairFloat_setCol(lsPairFloat_t *il, lsPairFloat_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsPairFloat_length(lsPairFloat_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsPairFloat_elem(lsPairFloat_t * il, pairFloat_t i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairFloat_neqElem(lsPairFloat_t *il, pairFloat_t i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairFloat_disjoint(lsPairFloat_t *il1, lsPairFloat_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsPairFloat_t *lsPairFloat_subst(lsPairFloat_t *il, pairFloat_t i, pairFloat_t j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsPairFloat_subBag(lsPairFloat_t *il_sub, lsPairFloat_t *il_super, pairFloat_t undef);
int lsPairFloat_subBagIndices(lsInt_t *indices,
		     lsPairFloat_t *il_sub, lsPairFloat_t *il_super, pairFloat_t undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsPairFloat_subBagLimitedIndices(lsInt_t *indices,
			    lsPairFloat_t *il_sub, lsPairFloat_t *il_super, 
			    lsInt_t *limit, pairFloat_t undef);
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

int lsPairFloat_getIndex(lsPairFloat_t * il, pairFloat_t item);
int lsPairFloat_getFstIndex(lsPairFloat_t *il, pairFloat_t item);
int lsPairFloat_getLastIndex(lsPairFloat_t *il, pairFloat_t item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairFloat_getLastNeqIndex(lsPairFloat_t * il, pairFloat_t item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPairFloat_getFstNeqIndex(lsPairFloat_t * il, pairFloat_t item);
/* wie lsPairFloat_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsPairFloat_elemFunc(lsPairFloat_t * il, pairFloat_t i, lsPairFloat_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairFloat_neqElemFunc(lsPairFloat_t *il, pairFloat_t i, lsPairFloat_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPairFloat_getIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsPairFloat_getLastNeqIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsPairFloat_getFstNeqIndexFunc(lsPairFloat_t * il, pairFloat_t item, lsPairFloat_cmp_t *func);
/* wie lsPairFloat_getLastNeqIndex, Suche beginnt hier beim 1. Element */

pairFloat_t lsPairFloat_foldl(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_t *func);
pairFloat_t 
lsPairFloat_foldl_2(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsPairFloat_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

pairFloat_t lsPairFloat_foldr(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_t *func);
pairFloat_t 
lsPairFloat_foldr_2(lsPairFloat_t *il, pairFloat_t item0, lsPairFloat_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsPairFloat_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

pairFloat_t lsPairFloat_maxFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
int       lsPairFloat_maxIndexFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairFloat_t lsPairFloat_minFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
int       lsPairFloat_minIndexFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsPairFloat_bsearchLtFunc(lsPairFloat_t *il, pairFloat_t i, lsPairFloat_cmp_t *func);
int lsPairFloat_bsearchLtFunc_2(lsPairFloat_t *il,pairFloat_t i,lsPairFloat_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairFloat_bsearchGtFunc(lsPairFloat_t *il, pairFloat_t i, lsPairFloat_cmp_t *func);
int lsPairFloat_bsearchGtFunc_2(lsPairFloat_t *il,pairFloat_t i,lsPairFloat_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPairFloat_cmpFunc(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsPairFloat_cmpIndex(int i, int j, pairPt_t *arg);
int lsPairFloat_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsPairFloat_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsPairFloat_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsPairFloat_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsPairFloat_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsPairFloat_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsPairFloat_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
pairFloat_t lsPairFloat_sum(lsPairFloat_t *il);
/* liefert die Summe ueber alle Listenelemente */

pairFloat_t lsPairFloat_prod(lsPairFloat_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsPairFloat_t * lsPairFloat_scale(lsPairFloat_t *il, pairFloat_t s);
/* skaliert jedes Element der List um Faktor s */

lsPairFloat_t *  lsPairFloat_delta(lsPairFloat_t *il_to, lsPairFloat_t *il_from, pairFloat_t base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

pairFloat_t lsPairFloat_max(lsPairFloat_t *il);
int       lsPairFloat_maxIndex(lsPairFloat_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

pairFloat_t lsPairFloat_min(lsPairFloat_t *il);
int       lsPairFloat_minIndex(lsPairFloat_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsPairFloat_t *lsPairFloat_rmdup(lsPairFloat_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsPairFloat_bsearchLt(lsPairFloat_t *il, pairFloat_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairFloat_bsearchGt(lsPairFloat_t *il, pairFloat_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPairFloat_cmp(lsPairFloat_t *il1, lsPairFloat_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsPairFloat_makeIndex(lsInt_t *index, lsPairFloat_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsPairFloat_t *lsPairFloat_reverse(lsPairFloat_t *il);
/* Dreht die Elemente einer Liste um */

lsPairFloat_t *  lsPairFloat_CpyMap(lsPairFloat_t * il_from, lsPairFloat_map_t *func);
lsPairFloat_t *  lsPairFloat_cpyMap(lsPairFloat_t * il_to, lsPairFloat_t * il_from, lsPairFloat_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairFloat_t *  lsPairFloat_map(lsPairFloat_t * il, lsPairFloat_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsPairFloat_t *  lsPairFloat_mapSet(lsPairFloat_t * il, lsPairFloat_map_t *func);
/* wie lsPairFloat_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairFloat_t *  lsPairFloat_map_2(lsPairFloat_t * il, lsPairFloat_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsPairFloat_t *  lsPairFloat_map_3(lsPairFloat_t * il, lsPairFloat_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsPairFloat_t *  lsPairFloat_mapSet_2(lsPairFloat_t * il, lsPairFloat_map_2_t *func, void *arg);
/* wie lsPairFloat_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPairFloat_t *  lsPairFloat_mapSet_3(lsPairFloat_t * il,lsPairFloat_map_3_t *func,void *arg1,void *arg2);
/* wie lsPairFloat_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsPairFloat_t *  lsPairFloat_CpyMap_2(lsPairFloat_t * il_from, lsPairFloat_map_2_t *func, void *arg);
lsPairFloat_t *  lsPairFloat_cpyMap_2(lsPairFloat_t * il_to, lsPairFloat_t * il_from, 
		      lsPairFloat_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairFloat_t *  lsPairFloat_CpyMap_3(lsPairFloat_t * il_from, lsPairFloat_map_3_t *func, 
		      void *arg1, void *arg2);
lsPairFloat_t *  lsPairFloat_cpyMap_3(lsPairFloat_t * il_to, lsPairFloat_t * il_from, 
		      lsPairFloat_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPairFloat_t * lsPairFloat_CartProd(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func);
lsPairFloat_t * lsPairFloat_cartProd(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func);
lsPairFloat_t * lsPairFloat_cpyCartProd(lsPairFloat_t *il_to, 
			lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_t *func);
lsPairFloat_t * lsPairFloat_cartProd_2(lsPairFloat_t *il1, lsPairFloat_t *il2, lsPairFloat_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsPairFloat_CartProd gibt eine neu allocierte List zurueck
 * lsPairFloat_cartProd gibt il1 als Ergebnisliste zurueck
 * lsPairFloat_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsPairFloat_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsPairFloat_t * lsPairFloat_sortByIndex(lsPairFloat_t *il, lsInt_t *index, pairFloat_t undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsPairFloat_t * lsPairFloat_cpySortByIndex(lsPairFloat_t *il_to, lsPairFloat_t *il_from, 
			   lsInt_t *index, pairFloat_t undef);
/* wie lsPairFloat_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsPairFloat_t * lsPairFloat_qsortLtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_qsortGtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_qsortLtFunc_2(lsPairFloat_t *il, lsPairFloat_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_qsortGtFunc_2(lsPairFloat_t *il, lsPairFloat_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairFloat_qsortIndexLtFunc(lsInt_t *index, lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairFloat_qsortIndexGtFunc(lsInt_t *index, lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_msortLtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsPairFloat_t * lsPairFloat_msortGtFunc(lsPairFloat_t *il, lsPairFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairFloat_t * lsPairFloat_qsortLt(lsPairFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_qsortGt(lsPairFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairFloat_qsortIndexLt(lsInt_t *index, lsPairFloat_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPairFloat_qsortIndexGt(lsInt_t *index, lsPairFloat_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPairFloat_t * lsPairFloat_msortLt(lsPairFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsPairFloat_t * lsPairFloat_msortGt(lsPairFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsPairFloat_t * lsPairFloat_filterByValue(lsPairFloat_t *il, pairFloat_t undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsPairFloat_t * lsPairFloat_CpyFilterByValue(lsPairFloat_t *il, pairFloat_t undef, lsInt_t *indices);
/* wie lsPairFloat_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairFloat_t * lsPairFloat_cpyFilterByValue(lsPairFloat_t *il_to, lsPairFloat_t *il_from, pairFloat_t undef, 
			     lsInt_t *indices);
/* wie lsPairFloat_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsPairFloat_t * lsPairFloat_filterByIndex(lsPairFloat_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsPairFloat_t * lsPairFloat_CpyFilterByIndex(lsPairFloat_t *il, lsInt_t *indices);
/* wie lsPairFloat_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPairFloat_t * lsPairFloat_cpyFilterByIndex(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsInt_t *indices);
/* wie lsPairFloat_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsPairFloat_t *  lsPairFloat_filter(lsPairFloat_t *il, lsPairFloat_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPairFloat_t *  lsPairFloat_CpyFilter(lsPairFloat_t *il_from, lsPairFloat_filter_t *func);
lsPairFloat_t *  lsPairFloat_cpyFilter(lsPairFloat_t *il_to, lsPairFloat_t *il_from, lsPairFloat_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPairFloat_t * lsPairFloat_sscan_chr(lsPairFloat_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsPairFloat_sprint_chr(char *s, lsPairFloat_t *il, char t);
char * lsPairFloat_sprintf_chr(char *s, lsPairFloat_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsPairFloat_fwrite(FILE *fp, lsPairFloat_t *il);
int lsPairFloat_fread(lsPairFloat_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




