#define LS_IS_lsTripleInt
#ifndef LS_lsTripleInt_INCLUDED
#define LS_lsTripleInt_INCLUDED

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

typedef struct lsTripleInt_t {
  tripleInt_t * list;
  int       n_list;
  int     max_list;
} lsTripleInt_t;

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

typedef tripleInt_t (lsTripleInt_fold_t)(tripleInt_t,tripleInt_t);
typedef tripleInt_t (lsTripleInt_fold_2_t)(tripleInt_t,tripleInt_t,void *);
typedef tripleInt_t (lsTripleInt_map_t)(tripleInt_t);
typedef tripleInt_t (lsTripleInt_map_2_t)(tripleInt_t,void *);
typedef tripleInt_t (lsTripleInt_map_3_t)(tripleInt_t,void *,void *);
typedef int (lsTripleInt_filter_t)(tripleInt_t);
typedef int (lsTripleInt_cmp_t)(tripleInt_t,tripleInt_t);
typedef int (lsTripleInt_cmp_2_t)(tripleInt_t,tripleInt_t,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsTripleInt_t *  lsTripleInt_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsTripleInt_t *  lsTripleInt_realloc(lsTripleInt_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsTripleInt_Nil neu allociert. */

lsTripleInt_t *  lsTripleInt_nil(lsTripleInt_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsTripleInt_t *  lsTripleInt_ConsNil(tripleInt_t i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsTripleInt_t *  lsTripleInt_setNil(lsTripleInt_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsTripleInt_t *  lsTripleInt_setConsNil(lsTripleInt_t * il, tripleInt_t i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsTripleInt_t *  lsTripleInt_SetPt(int n, tripleInt_t *items);
lsTripleInt_t *  lsTripleInt_setPt(lsTripleInt_t * il_to, int n, tripleInt_t *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsTripleInt_t *  lsTripleInt_CpyPt(lsTripleInt_t * il_from);
lsTripleInt_t *  lsTripleInt_cpyPt(lsTripleInt_t * il_to, lsTripleInt_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsTripleInt_t *  lsTripleInt_Cpy(const lsTripleInt_t * il_from);
lsTripleInt_t *  lsTripleInt_cpy(lsTripleInt_t * il_to, const lsTripleInt_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsTripleInt_t *  lsTripleInt_Cat(lsTripleInt_t * il_1, lsTripleInt_t * il_2);
lsTripleInt_t *  lsTripleInt_cat(lsTripleInt_t * il_to, lsTripleInt_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsTripleInt_t * lsTripleInt_addCat(lsTripleInt_t *il_to, lsTripleInt_t *il);
/* Rueckgabe: lsTripleInt_add(il_to, lsTripleInt_Cat(il_to,il)) */
lsTripleInt_t * lsTripleInt_AddCat(lsTripleInt_t *il_to, lsTripleInt_t *il);
/* Rueckgabe: lsTripleInt_Add(il_to, lsTripleInt_Cat(il_to,il)) */
#endif

void      lsTripleInt_Free(lsTripleInt_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsTripleInt_free(lsTripleInt_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

tripleInt_t lsTripleInt_setIndex(lsTripleInt_t *il, int index, tripleInt_t i, tripleInt_t i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsTripleInt_t * lsTripleInt_setIndices(lsTripleInt_t *il, lsInt_t *indices, tripleInt_t x,
		       tripleInt_t undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsTripleInt_setIndex: index < 0] */

lsTripleInt_t * lsTripleInt_nsetIndex(lsTripleInt_t *il, int index, int n, tripleInt_t x, 
		      tripleInt_t undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsTripleInt_setIndex: index < 0] */

tripleInt_t *lsTripleInt_getNewItem(lsTripleInt_t *il);
int        lsTripleInt_getNewItemIndex(lsTripleInt_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsTripleInt_pushLast(il,i) (lsTripleInt_add(il,i))
lsTripleInt_t *  lsTripleInt_add(lsTripleInt_t * il, tripleInt_t i);
lsTripleInt_t *  lsTripleInt_Add(lsTripleInt_t * il, tripleInt_t i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsTripleInt_pushHead(il,i) (lsTripleInt_cons(il,i)
lsTripleInt_t *  lsTripleInt_cons(lsTripleInt_t * il, tripleInt_t i);
lsTripleInt_t *  lsTripleInt_Cons(lsTripleInt_t * il, tripleInt_t i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

tripleInt_t lsTripleInt_popLast(lsTripleInt_t *il, tripleInt_t undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

tripleInt_t lsTripleInt_popHead(lsTripleInt_t *il, tripleInt_t undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsTripleInt_t *  lsTripleInt_init(lsTripleInt_t * il);
lsTripleInt_t *  lsTripleInt_Init(lsTripleInt_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsTripleInt_t *  lsTripleInt_tail(lsTripleInt_t * il);
lsTripleInt_t *  lsTripleInt_tail(lsTripleInt_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsTripleInt_t * lsTripleInt_take(lsTripleInt_t *il, int n);
lsTripleInt_t * lsTripleInt_Take(lsTripleInt_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsTripleInt_t * lsTripleInt_drop(lsTripleInt_t *il, int i);
lsTripleInt_t * lsTripleInt_Drop(lsTripleInt_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsTripleInt_t * lsTripleInt_dropPt(lsTripleInt_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsTripleInt_t * lsTripleInt_split(lsTripleInt_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsTripleInt_take(<il>,i)
 * - Rueckgabe lsTripleInt_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsTripleInt_nsplit(lsPt_t *il_split, lsTripleInt_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsTripleInt_take(<il>,is[0])
 * - Rueckgabe[j] = lsTripleInt_Drop(lsTripleInt_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsTripleInt_t * lsTripleInt_range(lsTripleInt_t *il, int i0, int iN);
lsTripleInt_t * lsTripleInt_Range(lsTripleInt_t *il, int i0, int iN);
lsTripleInt_t * lsTripleInt_rangePt(lsTripleInt_t *il, int i0, int iN);
lsTripleInt_t * lsTripleInt_cpyRange(lsTripleInt_t *il_to, lsTripleInt_t *il, int i0, int iN);
lsTripleInt_t * lsTripleInt_catRange(lsTripleInt_t *il_to, lsTripleInt_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsTripleInt_t * lsTripleInt_insSwap(lsTripleInt_t *il, int index, tripleInt_t i, tripleInt_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

tripleInt_t lsTripleInt_delSwap(lsTripleInt_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsTripleInt_t * lsTripleInt_insert(lsTripleInt_t *il, int index, tripleInt_t i, tripleInt_t i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsTripleInt_t * lsTripleInt_insertN(lsTripleInt_t *il, int index, int n, tripleInt_t i, tripleInt_t i0);
/* Wie lsTripleInt_insert, fuegt aber <n>-mal das Element <i> ein */

tripleInt_t lsTripleInt_delete(lsTripleInt_t *il, int index, tripleInt_t undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsTripleInt_t * lsTripleInt_join(lsTripleInt_t *il_to, lsTripleInt_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsTripleInt_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsTripleInt_t * lsTripleInt_joinInts(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsInt_t *value2index);
/* wie lsTripleInt_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsTripleInt_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

tripleInt_t lsTripleInt_last(lsTripleInt_t *il, tripleInt_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

tripleInt_t lsTripleInt_head(lsTripleInt_t *il, tripleInt_t undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

tripleInt_t lsTripleInt_get(lsTripleInt_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

tripleInt_t lsTripleInt_getCheck(lsTripleInt_t * il,int i, tripleInt_t undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

tripleInt_t lsTripleInt_getFlip(int i, lsTripleInt_t *il, tripleInt_t undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsTripleInt_t *  lsTripleInt_getRowPt(lsTripleInt_t * row, lsTripleInt_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsTripleInt_t *  lsTripleInt_getRow(lsTripleInt_t *row, lsTripleInt_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsTripleInt_t *  lsTripleInt_getCol(lsTripleInt_t *col, lsTripleInt_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsTripleInt_t *  lsTripleInt_setRow(lsTripleInt_t *il, lsTripleInt_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsTripleInt_t *  lsTripleInt_setCol(lsTripleInt_t *il, lsTripleInt_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsTripleInt_length(lsTripleInt_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsTripleInt_elem(lsTripleInt_t * il, tripleInt_t i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsTripleInt_neqElem(lsTripleInt_t *il, tripleInt_t i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsTripleInt_disjoint(lsTripleInt_t *il1, lsTripleInt_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsTripleInt_t *lsTripleInt_subst(lsTripleInt_t *il, tripleInt_t i, tripleInt_t j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsTripleInt_subBag(lsTripleInt_t *il_sub, lsTripleInt_t *il_super, tripleInt_t undef);
int lsTripleInt_subBagIndices(lsInt_t *indices,
		     lsTripleInt_t *il_sub, lsTripleInt_t *il_super, tripleInt_t undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsTripleInt_subBagLimitedIndices(lsInt_t *indices,
			    lsTripleInt_t *il_sub, lsTripleInt_t *il_super, 
			    lsInt_t *limit, tripleInt_t undef);
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

int lsTripleInt_getIndex(lsTripleInt_t * il, tripleInt_t item);
int lsTripleInt_getFstIndex(lsTripleInt_t *il, tripleInt_t item);
int lsTripleInt_getLastIndex(lsTripleInt_t *il, tripleInt_t item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsTripleInt_getLastNeqIndex(lsTripleInt_t * il, tripleInt_t item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsTripleInt_getFstNeqIndex(lsTripleInt_t * il, tripleInt_t item);
/* wie lsTripleInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsTripleInt_elemFunc(lsTripleInt_t * il, tripleInt_t i, lsTripleInt_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsTripleInt_neqElemFunc(lsTripleInt_t *il, tripleInt_t i, lsTripleInt_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsTripleInt_getIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsTripleInt_getLastNeqIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsTripleInt_getFstNeqIndexFunc(lsTripleInt_t * il, tripleInt_t item, lsTripleInt_cmp_t *func);
/* wie lsTripleInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */

tripleInt_t lsTripleInt_foldl(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_t *func);
tripleInt_t 
lsTripleInt_foldl_2(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsTripleInt_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

tripleInt_t lsTripleInt_foldr(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_t *func);
tripleInt_t 
lsTripleInt_foldr_2(lsTripleInt_t *il, tripleInt_t item0, lsTripleInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsTripleInt_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

tripleInt_t lsTripleInt_maxFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
int       lsTripleInt_maxIndexFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

tripleInt_t lsTripleInt_minFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
int       lsTripleInt_minIndexFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsTripleInt_bsearchLtFunc(lsTripleInt_t *il, tripleInt_t i, lsTripleInt_cmp_t *func);
int lsTripleInt_bsearchLtFunc_2(lsTripleInt_t *il,tripleInt_t i,lsTripleInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsTripleInt_bsearchGtFunc(lsTripleInt_t *il, tripleInt_t i, lsTripleInt_cmp_t *func);
int lsTripleInt_bsearchGtFunc_2(lsTripleInt_t *il,tripleInt_t i,lsTripleInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsTripleInt_cmpFunc(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsTripleInt_cmpIndex(int i, int j, pairPt_t *arg);
int lsTripleInt_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsTripleInt_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsTripleInt_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsTripleInt_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsTripleInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsTripleInt_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsTripleInt_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
tripleInt_t lsTripleInt_sum(lsTripleInt_t *il);
/* liefert die Summe ueber alle Listenelemente */

tripleInt_t lsTripleInt_prod(lsTripleInt_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsTripleInt_t * lsTripleInt_scale(lsTripleInt_t *il, tripleInt_t s);
/* skaliert jedes Element der List um Faktor s */

lsTripleInt_t *  lsTripleInt_delta(lsTripleInt_t *il_to, lsTripleInt_t *il_from, tripleInt_t base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

tripleInt_t lsTripleInt_max(lsTripleInt_t *il);
int       lsTripleInt_maxIndex(lsTripleInt_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

tripleInt_t lsTripleInt_min(lsTripleInt_t *il);
int       lsTripleInt_minIndex(lsTripleInt_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsTripleInt_t *lsTripleInt_rmdup(lsTripleInt_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsTripleInt_bsearchLt(lsTripleInt_t *il, tripleInt_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsTripleInt_bsearchGt(lsTripleInt_t *il, tripleInt_t i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsTripleInt_cmp(lsTripleInt_t *il1, lsTripleInt_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsTripleInt_makeIndex(lsInt_t *index, lsTripleInt_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsTripleInt_t *lsTripleInt_reverse(lsTripleInt_t *il);
/* Dreht die Elemente einer Liste um */

lsTripleInt_t *  lsTripleInt_CpyMap(lsTripleInt_t * il_from, lsTripleInt_map_t *func);
lsTripleInt_t *  lsTripleInt_cpyMap(lsTripleInt_t * il_to, lsTripleInt_t * il_from, lsTripleInt_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsTripleInt_t *  lsTripleInt_map(lsTripleInt_t * il, lsTripleInt_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsTripleInt_t *  lsTripleInt_mapSet(lsTripleInt_t * il, lsTripleInt_map_t *func);
/* wie lsTripleInt_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsTripleInt_t *  lsTripleInt_map_2(lsTripleInt_t * il, lsTripleInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsTripleInt_t *  lsTripleInt_map_3(lsTripleInt_t * il, lsTripleInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsTripleInt_t *  lsTripleInt_mapSet_2(lsTripleInt_t * il, lsTripleInt_map_2_t *func, void *arg);
/* wie lsTripleInt_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsTripleInt_t *  lsTripleInt_mapSet_3(lsTripleInt_t * il,lsTripleInt_map_3_t *func,void *arg1,void *arg2);
/* wie lsTripleInt_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsTripleInt_t *  lsTripleInt_CpyMap_2(lsTripleInt_t * il_from, lsTripleInt_map_2_t *func, void *arg);
lsTripleInt_t *  lsTripleInt_cpyMap_2(lsTripleInt_t * il_to, lsTripleInt_t * il_from, 
		      lsTripleInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsTripleInt_t *  lsTripleInt_CpyMap_3(lsTripleInt_t * il_from, lsTripleInt_map_3_t *func, 
		      void *arg1, void *arg2);
lsTripleInt_t *  lsTripleInt_cpyMap_3(lsTripleInt_t * il_to, lsTripleInt_t * il_from, 
		      lsTripleInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsTripleInt_t * lsTripleInt_CartProd(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func);
lsTripleInt_t * lsTripleInt_cartProd(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func);
lsTripleInt_t * lsTripleInt_cpyCartProd(lsTripleInt_t *il_to, 
			lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_t *func);
lsTripleInt_t * lsTripleInt_cartProd_2(lsTripleInt_t *il1, lsTripleInt_t *il2, lsTripleInt_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsTripleInt_CartProd gibt eine neu allocierte List zurueck
 * lsTripleInt_cartProd gibt il1 als Ergebnisliste zurueck
 * lsTripleInt_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsTripleInt_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsTripleInt_t * lsTripleInt_sortByIndex(lsTripleInt_t *il, lsInt_t *index, tripleInt_t undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsTripleInt_t * lsTripleInt_cpySortByIndex(lsTripleInt_t *il_to, lsTripleInt_t *il_from, 
			   lsInt_t *index, tripleInt_t undef);
/* wie lsTripleInt_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsTripleInt_t * lsTripleInt_qsortLtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_qsortGtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_qsortLtFunc_2(lsTripleInt_t *il, lsTripleInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_qsortGtFunc_2(lsTripleInt_t *il, lsTripleInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsTripleInt_qsortIndexLtFunc(lsInt_t *index, lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsTripleInt_qsortIndexGtFunc(lsInt_t *index, lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_msortLtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsTripleInt_t * lsTripleInt_msortGtFunc(lsTripleInt_t *il, lsTripleInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsTripleInt_t * lsTripleInt_qsortLt(lsTripleInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_qsortGt(lsTripleInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsTripleInt_qsortIndexLt(lsInt_t *index, lsTripleInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsTripleInt_qsortIndexGt(lsInt_t *index, lsTripleInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsTripleInt_t * lsTripleInt_msortLt(lsTripleInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsTripleInt_t * lsTripleInt_msortGt(lsTripleInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsTripleInt_t * lsTripleInt_filterByValue(lsTripleInt_t *il, tripleInt_t undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsTripleInt_t * lsTripleInt_CpyFilterByValue(lsTripleInt_t *il, tripleInt_t undef, lsInt_t *indices);
/* wie lsTripleInt_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsTripleInt_t * lsTripleInt_cpyFilterByValue(lsTripleInt_t *il_to, lsTripleInt_t *il_from, tripleInt_t undef, 
			     lsInt_t *indices);
/* wie lsTripleInt_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsTripleInt_t * lsTripleInt_filterByIndex(lsTripleInt_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsTripleInt_t * lsTripleInt_CpyFilterByIndex(lsTripleInt_t *il, lsInt_t *indices);
/* wie lsTripleInt_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsTripleInt_t * lsTripleInt_cpyFilterByIndex(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsInt_t *indices);
/* wie lsTripleInt_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsTripleInt_t *  lsTripleInt_filter(lsTripleInt_t *il, lsTripleInt_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsTripleInt_t *  lsTripleInt_CpyFilter(lsTripleInt_t *il_from, lsTripleInt_filter_t *func);
lsTripleInt_t *  lsTripleInt_cpyFilter(lsTripleInt_t *il_to, lsTripleInt_t *il_from, lsTripleInt_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsTripleInt_t * lsTripleInt_sscan_chr(lsTripleInt_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsTripleInt_sprint_chr(char *s, lsTripleInt_t *il, char t);
char * lsTripleInt_sprintf_chr(char *s, lsTripleInt_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsTripleInt_fwrite(FILE *fp, lsTripleInt_t *il);
int lsTripleInt_fread(lsTripleInt_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




