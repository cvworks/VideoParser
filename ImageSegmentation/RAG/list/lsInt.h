#define LS_IS_lsInt
#ifndef LS_lsInt_INCLUDED
#define LS_lsInt_INCLUDED

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

typedef struct lsInt_t {
  int * list;
  int       n_list;
  int     max_list;
} lsInt_t;

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

typedef int (lsInt_fold_t)(int,int);
typedef int (lsInt_fold_2_t)(int,int,void *);
typedef int (lsInt_map_t)(int);
typedef int (lsInt_map_2_t)(int,void *);
typedef int (lsInt_map_3_t)(int,void *,void *);
typedef int (lsInt_filter_t)(int);
typedef int (lsInt_cmp_t)(int,int);
typedef int (lsInt_cmp_2_t)(int,int,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsInt_t *  lsInt_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsInt_t *  lsInt_realloc(lsInt_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsInt_Nil neu allociert. */

lsInt_t *  lsInt_nil(lsInt_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsInt_t *  lsInt_ConsNil(int i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsInt_t *  lsInt_setNil(lsInt_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsInt_t *  lsInt_setConsNil(lsInt_t * il, int i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsInt_t *  lsInt_SetPt(int n, int *items);
lsInt_t *  lsInt_setPt(lsInt_t * il_to, int n, int *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsInt_t *  lsInt_CpyPt(lsInt_t * il_from);
lsInt_t *  lsInt_cpyPt(lsInt_t * il_to, lsInt_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsInt_t *  lsInt_Cpy(const lsInt_t * il_from);
lsInt_t *  lsInt_cpy(lsInt_t * il_to, const lsInt_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsInt_t *  lsInt_Cat(lsInt_t * il_1, lsInt_t * il_2);
lsInt_t *  lsInt_cat(lsInt_t * il_to, lsInt_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsInt_t * lsInt_addCat(lsInt_t *il_to, lsInt_t *il);
/* Rueckgabe: lsInt_add(il_to, lsInt_Cat(il_to,il)) */
lsInt_t * lsInt_AddCat(lsInt_t *il_to, lsInt_t *il);
/* Rueckgabe: lsInt_Add(il_to, lsInt_Cat(il_to,il)) */
#endif

void      lsInt_Free(lsInt_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsInt_free(lsInt_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

int lsInt_setIndex(lsInt_t *il, int index, int i, int i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsInt_t * lsInt_setIndices(lsInt_t *il, lsInt_t *indices, int x,
		       int undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsInt_setIndex: index < 0] */

lsInt_t * lsInt_nsetIndex(lsInt_t *il, int index, int n, int x, 
		      int undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsInt_setIndex: index < 0] */

int *lsInt_getNewItem(lsInt_t *il);
int        lsInt_getNewItemIndex(lsInt_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsInt_pushLast(il,i) (lsInt_add(il,i))
lsInt_t *  lsInt_add(lsInt_t * il, int i);
lsInt_t *  lsInt_Add(lsInt_t * il, int i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsInt_pushHead(il,i) (lsInt_cons(il,i)
lsInt_t *  lsInt_cons(lsInt_t * il, int i);
lsInt_t *  lsInt_Cons(lsInt_t * il, int i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

int lsInt_popLast(lsInt_t *il, int undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

int lsInt_popHead(lsInt_t *il, int undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsInt_t *  lsInt_init(lsInt_t * il);
lsInt_t *  lsInt_Init(lsInt_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsInt_t *  lsInt_tail(lsInt_t * il);
lsInt_t *  lsInt_tail(lsInt_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsInt_t * lsInt_take(lsInt_t *il, int n);
lsInt_t * lsInt_Take(lsInt_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsInt_t * lsInt_drop(lsInt_t *il, int i);
lsInt_t * lsInt_Drop(lsInt_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsInt_t * lsInt_dropPt(lsInt_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsInt_t * lsInt_split(lsInt_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsInt_take(<il>,i)
 * - Rueckgabe lsInt_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsInt_nsplit(lsPt_t *il_split, lsInt_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsInt_take(<il>,is[0])
 * - Rueckgabe[j] = lsInt_Drop(lsInt_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsInt_t * lsInt_range(lsInt_t *il, int i0, int iN);
lsInt_t * lsInt_Range(lsInt_t *il, int i0, int iN);
lsInt_t * lsInt_rangePt(lsInt_t *il, int i0, int iN);
lsInt_t * lsInt_cpyRange(lsInt_t *il_to, lsInt_t *il, int i0, int iN);
lsInt_t * lsInt_catRange(lsInt_t *il_to, lsInt_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsInt_t * lsInt_insSwap(lsInt_t *il, int index, int i, int i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

int lsInt_delSwap(lsInt_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsInt_t * lsInt_insert(lsInt_t *il, int index, int i, int i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsInt_t * lsInt_insertN(lsInt_t *il, int index, int n, int i, int i0);
/* Wie lsInt_insert, fuegt aber <n>-mal das Element <i> ein */

int lsInt_delete(lsInt_t *il, int index, int undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsInt_t * lsInt_join(lsInt_t *il_to, lsInt_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsInt_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsInt_joinInts(lsInt_t *il_to, lsInt_t *il_from, lsInt_t *value2index);
/* wie lsInt_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsInt_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

int lsInt_last(lsInt_t *il, int undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

int lsInt_head(lsInt_t *il, int undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

int lsInt_get(lsInt_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

int lsInt_getCheck(lsInt_t * il,int i, int undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

int lsInt_getFlip(int i, lsInt_t *il, int undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsInt_t *  lsInt_getRowPt(lsInt_t * row, lsInt_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsInt_t *  lsInt_getRow(lsInt_t *row, lsInt_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsInt_t *  lsInt_getCol(lsInt_t *col, lsInt_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsInt_t *  lsInt_setRow(lsInt_t *il, lsInt_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsInt_t *  lsInt_setCol(lsInt_t *il, lsInt_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsInt_length(lsInt_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsInt_elem(lsInt_t * il, int i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsInt_neqElem(lsInt_t *il, int i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsInt_disjoint(lsInt_t *il1, lsInt_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsInt_t *lsInt_subst(lsInt_t *il, int i, int j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsInt_subBag(lsInt_t *il_sub, lsInt_t *il_super, int undef);
int lsInt_subBagIndices(lsInt_t *indices,
		     lsInt_t *il_sub, lsInt_t *il_super, int undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsInt_subBagLimitedIndices(lsInt_t *indices,
			    lsInt_t *il_sub, lsInt_t *il_super, 
			    lsInt_t *limit, int undef);
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

int lsInt_getIndex(lsInt_t * il, int item);
int lsInt_getFstIndex(lsInt_t *il, int item);
int lsInt_getLastIndex(lsInt_t *il, int item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsInt_getLastNeqIndex(lsInt_t * il, int item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsInt_getFstNeqIndex(lsInt_t * il, int item);
/* wie lsInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsInt_elemFunc(lsInt_t * il, int i, lsInt_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsInt_neqElemFunc(lsInt_t *il, int i, lsInt_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsInt_getIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsInt_getLastNeqIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsInt_getFstNeqIndexFunc(lsInt_t * il, int item, lsInt_cmp_t *func);
/* wie lsInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */

int lsInt_foldl(lsInt_t *il, int item0, lsInt_fold_t *func);
int 
lsInt_foldl_2(lsInt_t *il, int item0, lsInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsInt_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

int lsInt_foldr(lsInt_t *il, int item0, lsInt_fold_t *func);
int 
lsInt_foldr_2(lsInt_t *il, int item0, lsInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsInt_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

int lsInt_maxFunc(lsInt_t *il, lsInt_cmp_t *func);
int       lsInt_maxIndexFunc(lsInt_t *il, lsInt_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsInt_minFunc(lsInt_t *il, lsInt_cmp_t *func);
int       lsInt_minIndexFunc(lsInt_t *il, lsInt_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsInt_bsearchLtFunc(lsInt_t *il, int i, lsInt_cmp_t *func);
int lsInt_bsearchLtFunc_2(lsInt_t *il,int i,lsInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsInt_bsearchGtFunc(lsInt_t *il, int i, lsInt_cmp_t *func);
int lsInt_bsearchGtFunc_2(lsInt_t *il,int i,lsInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsInt_cmpFunc(lsInt_t *il1, lsInt_t *il2, lsInt_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsInt_cmpIndex(int i, int j, pairPt_t *arg);
int lsInt_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsInt_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsInt_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsInt_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsInt_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsInt_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
int lsInt_sum(lsInt_t *il);
/* liefert die Summe ueber alle Listenelemente */

int lsInt_prod(lsInt_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsInt_t * lsInt_scale(lsInt_t *il, int s);
/* skaliert jedes Element der List um Faktor s */

lsInt_t *  lsInt_delta(lsInt_t *il_to, lsInt_t *il_from, int base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

int lsInt_max(lsInt_t *il);
int       lsInt_maxIndex(lsInt_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsInt_min(lsInt_t *il);
int       lsInt_minIndex(lsInt_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsInt_t *lsInt_rmdup(lsInt_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsInt_bsearchLt(lsInt_t *il, int i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsInt_bsearchGt(lsInt_t *il, int i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsInt_cmp(lsInt_t *il1, lsInt_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsInt_makeIndex(lsInt_t *index, lsInt_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsInt_t *lsInt_reverse(lsInt_t *il);
/* Dreht die Elemente einer Liste um */

lsInt_t *  lsInt_CpyMap(lsInt_t * il_from, lsInt_map_t *func);
lsInt_t *  lsInt_cpyMap(lsInt_t * il_to, lsInt_t * il_from, lsInt_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsInt_t *  lsInt_map(lsInt_t * il, lsInt_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsInt_t *  lsInt_mapSet(lsInt_t * il, lsInt_map_t *func);
/* wie lsInt_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsInt_t *  lsInt_map_2(lsInt_t * il, lsInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsInt_t *  lsInt_map_3(lsInt_t * il, lsInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsInt_t *  lsInt_mapSet_2(lsInt_t * il, lsInt_map_2_t *func, void *arg);
/* wie lsInt_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsInt_t *  lsInt_mapSet_3(lsInt_t * il,lsInt_map_3_t *func,void *arg1,void *arg2);
/* wie lsInt_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsInt_t *  lsInt_CpyMap_2(lsInt_t * il_from, lsInt_map_2_t *func, void *arg);
lsInt_t *  lsInt_cpyMap_2(lsInt_t * il_to, lsInt_t * il_from, 
		      lsInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsInt_t *  lsInt_CpyMap_3(lsInt_t * il_from, lsInt_map_3_t *func, 
		      void *arg1, void *arg2);
lsInt_t *  lsInt_cpyMap_3(lsInt_t * il_to, lsInt_t * il_from, 
		      lsInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsInt_t * lsInt_CartProd(lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func);
lsInt_t * lsInt_cartProd(lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func);
lsInt_t * lsInt_cpyCartProd(lsInt_t *il_to, 
			lsInt_t *il1, lsInt_t *il2, lsInt_fold_t *func);
lsInt_t * lsInt_cartProd_2(lsInt_t *il1, lsInt_t *il2, lsInt_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsInt_CartProd gibt eine neu allocierte List zurueck
 * lsInt_cartProd gibt il1 als Ergebnisliste zurueck
 * lsInt_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsInt_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsInt_t * lsInt_sortByIndex(lsInt_t *il, lsInt_t *index, int undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsInt_t * lsInt_cpySortByIndex(lsInt_t *il_to, lsInt_t *il_from, 
			   lsInt_t *index, int undef);
/* wie lsInt_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsInt_t * lsInt_qsortLtFunc(lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortGtFunc(lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortLtFunc_2(lsInt_t *il, lsInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortGtFunc_2(lsInt_t *il, lsInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortIndexLtFunc(lsInt_t *index, lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortIndexGtFunc(lsInt_t *index, lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_msortLtFunc(lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsInt_t * lsInt_msortGtFunc(lsInt_t *il, lsInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsInt_t * lsInt_qsortLt(lsInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortGt(lsInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortIndexLt(lsInt_t *index, lsInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_qsortIndexGt(lsInt_t *index, lsInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsInt_msortLt(lsInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsInt_t * lsInt_msortGt(lsInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsInt_t * lsInt_filterByValue(lsInt_t *il, int undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsInt_t * lsInt_CpyFilterByValue(lsInt_t *il, int undef, lsInt_t *indices);
/* wie lsInt_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsInt_t * lsInt_cpyFilterByValue(lsInt_t *il_to, lsInt_t *il_from, int undef, 
			     lsInt_t *indices);
/* wie lsInt_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsInt_t * lsInt_filterByIndex(lsInt_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsInt_t * lsInt_CpyFilterByIndex(lsInt_t *il, lsInt_t *indices);
/* wie lsInt_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsInt_t * lsInt_cpyFilterByIndex(lsInt_t *il_to, lsInt_t *il_from, lsInt_t *indices);
/* wie lsInt_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsInt_t *  lsInt_filter(lsInt_t *il, lsInt_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsInt_t *  lsInt_CpyFilter(lsInt_t *il_from, lsInt_filter_t *func);
lsInt_t *  lsInt_cpyFilter(lsInt_t *il_to, lsInt_t *il_from, lsInt_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsInt_t * lsInt_sscan_chr(lsInt_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsInt_sprint_chr(char *s, lsInt_t *il, char t);
char * lsInt_sprintf_chr(char *s, lsInt_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsInt_fwrite(FILE *fp, lsInt_t *il);
int lsInt_fread(lsInt_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




