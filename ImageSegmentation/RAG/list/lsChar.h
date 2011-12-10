#define LS_IS_lsChar
#ifndef LS_lsChar_INCLUDED
#define LS_lsChar_INCLUDED

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

typedef struct lsChar_t {
  char * list;
  int       n_list;
  int     max_list;
} lsChar_t;

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

typedef char (lsChar_fold_t)(char,char);
typedef char (lsChar_fold_2_t)(char,char,void *);
typedef char (lsChar_map_t)(char);
typedef char (lsChar_map_2_t)(char,void *);
typedef char (lsChar_map_3_t)(char,void *,void *);
typedef int (lsChar_filter_t)(char);
typedef int (lsChar_cmp_t)(char,char);
typedef int (lsChar_cmp_2_t)(char,char,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsChar_t *  lsChar_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsChar_t *  lsChar_realloc(lsChar_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsChar_Nil neu allociert. */

lsChar_t *  lsChar_nil(lsChar_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsChar_t *  lsChar_ConsNil(char i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsChar_t *  lsChar_setNil(lsChar_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsChar_t *  lsChar_setConsNil(lsChar_t * il, char i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsChar_t *  lsChar_SetPt(int n, char *items);
lsChar_t *  lsChar_setPt(lsChar_t * il_to, int n, char *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsChar_t *  lsChar_CpyPt(lsChar_t * il_from);
lsChar_t *  lsChar_cpyPt(lsChar_t * il_to, lsChar_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsChar_t *  lsChar_Cpy(const lsChar_t * il_from);
lsChar_t *  lsChar_cpy(lsChar_t * il_to, const lsChar_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsChar_t *  lsChar_Cat(lsChar_t * il_1, lsChar_t * il_2);
lsChar_t *  lsChar_cat(lsChar_t * il_to, lsChar_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsChar_t * lsChar_addCat(lsChar_t *il_to, lsChar_t *il);
/* Rueckgabe: lsChar_add(il_to, lsChar_Cat(il_to,il)) */
lsChar_t * lsChar_AddCat(lsChar_t *il_to, lsChar_t *il);
/* Rueckgabe: lsChar_Add(il_to, lsChar_Cat(il_to,il)) */
#endif

void      lsChar_Free(lsChar_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsChar_free(lsChar_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

char lsChar_setIndex(lsChar_t *il, int index, char i, char i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsChar_t * lsChar_setIndices(lsChar_t *il, lsInt_t *indices, char x,
		       char undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsChar_setIndex: index < 0] */

lsChar_t * lsChar_nsetIndex(lsChar_t *il, int index, int n, char x, 
		      char undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsChar_setIndex: index < 0] */

char *lsChar_getNewItem(lsChar_t *il);
int        lsChar_getNewItemIndex(lsChar_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsChar_pushLast(il,i) (lsChar_add(il,i))
lsChar_t *  lsChar_add(lsChar_t * il, char i);
lsChar_t *  lsChar_Add(lsChar_t * il, char i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsChar_pushHead(il,i) (lsChar_cons(il,i)
lsChar_t *  lsChar_cons(lsChar_t * il, char i);
lsChar_t *  lsChar_Cons(lsChar_t * il, char i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

char lsChar_popLast(lsChar_t *il, char undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

char lsChar_popHead(lsChar_t *il, char undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsChar_t *  lsChar_init(lsChar_t * il);
lsChar_t *  lsChar_Init(lsChar_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsChar_t *  lsChar_tail(lsChar_t * il);
lsChar_t *  lsChar_tail(lsChar_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsChar_t * lsChar_take(lsChar_t *il, int n);
lsChar_t * lsChar_Take(lsChar_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsChar_t * lsChar_drop(lsChar_t *il, int i);
lsChar_t * lsChar_Drop(lsChar_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsChar_t * lsChar_dropPt(lsChar_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsChar_t * lsChar_split(lsChar_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsChar_take(<il>,i)
 * - Rueckgabe lsChar_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsChar_nsplit(lsPt_t *il_split, lsChar_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsChar_take(<il>,is[0])
 * - Rueckgabe[j] = lsChar_Drop(lsChar_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsChar_t * lsChar_range(lsChar_t *il, int i0, int iN);
lsChar_t * lsChar_Range(lsChar_t *il, int i0, int iN);
lsChar_t * lsChar_rangePt(lsChar_t *il, int i0, int iN);
lsChar_t * lsChar_cpyRange(lsChar_t *il_to, lsChar_t *il, int i0, int iN);
lsChar_t * lsChar_catRange(lsChar_t *il_to, lsChar_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsChar_t * lsChar_insSwap(lsChar_t *il, int index, char i, char i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

char lsChar_delSwap(lsChar_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsChar_t * lsChar_insert(lsChar_t *il, int index, char i, char i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsChar_t * lsChar_insertN(lsChar_t *il, int index, int n, char i, char i0);
/* Wie lsChar_insert, fuegt aber <n>-mal das Element <i> ein */

char lsChar_delete(lsChar_t *il, int index, char undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsChar_t * lsChar_join(lsChar_t *il_to, lsChar_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsChar_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsChar_t * lsChar_joinInts(lsChar_t *il_to, lsChar_t *il_from, lsInt_t *value2index);
/* wie lsChar_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsChar_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

char lsChar_last(lsChar_t *il, char undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

char lsChar_head(lsChar_t *il, char undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

char lsChar_get(lsChar_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

char lsChar_getCheck(lsChar_t * il,int i, char undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

char lsChar_getFlip(int i, lsChar_t *il, char undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsChar_t *  lsChar_getRowPt(lsChar_t * row, lsChar_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsChar_t *  lsChar_getRow(lsChar_t *row, lsChar_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsChar_t *  lsChar_getCol(lsChar_t *col, lsChar_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsChar_t *  lsChar_setRow(lsChar_t *il, lsChar_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsChar_t *  lsChar_setCol(lsChar_t *il, lsChar_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsChar_length(lsChar_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsChar_elem(lsChar_t * il, char i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsChar_neqElem(lsChar_t *il, char i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsChar_disjoint(lsChar_t *il1, lsChar_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsChar_t *lsChar_subst(lsChar_t *il, char i, char j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsChar_subBag(lsChar_t *il_sub, lsChar_t *il_super, char undef);
int lsChar_subBagIndices(lsInt_t *indices,
		     lsChar_t *il_sub, lsChar_t *il_super, char undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsChar_subBagLimitedIndices(lsInt_t *indices,
			    lsChar_t *il_sub, lsChar_t *il_super, 
			    lsInt_t *limit, char undef);
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

int lsChar_getIndex(lsChar_t * il, char item);
int lsChar_getFstIndex(lsChar_t *il, char item);
int lsChar_getLastIndex(lsChar_t *il, char item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsChar_getLastNeqIndex(lsChar_t * il, char item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsChar_getFstNeqIndex(lsChar_t * il, char item);
/* wie lsChar_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsChar_elemFunc(lsChar_t * il, char i, lsChar_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsChar_neqElemFunc(lsChar_t *il, char i, lsChar_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsChar_getIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsChar_getLastNeqIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsChar_getFstNeqIndexFunc(lsChar_t * il, char item, lsChar_cmp_t *func);
/* wie lsChar_getLastNeqIndex, Suche beginnt hier beim 1. Element */

char lsChar_foldl(lsChar_t *il, char item0, lsChar_fold_t *func);
char 
lsChar_foldl_2(lsChar_t *il, char item0, lsChar_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsChar_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

char lsChar_foldr(lsChar_t *il, char item0, lsChar_fold_t *func);
char 
lsChar_foldr_2(lsChar_t *il, char item0, lsChar_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsChar_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

char lsChar_maxFunc(lsChar_t *il, lsChar_cmp_t *func);
int       lsChar_maxIndexFunc(lsChar_t *il, lsChar_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

char lsChar_minFunc(lsChar_t *il, lsChar_cmp_t *func);
int       lsChar_minIndexFunc(lsChar_t *il, lsChar_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsChar_bsearchLtFunc(lsChar_t *il, char i, lsChar_cmp_t *func);
int lsChar_bsearchLtFunc_2(lsChar_t *il,char i,lsChar_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsChar_bsearchGtFunc(lsChar_t *il, char i, lsChar_cmp_t *func);
int lsChar_bsearchGtFunc_2(lsChar_t *il,char i,lsChar_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsChar_cmpFunc(lsChar_t *il1, lsChar_t *il2, lsChar_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsChar_cmpIndex(int i, int j, pairPt_t *arg);
int lsChar_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsChar_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsChar_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsChar_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsChar_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsChar_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsChar_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
char lsChar_sum(lsChar_t *il);
/* liefert die Summe ueber alle Listenelemente */

char lsChar_prod(lsChar_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsChar_t * lsChar_scale(lsChar_t *il, char s);
/* skaliert jedes Element der List um Faktor s */

lsChar_t *  lsChar_delta(lsChar_t *il_to, lsChar_t *il_from, char base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

char lsChar_max(lsChar_t *il);
int       lsChar_maxIndex(lsChar_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

char lsChar_min(lsChar_t *il);
int       lsChar_minIndex(lsChar_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsChar_t *lsChar_rmdup(lsChar_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsChar_bsearchLt(lsChar_t *il, char i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsChar_bsearchGt(lsChar_t *il, char i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsChar_cmp(lsChar_t *il1, lsChar_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsChar_makeIndex(lsInt_t *index, lsChar_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsChar_t *lsChar_reverse(lsChar_t *il);
/* Dreht die Elemente einer Liste um */

lsChar_t *  lsChar_CpyMap(lsChar_t * il_from, lsChar_map_t *func);
lsChar_t *  lsChar_cpyMap(lsChar_t * il_to, lsChar_t * il_from, lsChar_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsChar_t *  lsChar_map(lsChar_t * il, lsChar_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsChar_t *  lsChar_mapSet(lsChar_t * il, lsChar_map_t *func);
/* wie lsChar_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsChar_t *  lsChar_map_2(lsChar_t * il, lsChar_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsChar_t *  lsChar_map_3(lsChar_t * il, lsChar_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsChar_t *  lsChar_mapSet_2(lsChar_t * il, lsChar_map_2_t *func, void *arg);
/* wie lsChar_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsChar_t *  lsChar_mapSet_3(lsChar_t * il,lsChar_map_3_t *func,void *arg1,void *arg2);
/* wie lsChar_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsChar_t *  lsChar_CpyMap_2(lsChar_t * il_from, lsChar_map_2_t *func, void *arg);
lsChar_t *  lsChar_cpyMap_2(lsChar_t * il_to, lsChar_t * il_from, 
		      lsChar_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsChar_t *  lsChar_CpyMap_3(lsChar_t * il_from, lsChar_map_3_t *func, 
		      void *arg1, void *arg2);
lsChar_t *  lsChar_cpyMap_3(lsChar_t * il_to, lsChar_t * il_from, 
		      lsChar_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsChar_t * lsChar_CartProd(lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func);
lsChar_t * lsChar_cartProd(lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func);
lsChar_t * lsChar_cpyCartProd(lsChar_t *il_to, 
			lsChar_t *il1, lsChar_t *il2, lsChar_fold_t *func);
lsChar_t * lsChar_cartProd_2(lsChar_t *il1, lsChar_t *il2, lsChar_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsChar_CartProd gibt eine neu allocierte List zurueck
 * lsChar_cartProd gibt il1 als Ergebnisliste zurueck
 * lsChar_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsChar_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsChar_t * lsChar_sortByIndex(lsChar_t *il, lsInt_t *index, char undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsChar_t * lsChar_cpySortByIndex(lsChar_t *il_to, lsChar_t *il_from, 
			   lsInt_t *index, char undef);
/* wie lsChar_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsChar_t * lsChar_qsortLtFunc(lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_qsortGtFunc(lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_qsortLtFunc_2(lsChar_t *il, lsChar_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_qsortGtFunc_2(lsChar_t *il, lsChar_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsChar_qsortIndexLtFunc(lsInt_t *index, lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsChar_qsortIndexGtFunc(lsInt_t *index, lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_msortLtFunc(lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsChar_t * lsChar_msortGtFunc(lsChar_t *il, lsChar_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsChar_t * lsChar_qsortLt(lsChar_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_qsortGt(lsChar_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsChar_qsortIndexLt(lsInt_t *index, lsChar_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsChar_qsortIndexGt(lsInt_t *index, lsChar_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsChar_t * lsChar_msortLt(lsChar_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsChar_t * lsChar_msortGt(lsChar_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsChar_t * lsChar_filterByValue(lsChar_t *il, char undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsChar_t * lsChar_CpyFilterByValue(lsChar_t *il, char undef, lsInt_t *indices);
/* wie lsChar_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsChar_t * lsChar_cpyFilterByValue(lsChar_t *il_to, lsChar_t *il_from, char undef, 
			     lsInt_t *indices);
/* wie lsChar_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsChar_t * lsChar_filterByIndex(lsChar_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsChar_t * lsChar_CpyFilterByIndex(lsChar_t *il, lsInt_t *indices);
/* wie lsChar_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsChar_t * lsChar_cpyFilterByIndex(lsChar_t *il_to, lsChar_t *il_from, lsInt_t *indices);
/* wie lsChar_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsChar_t *  lsChar_filter(lsChar_t *il, lsChar_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsChar_t *  lsChar_CpyFilter(lsChar_t *il_from, lsChar_filter_t *func);
lsChar_t *  lsChar_cpyFilter(lsChar_t *il_to, lsChar_t *il_from, lsChar_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsChar_t * lsChar_sscan_chr(lsChar_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsChar_sprint_chr(char *s, lsChar_t *il, char t);
char * lsChar_sprintf_chr(char *s, lsChar_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsChar_fwrite(FILE *fp, lsChar_t *il);
int lsChar_fread(lsChar_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




