#define LS_IS_TYPE
#ifndef LS_TYPE_INCLUDED
#define LS_TYPE_INCLUDED

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

typedef struct list_t {
  LIST_TYPE * list;
  int       n_list;
  int     max_list;
} list_t;

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

typedef LIST_TYPE (LIST_FOLD_TYPE)(LIST_TYPE,LIST_TYPE);
typedef LIST_TYPE (LIST_FOLD_2_TYPE)(LIST_TYPE,LIST_TYPE,void *);
typedef LIST_TYPE (LIST_MAP_TYPE)(LIST_TYPE);
typedef LIST_TYPE (LIST_MAP_2_TYPE)(LIST_TYPE,void *);
typedef LIST_TYPE (LIST_MAP_3_TYPE)(LIST_TYPE,void *,void *);
typedef int (LIST_FILTER_TYPE)(LIST_TYPE);
typedef int (LIST_CMP_TYPE)(LIST_TYPE,LIST_TYPE);
typedef int (LIST_CMP_2_TYPE)(LIST_TYPE,LIST_TYPE,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

list_t *  ls_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

list_t *  ls_realloc(list_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per ls_Nil neu allociert. */

list_t *  ls_nil(list_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

list_t *  ls_ConsNil(LIST_TYPE i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

list_t *  ls_setNil(list_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

list_t *  ls_setConsNil(list_t * il, LIST_TYPE i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

list_t *  ls_SetPt(int n, LIST_TYPE *items);
list_t *  ls_setPt(list_t * il_to, int n, LIST_TYPE *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

list_t *  ls_CpyPt(list_t * il_from);
list_t *  ls_cpyPt(list_t * il_to, list_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

list_t *  ls_Cpy(const list_t * il_from);
list_t *  ls_cpy(list_t * il_to, const list_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

list_t *  ls_Cat(list_t * il_1, list_t * il_2);
list_t *  ls_cat(list_t * il_to, list_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
list_t * ls_addCat(list_t *il_to, list_t *il);
/* Rueckgabe: ls_add(il_to, ls_Cat(il_to,il)) */
list_t * ls_AddCat(list_t *il_to, list_t *il);
/* Rueckgabe: ls_Add(il_to, ls_Cat(il_to,il)) */
#endif

void      ls_Free(list_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      ls_free(list_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

LIST_TYPE ls_setIndex(list_t *il, int index, LIST_TYPE i, LIST_TYPE i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

list_t * ls_setIndices(list_t *il, lsInt_t *indices, LIST_TYPE x,
		       LIST_TYPE undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von ls_setIndex: index < 0] */

list_t * ls_nsetIndex(list_t *il, int index, int n, LIST_TYPE x, 
		      LIST_TYPE undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von ls_setIndex: index < 0] */

LIST_TYPE *ls_getNewItem(list_t *il);
int        ls_getNewItemIndex(list_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   ls_pushLast(il,i) (ls_add(il,i))
list_t *  ls_add(list_t * il, LIST_TYPE i);
list_t *  ls_Add(list_t * il, LIST_TYPE i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   ls_pushHead(il,i) (ls_cons(il,i)
list_t *  ls_cons(list_t * il, LIST_TYPE i);
list_t *  ls_Cons(list_t * il, LIST_TYPE i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

LIST_TYPE ls_popLast(list_t *il, LIST_TYPE undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

LIST_TYPE ls_popHead(list_t *il, LIST_TYPE undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

list_t *  ls_init(list_t * il);
list_t *  ls_Init(list_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

list_t *  ls_tail(list_t * il);
list_t *  ls_tail(list_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

list_t * ls_take(list_t *il, int n);
list_t * ls_Take(list_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

list_t * ls_drop(list_t *il, int i);
list_t * ls_Drop(list_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

list_t * ls_dropPt(list_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

list_t * ls_split(list_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = ls_take(<il>,i)
 * - Rueckgabe ls_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * ls_nsplit(lsPt_t *il_split, list_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = ls_take(<il>,is[0])
 * - Rueckgabe[j] = ls_Drop(ls_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

list_t * ls_range(list_t *il, int i0, int iN);
list_t * ls_Range(list_t *il, int i0, int iN);
list_t * ls_rangePt(list_t *il, int i0, int iN);
list_t * ls_cpyRange(list_t *il_to, list_t *il, int i0, int iN);
list_t * ls_catRange(list_t *il_to, list_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

list_t * ls_insSwap(list_t *il, int index, LIST_TYPE i, LIST_TYPE i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

LIST_TYPE ls_delSwap(list_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

list_t * ls_insert(list_t *il, int index, LIST_TYPE i, LIST_TYPE i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

list_t * ls_insertN(list_t *il, int index, int n, LIST_TYPE i, LIST_TYPE i0);
/* Wie ls_insert, fuegt aber <n>-mal das Element <i> ein */

LIST_TYPE ls_delete(list_t *il, int index, LIST_TYPE undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
list_t * ls_join(list_t *il_to, list_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber ls_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
list_t * ls_joinInts(list_t *il_to, list_t *il_from, lsInt_t *value2index);
/* wie ls_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber ls_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

LIST_TYPE ls_last(list_t *il, LIST_TYPE undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

LIST_TYPE ls_head(list_t *il, LIST_TYPE undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

LIST_TYPE ls_get(list_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

LIST_TYPE ls_getCheck(list_t * il,int i, LIST_TYPE undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

LIST_TYPE ls_getFlip(int i, list_t *il, LIST_TYPE undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

list_t *  ls_getRowPt(list_t * row, list_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

list_t *  ls_getRow(list_t *row, list_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

list_t *  ls_getCol(list_t *col, list_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

list_t *  ls_setRow(list_t *il, list_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

list_t *  ls_setCol(list_t *il, list_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       ls_length(list_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int ls_elem(list_t * il, LIST_TYPE i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int ls_neqElem(list_t *il, LIST_TYPE i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int ls_disjoint(list_t *il1, list_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

list_t *ls_subst(list_t *il, LIST_TYPE i, LIST_TYPE j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int ls_subBag(list_t *il_sub, list_t *il_super, LIST_TYPE undef);
int ls_subBagIndices(lsInt_t *indices,
		     list_t *il_sub, list_t *il_super, LIST_TYPE undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int ls_subBagLimitedIndices(lsInt_t *indices,
			    list_t *il_sub, list_t *il_super, 
			    lsInt_t *limit, LIST_TYPE undef);
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

int ls_getIndex(list_t * il, LIST_TYPE item);
int ls_getFstIndex(list_t *il, LIST_TYPE item);
int ls_getLastIndex(list_t *il, LIST_TYPE item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int ls_getLastNeqIndex(list_t * il, LIST_TYPE item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int ls_getFstNeqIndex(list_t * il, LIST_TYPE item);
/* wie ls_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int ls_elemFunc(list_t * il, LIST_TYPE i, LIST_CMP_TYPE *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int ls_neqElemFunc(list_t *il, LIST_TYPE i, LIST_CMP_TYPE *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int ls_getIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int ls_getLastNeqIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int ls_getFstNeqIndexFunc(list_t * il, LIST_TYPE item, LIST_CMP_TYPE *func);
/* wie ls_getLastNeqIndex, Suche beginnt hier beim 1. Element */

LIST_TYPE ls_foldl(list_t *il, LIST_TYPE item0, LIST_FOLD_TYPE *func);
LIST_TYPE 
ls_foldl_2(list_t *il, LIST_TYPE item0, LIST_FOLD_2_TYPE *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * ls_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

LIST_TYPE ls_foldr(list_t *il, LIST_TYPE item0, LIST_FOLD_TYPE *func);
LIST_TYPE 
ls_foldr_2(list_t *il, LIST_TYPE item0, LIST_FOLD_2_TYPE *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * ls_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

LIST_TYPE ls_maxFunc(list_t *il, LIST_CMP_TYPE *func);
int       ls_maxIndexFunc(list_t *il, LIST_CMP_TYPE *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

LIST_TYPE ls_minFunc(list_t *il, LIST_CMP_TYPE *func);
int       ls_minIndexFunc(list_t *il, LIST_CMP_TYPE *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int ls_bsearchLtFunc(list_t *il, LIST_TYPE i, LIST_CMP_TYPE *func);
int ls_bsearchLtFunc_2(list_t *il,LIST_TYPE i,LIST_CMP_2_TYPE *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int ls_bsearchGtFunc(list_t *il, LIST_TYPE i, LIST_CMP_TYPE *func);
int ls_bsearchGtFunc_2(list_t *il,LIST_TYPE i,LIST_CMP_2_TYPE *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int ls_cmpFunc(list_t *il1, list_t *il2, LIST_CMP_TYPE *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int ls_cmpIndex(int i, int j, pairPt_t *arg);
int ls_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * ls_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * ls_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int ls_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int ls_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * ls_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * ls_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
LIST_TYPE ls_sum(list_t *il);
/* liefert die Summe ueber alle Listenelemente */

LIST_TYPE ls_prod(list_t *il);
/* liefert das Produkt ueber alle Listenelemente */

list_t * ls_scale(list_t *il, LIST_TYPE s);
/* skaliert jedes Element der List um Faktor s */

list_t *  ls_delta(list_t *il_to, list_t *il_from, LIST_TYPE base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

LIST_TYPE ls_max(list_t *il);
int       ls_maxIndex(list_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

LIST_TYPE ls_min(list_t *il);
int       ls_minIndex(list_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

list_t *ls_rmdup(list_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int ls_bsearchLt(list_t *il, LIST_TYPE i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int ls_bsearchGt(list_t *il, LIST_TYPE i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int ls_cmp(list_t *il1, list_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * ls_makeIndex(lsInt_t *index, list_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

list_t *ls_reverse(list_t *il);
/* Dreht die Elemente einer Liste um */

list_t *  ls_CpyMap(list_t * il_from, LIST_MAP_TYPE *func);
list_t *  ls_cpyMap(list_t * il_to, list_t * il_from, LIST_MAP_TYPE *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

list_t *  ls_map(list_t * il, LIST_MAP_TYPE *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

list_t *  ls_mapSet(list_t * il, LIST_MAP_TYPE *func);
/* wie ls_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

list_t *  ls_map_2(list_t * il, LIST_MAP_2_TYPE *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

list_t *  ls_map_3(list_t * il, LIST_MAP_3_TYPE *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

list_t *  ls_mapSet_2(list_t * il, LIST_MAP_2_TYPE *func, void *arg);
/* wie ls_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

list_t *  ls_mapSet_3(list_t * il,LIST_MAP_3_TYPE *func,void *arg1,void *arg2);
/* wie ls_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

list_t *  ls_CpyMap_2(list_t * il_from, LIST_MAP_2_TYPE *func, void *arg);
list_t *  ls_cpyMap_2(list_t * il_to, list_t * il_from, 
		      LIST_MAP_2_TYPE *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

list_t *  ls_CpyMap_3(list_t * il_from, LIST_MAP_3_TYPE *func, 
		      void *arg1, void *arg2);
list_t *  ls_cpyMap_3(list_t * il_to, list_t * il_from, 
		      LIST_MAP_3_TYPE *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

list_t * ls_CartProd(list_t *il1, list_t *il2, LIST_FOLD_TYPE *func);
list_t * ls_cartProd(list_t *il1, list_t *il2, LIST_FOLD_TYPE *func);
list_t * ls_cpyCartProd(list_t *il_to, 
			list_t *il1, list_t *il2, LIST_FOLD_TYPE *func);
list_t * ls_cartProd_2(list_t *il1, list_t *il2, LIST_FOLD_2_TYPE *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * ls_CartProd gibt eine neu allocierte List zurueck
 * ls_cartProd gibt il1 als Ergebnisliste zurueck
 * ls_cpyCartProd gibt il_to als Egebnislist zurueck
 * ls_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

list_t * ls_sortByIndex(list_t *il, lsInt_t *index, LIST_TYPE undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

list_t * ls_cpySortByIndex(list_t *il_to, list_t *il_from, 
			   lsInt_t *index, LIST_TYPE undef);
/* wie ls_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

list_t * ls_qsortLtFunc(list_t *il, LIST_CMP_TYPE *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_qsortGtFunc(list_t *il, LIST_CMP_TYPE *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_qsortLtFunc_2(list_t *il, LIST_CMP_2_TYPE *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_qsortGtFunc_2(list_t *il, LIST_CMP_2_TYPE *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * ls_qsortIndexLtFunc(lsInt_t *index, list_t *il, LIST_CMP_TYPE *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * ls_qsortIndexGtFunc(lsInt_t *index, list_t *il, LIST_CMP_TYPE *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_msortLtFunc(list_t *il, LIST_CMP_TYPE *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

list_t * ls_msortGtFunc(list_t *il, LIST_CMP_TYPE *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
list_t * ls_qsortLt(list_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_qsortGt(list_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * ls_qsortIndexLt(lsInt_t *index, list_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * ls_qsortIndexGt(lsInt_t *index, list_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

list_t * ls_msortLt(list_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

list_t * ls_msortGt(list_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
list_t * ls_filterByValue(list_t *il, LIST_TYPE undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

list_t * ls_CpyFilterByValue(list_t *il, LIST_TYPE undef, lsInt_t *indices);
/* wie ls_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

list_t * ls_cpyFilterByValue(list_t *il_to, list_t *il_from, LIST_TYPE undef, 
			     lsInt_t *indices);
/* wie ls_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

list_t * ls_filterByIndex(list_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

list_t * ls_CpyFilterByIndex(list_t *il, lsInt_t *indices);
/* wie ls_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

list_t * ls_cpyFilterByIndex(list_t *il_to, list_t *il_from, lsInt_t *indices);
/* wie ls_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

list_t *  ls_filter(list_t *il, LIST_FILTER_TYPE *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

list_t *  ls_CpyFilter(list_t *il_from, LIST_FILTER_TYPE *func);
list_t *  ls_cpyFilter(list_t *il_to, list_t *il_from, LIST_FILTER_TYPE *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
list_t * ls_sscan_chr(list_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * ls_sprint_chr(char *s, list_t *il, char t);
char * ls_sprintf_chr(char *s, list_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int ls_fwrite(FILE *fp, list_t *il);
int ls_fread(list_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




