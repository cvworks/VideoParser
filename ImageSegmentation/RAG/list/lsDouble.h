#define LS_IS_lsDouble
#ifndef LS_lsDouble_INCLUDED
#define LS_lsDouble_INCLUDED

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

typedef struct lsDouble_t {
  double * list;
  int       n_list;
  int     max_list;
} lsDouble_t;

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

typedef double (lsDouble_fold_t)(double,double);
typedef double (lsDouble_fold_2_t)(double,double,void *);
typedef double (lsDouble_map_t)(double);
typedef double (lsDouble_map_2_t)(double,void *);
typedef double (lsDouble_map_3_t)(double,void *,void *);
typedef int (lsDouble_filter_t)(double);
typedef int (lsDouble_cmp_t)(double,double);
typedef int (lsDouble_cmp_2_t)(double,double,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsDouble_t *  lsDouble_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsDouble_t *  lsDouble_realloc(lsDouble_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsDouble_Nil neu allociert. */

lsDouble_t *  lsDouble_nil(lsDouble_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsDouble_t *  lsDouble_ConsNil(double i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsDouble_t *  lsDouble_setNil(lsDouble_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsDouble_t *  lsDouble_setConsNil(lsDouble_t * il, double i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsDouble_t *  lsDouble_SetPt(int n, double *items);
lsDouble_t *  lsDouble_setPt(lsDouble_t * il_to, int n, double *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsDouble_t *  lsDouble_CpyPt(lsDouble_t * il_from);
lsDouble_t *  lsDouble_cpyPt(lsDouble_t * il_to, lsDouble_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsDouble_t *  lsDouble_Cpy(const lsDouble_t * il_from);
lsDouble_t *  lsDouble_cpy(lsDouble_t * il_to, const lsDouble_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsDouble_t *  lsDouble_Cat(lsDouble_t * il_1, lsDouble_t * il_2);
lsDouble_t *  lsDouble_cat(lsDouble_t * il_to, lsDouble_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsDouble_t * lsDouble_addCat(lsDouble_t *il_to, lsDouble_t *il);
/* Rueckgabe: lsDouble_add(il_to, lsDouble_Cat(il_to,il)) */
lsDouble_t * lsDouble_AddCat(lsDouble_t *il_to, lsDouble_t *il);
/* Rueckgabe: lsDouble_Add(il_to, lsDouble_Cat(il_to,il)) */
#endif

void      lsDouble_Free(lsDouble_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsDouble_free(lsDouble_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

double lsDouble_setIndex(lsDouble_t *il, int index, double i, double i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsDouble_t * lsDouble_setIndices(lsDouble_t *il, lsInt_t *indices, double x,
		       double undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsDouble_setIndex: index < 0] */

lsDouble_t * lsDouble_nsetIndex(lsDouble_t *il, int index, int n, double x, 
		      double undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsDouble_setIndex: index < 0] */

double *lsDouble_getNewItem(lsDouble_t *il);
int        lsDouble_getNewItemIndex(lsDouble_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsDouble_pushLast(il,i) (lsDouble_add(il,i))
lsDouble_t *  lsDouble_add(lsDouble_t * il, double i);
lsDouble_t *  lsDouble_Add(lsDouble_t * il, double i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsDouble_pushHead(il,i) (lsDouble_cons(il,i)
lsDouble_t *  lsDouble_cons(lsDouble_t * il, double i);
lsDouble_t *  lsDouble_Cons(lsDouble_t * il, double i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

double lsDouble_popLast(lsDouble_t *il, double undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

double lsDouble_popHead(lsDouble_t *il, double undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsDouble_t *  lsDouble_init(lsDouble_t * il);
lsDouble_t *  lsDouble_Init(lsDouble_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsDouble_t *  lsDouble_tail(lsDouble_t * il);
lsDouble_t *  lsDouble_tail(lsDouble_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsDouble_t * lsDouble_take(lsDouble_t *il, int n);
lsDouble_t * lsDouble_Take(lsDouble_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsDouble_t * lsDouble_drop(lsDouble_t *il, int i);
lsDouble_t * lsDouble_Drop(lsDouble_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsDouble_t * lsDouble_dropPt(lsDouble_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsDouble_t * lsDouble_split(lsDouble_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsDouble_take(<il>,i)
 * - Rueckgabe lsDouble_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsDouble_nsplit(lsPt_t *il_split, lsDouble_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsDouble_take(<il>,is[0])
 * - Rueckgabe[j] = lsDouble_Drop(lsDouble_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsDouble_t * lsDouble_range(lsDouble_t *il, int i0, int iN);
lsDouble_t * lsDouble_Range(lsDouble_t *il, int i0, int iN);
lsDouble_t * lsDouble_rangePt(lsDouble_t *il, int i0, int iN);
lsDouble_t * lsDouble_cpyRange(lsDouble_t *il_to, lsDouble_t *il, int i0, int iN);
lsDouble_t * lsDouble_catRange(lsDouble_t *il_to, lsDouble_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsDouble_t * lsDouble_insSwap(lsDouble_t *il, int index, double i, double i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

double lsDouble_delSwap(lsDouble_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsDouble_t * lsDouble_insert(lsDouble_t *il, int index, double i, double i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsDouble_t * lsDouble_insertN(lsDouble_t *il, int index, int n, double i, double i0);
/* Wie lsDouble_insert, fuegt aber <n>-mal das Element <i> ein */

double lsDouble_delete(lsDouble_t *il, int index, double undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsDouble_t * lsDouble_join(lsDouble_t *il_to, lsDouble_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsDouble_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsDouble_t * lsDouble_joinInts(lsDouble_t *il_to, lsDouble_t *il_from, lsInt_t *value2index);
/* wie lsDouble_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsDouble_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

double lsDouble_last(lsDouble_t *il, double undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

double lsDouble_head(lsDouble_t *il, double undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

double lsDouble_get(lsDouble_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

double lsDouble_getCheck(lsDouble_t * il,int i, double undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

double lsDouble_getFlip(int i, lsDouble_t *il, double undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsDouble_t *  lsDouble_getRowPt(lsDouble_t * row, lsDouble_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsDouble_t *  lsDouble_getRow(lsDouble_t *row, lsDouble_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsDouble_t *  lsDouble_getCol(lsDouble_t *col, lsDouble_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsDouble_t *  lsDouble_setRow(lsDouble_t *il, lsDouble_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsDouble_t *  lsDouble_setCol(lsDouble_t *il, lsDouble_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsDouble_length(lsDouble_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsDouble_elem(lsDouble_t * il, double i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsDouble_neqElem(lsDouble_t *il, double i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsDouble_disjoint(lsDouble_t *il1, lsDouble_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsDouble_t *lsDouble_subst(lsDouble_t *il, double i, double j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsDouble_subBag(lsDouble_t *il_sub, lsDouble_t *il_super, double undef);
int lsDouble_subBagIndices(lsInt_t *indices,
		     lsDouble_t *il_sub, lsDouble_t *il_super, double undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsDouble_subBagLimitedIndices(lsInt_t *indices,
			    lsDouble_t *il_sub, lsDouble_t *il_super, 
			    lsInt_t *limit, double undef);
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

int lsDouble_getIndex(lsDouble_t * il, double item);
int lsDouble_getFstIndex(lsDouble_t *il, double item);
int lsDouble_getLastIndex(lsDouble_t *il, double item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsDouble_getLastNeqIndex(lsDouble_t * il, double item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsDouble_getFstNeqIndex(lsDouble_t * il, double item);
/* wie lsDouble_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsDouble_elemFunc(lsDouble_t * il, double i, lsDouble_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsDouble_neqElemFunc(lsDouble_t *il, double i, lsDouble_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsDouble_getIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsDouble_getLastNeqIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsDouble_getFstNeqIndexFunc(lsDouble_t * il, double item, lsDouble_cmp_t *func);
/* wie lsDouble_getLastNeqIndex, Suche beginnt hier beim 1. Element */

double lsDouble_foldl(lsDouble_t *il, double item0, lsDouble_fold_t *func);
double 
lsDouble_foldl_2(lsDouble_t *il, double item0, lsDouble_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsDouble_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

double lsDouble_foldr(lsDouble_t *il, double item0, lsDouble_fold_t *func);
double 
lsDouble_foldr_2(lsDouble_t *il, double item0, lsDouble_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsDouble_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

double lsDouble_maxFunc(lsDouble_t *il, lsDouble_cmp_t *func);
int       lsDouble_maxIndexFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

double lsDouble_minFunc(lsDouble_t *il, lsDouble_cmp_t *func);
int       lsDouble_minIndexFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsDouble_bsearchLtFunc(lsDouble_t *il, double i, lsDouble_cmp_t *func);
int lsDouble_bsearchLtFunc_2(lsDouble_t *il,double i,lsDouble_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsDouble_bsearchGtFunc(lsDouble_t *il, double i, lsDouble_cmp_t *func);
int lsDouble_bsearchGtFunc_2(lsDouble_t *il,double i,lsDouble_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsDouble_cmpFunc(lsDouble_t *il1, lsDouble_t *il2, lsDouble_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsDouble_cmpIndex(int i, int j, pairPt_t *arg);
int lsDouble_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsDouble_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsDouble_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsDouble_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsDouble_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsDouble_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsDouble_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
double lsDouble_sum(lsDouble_t *il);
/* liefert die Summe ueber alle Listenelemente */

double lsDouble_prod(lsDouble_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsDouble_t * lsDouble_scale(lsDouble_t *il, double s);
/* skaliert jedes Element der List um Faktor s */

lsDouble_t *  lsDouble_delta(lsDouble_t *il_to, lsDouble_t *il_from, double base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

double lsDouble_max(lsDouble_t *il);
int       lsDouble_maxIndex(lsDouble_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

double lsDouble_min(lsDouble_t *il);
int       lsDouble_minIndex(lsDouble_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsDouble_t *lsDouble_rmdup(lsDouble_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsDouble_bsearchLt(lsDouble_t *il, double i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsDouble_bsearchGt(lsDouble_t *il, double i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsDouble_cmp(lsDouble_t *il1, lsDouble_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsDouble_makeIndex(lsInt_t *index, lsDouble_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsDouble_t *lsDouble_reverse(lsDouble_t *il);
/* Dreht die Elemente einer Liste um */

lsDouble_t *  lsDouble_CpyMap(lsDouble_t * il_from, lsDouble_map_t *func);
lsDouble_t *  lsDouble_cpyMap(lsDouble_t * il_to, lsDouble_t * il_from, lsDouble_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsDouble_t *  lsDouble_map(lsDouble_t * il, lsDouble_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsDouble_t *  lsDouble_mapSet(lsDouble_t * il, lsDouble_map_t *func);
/* wie lsDouble_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsDouble_t *  lsDouble_map_2(lsDouble_t * il, lsDouble_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsDouble_t *  lsDouble_map_3(lsDouble_t * il, lsDouble_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsDouble_t *  lsDouble_mapSet_2(lsDouble_t * il, lsDouble_map_2_t *func, void *arg);
/* wie lsDouble_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsDouble_t *  lsDouble_mapSet_3(lsDouble_t * il,lsDouble_map_3_t *func,void *arg1,void *arg2);
/* wie lsDouble_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsDouble_t *  lsDouble_CpyMap_2(lsDouble_t * il_from, lsDouble_map_2_t *func, void *arg);
lsDouble_t *  lsDouble_cpyMap_2(lsDouble_t * il_to, lsDouble_t * il_from, 
		      lsDouble_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsDouble_t *  lsDouble_CpyMap_3(lsDouble_t * il_from, lsDouble_map_3_t *func, 
		      void *arg1, void *arg2);
lsDouble_t *  lsDouble_cpyMap_3(lsDouble_t * il_to, lsDouble_t * il_from, 
		      lsDouble_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsDouble_t * lsDouble_CartProd(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func);
lsDouble_t * lsDouble_cartProd(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func);
lsDouble_t * lsDouble_cpyCartProd(lsDouble_t *il_to, 
			lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_t *func);
lsDouble_t * lsDouble_cartProd_2(lsDouble_t *il1, lsDouble_t *il2, lsDouble_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsDouble_CartProd gibt eine neu allocierte List zurueck
 * lsDouble_cartProd gibt il1 als Ergebnisliste zurueck
 * lsDouble_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsDouble_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsDouble_t * lsDouble_sortByIndex(lsDouble_t *il, lsInt_t *index, double undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsDouble_t * lsDouble_cpySortByIndex(lsDouble_t *il_to, lsDouble_t *il_from, 
			   lsInt_t *index, double undef);
/* wie lsDouble_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsDouble_t * lsDouble_qsortLtFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_qsortGtFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_qsortLtFunc_2(lsDouble_t *il, lsDouble_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_qsortGtFunc_2(lsDouble_t *il, lsDouble_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsDouble_qsortIndexLtFunc(lsInt_t *index, lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsDouble_qsortIndexGtFunc(lsInt_t *index, lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_msortLtFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsDouble_t * lsDouble_msortGtFunc(lsDouble_t *il, lsDouble_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsDouble_t * lsDouble_qsortLt(lsDouble_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_qsortGt(lsDouble_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsDouble_qsortIndexLt(lsInt_t *index, lsDouble_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsDouble_qsortIndexGt(lsInt_t *index, lsDouble_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsDouble_t * lsDouble_msortLt(lsDouble_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsDouble_t * lsDouble_msortGt(lsDouble_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsDouble_t * lsDouble_filterByValue(lsDouble_t *il, double undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsDouble_t * lsDouble_CpyFilterByValue(lsDouble_t *il, double undef, lsInt_t *indices);
/* wie lsDouble_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsDouble_t * lsDouble_cpyFilterByValue(lsDouble_t *il_to, lsDouble_t *il_from, double undef, 
			     lsInt_t *indices);
/* wie lsDouble_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsDouble_t * lsDouble_filterByIndex(lsDouble_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsDouble_t * lsDouble_CpyFilterByIndex(lsDouble_t *il, lsInt_t *indices);
/* wie lsDouble_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsDouble_t * lsDouble_cpyFilterByIndex(lsDouble_t *il_to, lsDouble_t *il_from, lsInt_t *indices);
/* wie lsDouble_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsDouble_t *  lsDouble_filter(lsDouble_t *il, lsDouble_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsDouble_t *  lsDouble_CpyFilter(lsDouble_t *il_from, lsDouble_filter_t *func);
lsDouble_t *  lsDouble_cpyFilter(lsDouble_t *il_to, lsDouble_t *il_from, lsDouble_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsDouble_t * lsDouble_sscan_chr(lsDouble_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsDouble_sprint_chr(char *s, lsDouble_t *il, char t);
char * lsDouble_sprintf_chr(char *s, lsDouble_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsDouble_fwrite(FILE *fp, lsDouble_t *il);
int lsDouble_fread(lsDouble_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




