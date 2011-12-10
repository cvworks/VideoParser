#define LS_IS_lsFloat
#ifndef LS_lsFloat_INCLUDED
#define LS_lsFloat_INCLUDED

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

typedef struct lsFloat_t {
  float * list;
  int       n_list;
  int     max_list;
} lsFloat_t;

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

typedef float (lsFloat_fold_t)(float,float);
typedef float (lsFloat_fold_2_t)(float,float,void *);
typedef float (lsFloat_map_t)(float);
typedef float (lsFloat_map_2_t)(float,void *);
typedef float (lsFloat_map_3_t)(float,void *,void *);
typedef int (lsFloat_filter_t)(float);
typedef int (lsFloat_cmp_t)(float,float);
typedef int (lsFloat_cmp_2_t)(float,float,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsFloat_t *  lsFloat_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsFloat_t *  lsFloat_realloc(lsFloat_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsFloat_Nil neu allociert. */

lsFloat_t *  lsFloat_nil(lsFloat_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsFloat_t *  lsFloat_ConsNil(float i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsFloat_t *  lsFloat_setNil(lsFloat_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsFloat_t *  lsFloat_setConsNil(lsFloat_t * il, float i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsFloat_t *  lsFloat_SetPt(int n, float *items);
lsFloat_t *  lsFloat_setPt(lsFloat_t * il_to, int n, float *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsFloat_t *  lsFloat_CpyPt(lsFloat_t * il_from);
lsFloat_t *  lsFloat_cpyPt(lsFloat_t * il_to, lsFloat_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsFloat_t *  lsFloat_Cpy(const lsFloat_t * il_from);
lsFloat_t *  lsFloat_cpy(lsFloat_t * il_to, const lsFloat_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsFloat_t *  lsFloat_Cat(lsFloat_t * il_1, lsFloat_t * il_2);
lsFloat_t *  lsFloat_cat(lsFloat_t * il_to, lsFloat_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsFloat_t * lsFloat_addCat(lsFloat_t *il_to, lsFloat_t *il);
/* Rueckgabe: lsFloat_add(il_to, lsFloat_Cat(il_to,il)) */
lsFloat_t * lsFloat_AddCat(lsFloat_t *il_to, lsFloat_t *il);
/* Rueckgabe: lsFloat_Add(il_to, lsFloat_Cat(il_to,il)) */
#endif

void      lsFloat_Free(lsFloat_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsFloat_free(lsFloat_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

float lsFloat_setIndex(lsFloat_t *il, int index, float i, float i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsFloat_t * lsFloat_setIndices(lsFloat_t *il, lsInt_t *indices, float x,
		       float undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsFloat_setIndex: index < 0] */

lsFloat_t * lsFloat_nsetIndex(lsFloat_t *il, int index, int n, float x, 
		      float undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsFloat_setIndex: index < 0] */

float *lsFloat_getNewItem(lsFloat_t *il);
int        lsFloat_getNewItemIndex(lsFloat_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsFloat_pushLast(il,i) (lsFloat_add(il,i))
lsFloat_t *  lsFloat_add(lsFloat_t * il, float i);
lsFloat_t *  lsFloat_Add(lsFloat_t * il, float i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsFloat_pushHead(il,i) (lsFloat_cons(il,i)
lsFloat_t *  lsFloat_cons(lsFloat_t * il, float i);
lsFloat_t *  lsFloat_Cons(lsFloat_t * il, float i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

float lsFloat_popLast(lsFloat_t *il, float undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

float lsFloat_popHead(lsFloat_t *il, float undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsFloat_t *  lsFloat_init(lsFloat_t * il);
lsFloat_t *  lsFloat_Init(lsFloat_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsFloat_t *  lsFloat_tail(lsFloat_t * il);
lsFloat_t *  lsFloat_tail(lsFloat_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsFloat_t * lsFloat_take(lsFloat_t *il, int n);
lsFloat_t * lsFloat_Take(lsFloat_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsFloat_t * lsFloat_drop(lsFloat_t *il, int i);
lsFloat_t * lsFloat_Drop(lsFloat_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsFloat_t * lsFloat_dropPt(lsFloat_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsFloat_t * lsFloat_split(lsFloat_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsFloat_take(<il>,i)
 * - Rueckgabe lsFloat_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsFloat_nsplit(lsPt_t *il_split, lsFloat_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsFloat_take(<il>,is[0])
 * - Rueckgabe[j] = lsFloat_Drop(lsFloat_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsFloat_t * lsFloat_range(lsFloat_t *il, int i0, int iN);
lsFloat_t * lsFloat_Range(lsFloat_t *il, int i0, int iN);
lsFloat_t * lsFloat_rangePt(lsFloat_t *il, int i0, int iN);
lsFloat_t * lsFloat_cpyRange(lsFloat_t *il_to, lsFloat_t *il, int i0, int iN);
lsFloat_t * lsFloat_catRange(lsFloat_t *il_to, lsFloat_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsFloat_t * lsFloat_insSwap(lsFloat_t *il, int index, float i, float i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

float lsFloat_delSwap(lsFloat_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsFloat_t * lsFloat_insert(lsFloat_t *il, int index, float i, float i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsFloat_t * lsFloat_insertN(lsFloat_t *il, int index, int n, float i, float i0);
/* Wie lsFloat_insert, fuegt aber <n>-mal das Element <i> ein */

float lsFloat_delete(lsFloat_t *il, int index, float undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsFloat_t * lsFloat_join(lsFloat_t *il_to, lsFloat_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsFloat_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsFloat_t * lsFloat_joinInts(lsFloat_t *il_to, lsFloat_t *il_from, lsInt_t *value2index);
/* wie lsFloat_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsFloat_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

float lsFloat_last(lsFloat_t *il, float undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

float lsFloat_head(lsFloat_t *il, float undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

float lsFloat_get(lsFloat_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

float lsFloat_getCheck(lsFloat_t * il,int i, float undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

float lsFloat_getFlip(int i, lsFloat_t *il, float undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsFloat_t *  lsFloat_getRowPt(lsFloat_t * row, lsFloat_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsFloat_t *  lsFloat_getRow(lsFloat_t *row, lsFloat_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsFloat_t *  lsFloat_getCol(lsFloat_t *col, lsFloat_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsFloat_t *  lsFloat_setRow(lsFloat_t *il, lsFloat_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsFloat_t *  lsFloat_setCol(lsFloat_t *il, lsFloat_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsFloat_length(lsFloat_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsFloat_elem(lsFloat_t * il, float i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsFloat_neqElem(lsFloat_t *il, float i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsFloat_disjoint(lsFloat_t *il1, lsFloat_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsFloat_t *lsFloat_subst(lsFloat_t *il, float i, float j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsFloat_subBag(lsFloat_t *il_sub, lsFloat_t *il_super, float undef);
int lsFloat_subBagIndices(lsInt_t *indices,
		     lsFloat_t *il_sub, lsFloat_t *il_super, float undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsFloat_subBagLimitedIndices(lsInt_t *indices,
			    lsFloat_t *il_sub, lsFloat_t *il_super, 
			    lsInt_t *limit, float undef);
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

int lsFloat_getIndex(lsFloat_t * il, float item);
int lsFloat_getFstIndex(lsFloat_t *il, float item);
int lsFloat_getLastIndex(lsFloat_t *il, float item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsFloat_getLastNeqIndex(lsFloat_t * il, float item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsFloat_getFstNeqIndex(lsFloat_t * il, float item);
/* wie lsFloat_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsFloat_elemFunc(lsFloat_t * il, float i, lsFloat_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsFloat_neqElemFunc(lsFloat_t *il, float i, lsFloat_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsFloat_getIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsFloat_getLastNeqIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsFloat_getFstNeqIndexFunc(lsFloat_t * il, float item, lsFloat_cmp_t *func);
/* wie lsFloat_getLastNeqIndex, Suche beginnt hier beim 1. Element */

float lsFloat_foldl(lsFloat_t *il, float item0, lsFloat_fold_t *func);
float 
lsFloat_foldl_2(lsFloat_t *il, float item0, lsFloat_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsFloat_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

float lsFloat_foldr(lsFloat_t *il, float item0, lsFloat_fold_t *func);
float 
lsFloat_foldr_2(lsFloat_t *il, float item0, lsFloat_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsFloat_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

float lsFloat_maxFunc(lsFloat_t *il, lsFloat_cmp_t *func);
int       lsFloat_maxIndexFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

float lsFloat_minFunc(lsFloat_t *il, lsFloat_cmp_t *func);
int       lsFloat_minIndexFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsFloat_bsearchLtFunc(lsFloat_t *il, float i, lsFloat_cmp_t *func);
int lsFloat_bsearchLtFunc_2(lsFloat_t *il,float i,lsFloat_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsFloat_bsearchGtFunc(lsFloat_t *il, float i, lsFloat_cmp_t *func);
int lsFloat_bsearchGtFunc_2(lsFloat_t *il,float i,lsFloat_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsFloat_cmpFunc(lsFloat_t *il1, lsFloat_t *il2, lsFloat_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsFloat_cmpIndex(int i, int j, pairPt_t *arg);
int lsFloat_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsFloat_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsFloat_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsFloat_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsFloat_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsFloat_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsFloat_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
float lsFloat_sum(lsFloat_t *il);
/* liefert die Summe ueber alle Listenelemente */

float lsFloat_prod(lsFloat_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsFloat_t * lsFloat_scale(lsFloat_t *il, float s);
/* skaliert jedes Element der List um Faktor s */

lsFloat_t *  lsFloat_delta(lsFloat_t *il_to, lsFloat_t *il_from, float base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

float lsFloat_max(lsFloat_t *il);
int       lsFloat_maxIndex(lsFloat_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

float lsFloat_min(lsFloat_t *il);
int       lsFloat_minIndex(lsFloat_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsFloat_t *lsFloat_rmdup(lsFloat_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsFloat_bsearchLt(lsFloat_t *il, float i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsFloat_bsearchGt(lsFloat_t *il, float i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsFloat_cmp(lsFloat_t *il1, lsFloat_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsFloat_makeIndex(lsInt_t *index, lsFloat_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsFloat_t *lsFloat_reverse(lsFloat_t *il);
/* Dreht die Elemente einer Liste um */

lsFloat_t *  lsFloat_CpyMap(lsFloat_t * il_from, lsFloat_map_t *func);
lsFloat_t *  lsFloat_cpyMap(lsFloat_t * il_to, lsFloat_t * il_from, lsFloat_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsFloat_t *  lsFloat_map(lsFloat_t * il, lsFloat_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsFloat_t *  lsFloat_mapSet(lsFloat_t * il, lsFloat_map_t *func);
/* wie lsFloat_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsFloat_t *  lsFloat_map_2(lsFloat_t * il, lsFloat_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsFloat_t *  lsFloat_map_3(lsFloat_t * il, lsFloat_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsFloat_t *  lsFloat_mapSet_2(lsFloat_t * il, lsFloat_map_2_t *func, void *arg);
/* wie lsFloat_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsFloat_t *  lsFloat_mapSet_3(lsFloat_t * il,lsFloat_map_3_t *func,void *arg1,void *arg2);
/* wie lsFloat_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsFloat_t *  lsFloat_CpyMap_2(lsFloat_t * il_from, lsFloat_map_2_t *func, void *arg);
lsFloat_t *  lsFloat_cpyMap_2(lsFloat_t * il_to, lsFloat_t * il_from, 
		      lsFloat_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsFloat_t *  lsFloat_CpyMap_3(lsFloat_t * il_from, lsFloat_map_3_t *func, 
		      void *arg1, void *arg2);
lsFloat_t *  lsFloat_cpyMap_3(lsFloat_t * il_to, lsFloat_t * il_from, 
		      lsFloat_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsFloat_t * lsFloat_CartProd(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func);
lsFloat_t * lsFloat_cartProd(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func);
lsFloat_t * lsFloat_cpyCartProd(lsFloat_t *il_to, 
			lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_t *func);
lsFloat_t * lsFloat_cartProd_2(lsFloat_t *il1, lsFloat_t *il2, lsFloat_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsFloat_CartProd gibt eine neu allocierte List zurueck
 * lsFloat_cartProd gibt il1 als Ergebnisliste zurueck
 * lsFloat_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsFloat_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsFloat_t * lsFloat_sortByIndex(lsFloat_t *il, lsInt_t *index, float undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsFloat_t * lsFloat_cpySortByIndex(lsFloat_t *il_to, lsFloat_t *il_from, 
			   lsInt_t *index, float undef);
/* wie lsFloat_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsFloat_t * lsFloat_qsortLtFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_qsortGtFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_qsortLtFunc_2(lsFloat_t *il, lsFloat_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_qsortGtFunc_2(lsFloat_t *il, lsFloat_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsFloat_qsortIndexLtFunc(lsInt_t *index, lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsFloat_qsortIndexGtFunc(lsInt_t *index, lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_msortLtFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsFloat_t * lsFloat_msortGtFunc(lsFloat_t *il, lsFloat_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsFloat_t * lsFloat_qsortLt(lsFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_qsortGt(lsFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsFloat_qsortIndexLt(lsInt_t *index, lsFloat_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsFloat_qsortIndexGt(lsInt_t *index, lsFloat_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsFloat_t * lsFloat_msortLt(lsFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsFloat_t * lsFloat_msortGt(lsFloat_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsFloat_t * lsFloat_filterByValue(lsFloat_t *il, float undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsFloat_t * lsFloat_CpyFilterByValue(lsFloat_t *il, float undef, lsInt_t *indices);
/* wie lsFloat_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsFloat_t * lsFloat_cpyFilterByValue(lsFloat_t *il_to, lsFloat_t *il_from, float undef, 
			     lsInt_t *indices);
/* wie lsFloat_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsFloat_t * lsFloat_filterByIndex(lsFloat_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsFloat_t * lsFloat_CpyFilterByIndex(lsFloat_t *il, lsInt_t *indices);
/* wie lsFloat_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsFloat_t * lsFloat_cpyFilterByIndex(lsFloat_t *il_to, lsFloat_t *il_from, lsInt_t *indices);
/* wie lsFloat_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsFloat_t *  lsFloat_filter(lsFloat_t *il, lsFloat_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsFloat_t *  lsFloat_CpyFilter(lsFloat_t *il_from, lsFloat_filter_t *func);
lsFloat_t *  lsFloat_cpyFilter(lsFloat_t *il_to, lsFloat_t *il_from, lsFloat_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsFloat_t * lsFloat_sscan_chr(lsFloat_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsFloat_sprint_chr(char *s, lsFloat_t *il, char t);
char * lsFloat_sprintf_chr(char *s, lsFloat_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsFloat_fwrite(FILE *fp, lsFloat_t *il);
int lsFloat_fread(lsFloat_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




