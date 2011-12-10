#define LS_IS_lsPt
#ifndef LS_lsPt_INCLUDED
#define LS_lsPt_INCLUDED

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

typedef struct lsPt_t {
  void * * list;
  int       n_list;
  int     max_list;
} lsPt_t;

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

typedef void * (lsPt_fold_t)(void *,void *);
typedef void * (lsPt_fold_2_t)(void *,void *,void *);
typedef void * (lsPt_map_t)(void *);
typedef void * (lsPt_map_2_t)(void *,void *);
typedef void * (lsPt_map_3_t)(void *,void *,void *);
typedef int (lsPt_filter_t)(void *);
typedef int (lsPt_cmp_t)(void *,void *);
typedef int (lsPt_cmp_2_t)(void *,void *,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsPt_t *  lsPt_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsPt_t *  lsPt_realloc(lsPt_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsPt_Nil neu allociert. */

lsPt_t *  lsPt_nil(lsPt_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsPt_t *  lsPt_ConsNil(void * i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsPt_t *  lsPt_setNil(lsPt_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPt_t *  lsPt_setConsNil(lsPt_t * il, void * i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPt_t *  lsPt_SetPt(int n, void * *items);
lsPt_t *  lsPt_setPt(lsPt_t * il_to, int n, void * *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPt_t *  lsPt_CpyPt(lsPt_t * il_from);
lsPt_t *  lsPt_cpyPt(lsPt_t * il_to, lsPt_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPt_t *  lsPt_Cpy(const lsPt_t * il_from);
lsPt_t *  lsPt_cpy(lsPt_t * il_to, const lsPt_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsPt_t *  lsPt_Cat(lsPt_t * il_1, lsPt_t * il_2);
lsPt_t *  lsPt_cat(lsPt_t * il_to, lsPt_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsPt_t * lsPt_addCat(lsPt_t *il_to, lsPt_t *il);
/* Rueckgabe: lsPt_add(il_to, lsPt_Cat(il_to,il)) */
lsPt_t * lsPt_AddCat(lsPt_t *il_to, lsPt_t *il);
/* Rueckgabe: lsPt_Add(il_to, lsPt_Cat(il_to,il)) */
#endif

void      lsPt_Free(lsPt_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsPt_free(lsPt_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

void * lsPt_setIndex(lsPt_t *il, int index, void * i, void * i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsPt_t * lsPt_setIndices(lsPt_t *il, lsInt_t *indices, void * x,
		       void * undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPt_setIndex: index < 0] */

lsPt_t * lsPt_nsetIndex(lsPt_t *il, int index, int n, void * x, 
		      void * undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsPt_setIndex: index < 0] */

void * *lsPt_getNewItem(lsPt_t *il);
int        lsPt_getNewItemIndex(lsPt_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsPt_pushLast(il,i) (lsPt_add(il,i))
lsPt_t *  lsPt_add(lsPt_t * il, void * i);
lsPt_t *  lsPt_Add(lsPt_t * il, void * i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsPt_pushHead(il,i) (lsPt_cons(il,i)
lsPt_t *  lsPt_cons(lsPt_t * il, void * i);
lsPt_t *  lsPt_Cons(lsPt_t * il, void * i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

void * lsPt_popLast(lsPt_t *il, void * undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

void * lsPt_popHead(lsPt_t *il, void * undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsPt_t *  lsPt_init(lsPt_t * il);
lsPt_t *  lsPt_Init(lsPt_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPt_t *  lsPt_tail(lsPt_t * il);
lsPt_t *  lsPt_tail(lsPt_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPt_t * lsPt_take(lsPt_t *il, int n);
lsPt_t * lsPt_Take(lsPt_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsPt_t * lsPt_drop(lsPt_t *il, int i);
lsPt_t * lsPt_Drop(lsPt_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsPt_t * lsPt_dropPt(lsPt_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsPt_t * lsPt_split(lsPt_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsPt_take(<il>,i)
 * - Rueckgabe lsPt_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsPt_nsplit(lsPt_t *il_split, lsPt_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsPt_take(<il>,is[0])
 * - Rueckgabe[j] = lsPt_Drop(lsPt_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsPt_t * lsPt_range(lsPt_t *il, int i0, int iN);
lsPt_t * lsPt_Range(lsPt_t *il, int i0, int iN);
lsPt_t * lsPt_rangePt(lsPt_t *il, int i0, int iN);
lsPt_t * lsPt_cpyRange(lsPt_t *il_to, lsPt_t *il, int i0, int iN);
lsPt_t * lsPt_catRange(lsPt_t *il_to, lsPt_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsPt_t * lsPt_insSwap(lsPt_t *il, int index, void * i, void * i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

void * lsPt_delSwap(lsPt_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsPt_t * lsPt_insert(lsPt_t *il, int index, void * i, void * i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsPt_t * lsPt_insertN(lsPt_t *il, int index, int n, void * i, void * i0);
/* Wie lsPt_insert, fuegt aber <n>-mal das Element <i> ein */

void * lsPt_delete(lsPt_t *il, int index, void * undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsPt_t * lsPt_join(lsPt_t *il_to, lsPt_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsPt_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsPt_t * lsPt_joinInts(lsPt_t *il_to, lsPt_t *il_from, lsInt_t *value2index);
/* wie lsPt_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsPt_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

void * lsPt_last(lsPt_t *il, void * undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

void * lsPt_head(lsPt_t *il, void * undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

void * lsPt_get(lsPt_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

void * lsPt_getCheck(lsPt_t * il,int i, void * undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

void * lsPt_getFlip(int i, lsPt_t *il, void * undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsPt_t *  lsPt_getRowPt(lsPt_t * row, lsPt_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsPt_t *  lsPt_getRow(lsPt_t *row, lsPt_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsPt_t *  lsPt_getCol(lsPt_t *col, lsPt_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsPt_t *  lsPt_setRow(lsPt_t *il, lsPt_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsPt_t *  lsPt_setCol(lsPt_t *il, lsPt_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsPt_length(lsPt_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsPt_elem(lsPt_t * il, void * i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPt_neqElem(lsPt_t *il, void * i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPt_disjoint(lsPt_t *il1, lsPt_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsPt_t *lsPt_subst(lsPt_t *il, void * i, void * j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsPt_subBag(lsPt_t *il_sub, lsPt_t *il_super, void * undef);
int lsPt_subBagIndices(lsInt_t *indices,
		     lsPt_t *il_sub, lsPt_t *il_super, void * undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsPt_subBagLimitedIndices(lsInt_t *indices,
			    lsPt_t *il_sub, lsPt_t *il_super, 
			    lsInt_t *limit, void * undef);
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

int lsPt_getIndex(lsPt_t * il, void * item);
int lsPt_getFstIndex(lsPt_t *il, void * item);
int lsPt_getLastIndex(lsPt_t *il, void * item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPt_getLastNeqIndex(lsPt_t * il, void * item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsPt_getFstNeqIndex(lsPt_t * il, void * item);
/* wie lsPt_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsPt_elemFunc(lsPt_t * il, void * i, lsPt_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPt_neqElemFunc(lsPt_t *il, void * i, lsPt_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsPt_getIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsPt_getLastNeqIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsPt_getFstNeqIndexFunc(lsPt_t * il, void * item, lsPt_cmp_t *func);
/* wie lsPt_getLastNeqIndex, Suche beginnt hier beim 1. Element */

void * lsPt_foldl(lsPt_t *il, void * item0, lsPt_fold_t *func);
void * 
lsPt_foldl_2(lsPt_t *il, void * item0, lsPt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsPt_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

void * lsPt_foldr(lsPt_t *il, void * item0, lsPt_fold_t *func);
void * 
lsPt_foldr_2(lsPt_t *il, void * item0, lsPt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsPt_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

void * lsPt_maxFunc(lsPt_t *il, lsPt_cmp_t *func);
int       lsPt_maxIndexFunc(lsPt_t *il, lsPt_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

void * lsPt_minFunc(lsPt_t *il, lsPt_cmp_t *func);
int       lsPt_minIndexFunc(lsPt_t *il, lsPt_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsPt_bsearchLtFunc(lsPt_t *il, void * i, lsPt_cmp_t *func);
int lsPt_bsearchLtFunc_2(lsPt_t *il,void * i,lsPt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPt_bsearchGtFunc(lsPt_t *il, void * i, lsPt_cmp_t *func);
int lsPt_bsearchGtFunc_2(lsPt_t *il,void * i,lsPt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsPt_cmpFunc(lsPt_t *il1, lsPt_t *il2, lsPt_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsPt_cmpIndex(int i, int j, pairPt_t *arg);
int lsPt_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsPt_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsPt_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsPt_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsPt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsPt_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsPt_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
void * lsPt_sum(lsPt_t *il);
/* liefert die Summe ueber alle Listenelemente */

void * lsPt_prod(lsPt_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsPt_t * lsPt_scale(lsPt_t *il, void * s);
/* skaliert jedes Element der List um Faktor s */

lsPt_t *  lsPt_delta(lsPt_t *il_to, lsPt_t *il_from, void * base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

void * lsPt_max(lsPt_t *il);
int       lsPt_maxIndex(lsPt_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

void * lsPt_min(lsPt_t *il);
int       lsPt_minIndex(lsPt_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsPt_t *lsPt_rmdup(lsPt_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsPt_bsearchLt(lsPt_t *il, void * i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPt_bsearchGt(lsPt_t *il, void * i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsPt_cmp(lsPt_t *il1, lsPt_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsPt_makeIndex(lsInt_t *index, lsPt_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsPt_t *lsPt_reverse(lsPt_t *il);
/* Dreht die Elemente einer Liste um */

lsPt_t *  lsPt_CpyMap(lsPt_t * il_from, lsPt_map_t *func);
lsPt_t *  lsPt_cpyMap(lsPt_t * il_to, lsPt_t * il_from, lsPt_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPt_t *  lsPt_map(lsPt_t * il, lsPt_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsPt_t *  lsPt_mapSet(lsPt_t * il, lsPt_map_t *func);
/* wie lsPt_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPt_t *  lsPt_map_2(lsPt_t * il, lsPt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsPt_t *  lsPt_map_3(lsPt_t * il, lsPt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsPt_t *  lsPt_mapSet_2(lsPt_t * il, lsPt_map_2_t *func, void *arg);
/* wie lsPt_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsPt_t *  lsPt_mapSet_3(lsPt_t * il,lsPt_map_3_t *func,void *arg1,void *arg2);
/* wie lsPt_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsPt_t *  lsPt_CpyMap_2(lsPt_t * il_from, lsPt_map_2_t *func, void *arg);
lsPt_t *  lsPt_cpyMap_2(lsPt_t * il_to, lsPt_t * il_from, 
		      lsPt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPt_t *  lsPt_CpyMap_3(lsPt_t * il_from, lsPt_map_3_t *func, 
		      void *arg1, void *arg2);
lsPt_t *  lsPt_cpyMap_3(lsPt_t * il_to, lsPt_t * il_from, 
		      lsPt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsPt_t * lsPt_CartProd(lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func);
lsPt_t * lsPt_cartProd(lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func);
lsPt_t * lsPt_cpyCartProd(lsPt_t *il_to, 
			lsPt_t *il1, lsPt_t *il2, lsPt_fold_t *func);
lsPt_t * lsPt_cartProd_2(lsPt_t *il1, lsPt_t *il2, lsPt_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsPt_CartProd gibt eine neu allocierte List zurueck
 * lsPt_cartProd gibt il1 als Ergebnisliste zurueck
 * lsPt_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsPt_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsPt_t * lsPt_sortByIndex(lsPt_t *il, lsInt_t *index, void * undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsPt_t * lsPt_cpySortByIndex(lsPt_t *il_to, lsPt_t *il_from, 
			   lsInt_t *index, void * undef);
/* wie lsPt_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsPt_t * lsPt_qsortLtFunc(lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_qsortGtFunc(lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_qsortLtFunc_2(lsPt_t *il, lsPt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_qsortGtFunc_2(lsPt_t *il, lsPt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPt_qsortIndexLtFunc(lsInt_t *index, lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPt_qsortIndexGtFunc(lsInt_t *index, lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_msortLtFunc(lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsPt_t * lsPt_msortGtFunc(lsPt_t *il, lsPt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPt_t * lsPt_qsortLt(lsPt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_qsortGt(lsPt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPt_qsortIndexLt(lsInt_t *index, lsPt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsPt_qsortIndexGt(lsInt_t *index, lsPt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsPt_t * lsPt_msortLt(lsPt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsPt_t * lsPt_msortGt(lsPt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsPt_t * lsPt_filterByValue(lsPt_t *il, void * undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsPt_t * lsPt_CpyFilterByValue(lsPt_t *il, void * undef, lsInt_t *indices);
/* wie lsPt_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPt_t * lsPt_cpyFilterByValue(lsPt_t *il_to, lsPt_t *il_from, void * undef, 
			     lsInt_t *indices);
/* wie lsPt_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsPt_t * lsPt_filterByIndex(lsPt_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsPt_t * lsPt_CpyFilterByIndex(lsPt_t *il, lsInt_t *indices);
/* wie lsPt_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsPt_t * lsPt_cpyFilterByIndex(lsPt_t *il_to, lsPt_t *il_from, lsInt_t *indices);
/* wie lsPt_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsPt_t *  lsPt_filter(lsPt_t *il, lsPt_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsPt_t *  lsPt_CpyFilter(lsPt_t *il_from, lsPt_filter_t *func);
lsPt_t *  lsPt_cpyFilter(lsPt_t *il_to, lsPt_t *il_from, lsPt_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsPt_t * lsPt_sscan_chr(lsPt_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsPt_sprint_chr(char *s, lsPt_t *il, char t);
char * lsPt_sprintf_chr(char *s, lsPt_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsPt_fwrite(FILE *fp, lsPt_t *il);
int lsPt_fread(lsPt_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




