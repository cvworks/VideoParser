#define LS_IS_lsULInt
#ifndef LS_lsULInt_INCLUDED
#define LS_lsULInt_INCLUDED

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

typedef struct lsULInt_t {
  unsigned long int * list;
  int       n_list;
  int     max_list;
} lsULInt_t;

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

typedef unsigned long int (lsULInt_fold_t)(unsigned long int,unsigned long int);
typedef unsigned long int (lsULInt_fold_2_t)(unsigned long int,unsigned long int,void *);
typedef unsigned long int (lsULInt_map_t)(unsigned long int);
typedef unsigned long int (lsULInt_map_2_t)(unsigned long int,void *);
typedef unsigned long int (lsULInt_map_3_t)(unsigned long int,void *,void *);
typedef int (lsULInt_filter_t)(unsigned long int);
typedef int (lsULInt_cmp_t)(unsigned long int,unsigned long int);
typedef int (lsULInt_cmp_2_t)(unsigned long int,unsigned long int,void *);

/*
 * LISTEN ERZEUGEN UND FREIGEBEN ------------------------------------
 */

lsULInt_t *  lsULInt_Nil(void);
/* Allocieren und Initialisieren einer neuen Liste */

lsULInt_t *  lsULInt_realloc(lsULInt_t *il, int n);
/* Allocieren von <n> Listenelementen 
 * Falls <il>==NULL wird die Liste per lsULInt_Nil neu allociert. */

lsULInt_t *  lsULInt_nil(lsULInt_t *il);
/* Initialisieren einer neuen Liste (bei Null-Pointer allocieren)
 * [Achtung: neuer Speicher fuer Listen-Elemente, keine Freigabe] */ 

lsULInt_t *  lsULInt_ConsNil(unsigned long int i);
/* Allocieren und Initialisieren einer ein-elementigen Liste */

lsULInt_t *  lsULInt_setNil(lsULInt_t * il);
/* Setzt Anzahl Listen-Elemente auf Null
 * Falls Liste Null-Pointer wird der Null-Pointer zurueckgegeben.
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsULInt_t *  lsULInt_setConsNil(lsULInt_t * il, unsigned long int i);
/* Setzt <il> auf eine ein-elementige Liste mit Element <i>
 * (bei Null-Pointer wird die Liste allociert)
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsULInt_t *  lsULInt_SetPt(int n, unsigned long int *items);
lsULInt_t *  lsULInt_setPt(lsULInt_t * il_to, int n, unsigned long int *items);
/* Belegt Pointer auf List-Items mit <items> und
 * setzt <n_list> und <max_list> auf <n>.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsULInt_t *  lsULInt_CpyPt(lsULInt_t * il_from);
lsULInt_t *  lsULInt_cpyPt(lsULInt_t * il_to, lsULInt_t * il_from);
/* Kopiert Pointer auf List-Items von <il_from> nach <il_to> und
 * setzt <n_list> und <max_list> entsprechend.
 * (Bei Null-Pointer wird <il_to> allociert.)
 * Gibt eventuell vorhanden List-Item-Speicher frei.
 * [ACHTUNG: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsULInt_t *  lsULInt_Cpy(const lsULInt_t * il_from);
lsULInt_t *  lsULInt_cpy(lsULInt_t * il_to, const lsULInt_t * il_from);
/* Kopiert <il_from> nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to>. */

lsULInt_t *  lsULInt_Cat(lsULInt_t * il_1, lsULInt_t * il_2);
lsULInt_t *  lsULInt_cat(lsULInt_t * il_to, lsULInt_t * il_from);
/* Kopiert die Liste <il_from> hinter die Liste <il_to> 
 * Pointer werden als Pointer kopiert. Rueckgabe ist <il_to> 
 * [Fehlermeldung: <il_to> Null-Pointer] */
#if defined(LS_IS_lsPt)
lsULInt_t * lsULInt_addCat(lsULInt_t *il_to, lsULInt_t *il);
/* Rueckgabe: lsULInt_add(il_to, lsULInt_Cat(il_to,il)) */
lsULInt_t * lsULInt_AddCat(lsULInt_t *il_to, lsULInt_t *il);
/* Rueckgabe: lsULInt_Add(il_to, lsULInt_Cat(il_to,il)) */
#endif

void      lsULInt_Free(lsULInt_t * il);
/* Gibt den Speicher einer Liste frei
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

void      lsULInt_free(lsULInt_t * il);
/* Gibt den Speicher der Listenelemente frei. Keine Freigabe der Liste selbst.
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

/*
 * LISTEN ERWEITERN UND REDUZIEREN --------------------------------------
 */

unsigned long int lsULInt_setIndex(lsULInt_t *il, int index, unsigned long int i, unsigned long int i0);
/* Setzt Listen-Element <index> auf <i>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist das alte Listen-Element. 
 * [Fehlermeldung: Liste Null-Pointer, index < 0] */

lsULInt_t * lsULInt_setIndices(lsULInt_t *il, lsInt_t *indices, unsigned long int x,
		       unsigned long int undef);
/* Setzt die Listen-Elemente mit Index aus <indices> auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsULInt_setIndex: index < 0] */

lsULInt_t * lsULInt_nsetIndex(lsULInt_t *il, int index, int n, unsigned long int x, 
		      unsigned long int undef);
/* Setzt <n> Listen-Elemente <index>,<index>+1,..,<index>+n-1 auf den Wert <x>.
 * Speicherbereich wird automatisch erweitern und mit <i0> initialisiert.
 * Rueckgabe ist die neue Liste
 * [Fehlermeldung von lsULInt_setIndex: index < 0] */

unsigned long int *lsULInt_getNewItem(lsULInt_t *il);
int        lsULInt_getNewItemIndex(lsULInt_t *il);
/* Fuegt ein neues nicht initialisiertes Element an die Liste <il> an.
 * Rueckgabe ist ein Pointer/Index auf dieses Element */

#define   lsULInt_pushLast(il,i) (lsULInt_add(il,i))
lsULInt_t *  lsULInt_add(lsULInt_t * il, unsigned long int i);
lsULInt_t *  lsULInt_Add(lsULInt_t * il, unsigned long int i);
/* Fuegt Element <i> hinten an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

#define   lsULInt_pushHead(il,i) (lsULInt_cons(il,i)
lsULInt_t *  lsULInt_cons(lsULInt_t * il, unsigned long int i);
lsULInt_t *  lsULInt_Cons(lsULInt_t * il, unsigned long int i);
/* Fuegt Element <i> vorne an die Liste <il> an
 * !!!!!Falls Liste Null-Pointer wird Liste neu allociert */

unsigned long int lsULInt_popLast(lsULInt_t *il, unsigned long int undef);
/* Entfernt das letzte Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

unsigned long int lsULInt_popHead(lsULInt_t *il, unsigned long int undef);
/* Entfernt das erste Element aus Liste <il>.
 * Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

lsULInt_t *  lsULInt_init(lsULInt_t * il);
lsULInt_t *  lsULInt_Init(lsULInt_t * il);
/* Entfernt das letzte Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsULInt_t *  lsULInt_tail(lsULInt_t * il);
lsULInt_t *  lsULInt_tail(lsULInt_t * il);
/* Entfernt das erste Element der Liste <il> 
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] 
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsULInt_t * lsULInt_take(lsULInt_t *il, int n);
lsULInt_t * lsULInt_Take(lsULInt_t *il, int n);
/* Nimmt die ersten <n> Elemente der Liste <il>
 * [Achtung: bei Pointer-Liste keine Freigabe der Element-Daten] */

lsULInt_t * lsULInt_drop(lsULInt_t *il, int i);
lsULInt_t * lsULInt_Drop(lsULInt_t *il, int i);
/* Kopiert die Elemente ab Element <i>+1 in eine neue Rueckgabeliste
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */ 

lsULInt_t * lsULInt_dropPt(lsULInt_t *il, int i);
/* Setzt den Pointer auf den Start der Liste um <i> Elemente weiter
 * und reduziert die Anzahl der Elemente um <i>.
 * Falls <i> groesser als die Anzahl der Elemente, wird <i> gleich
 * der Anzahl der Elemente gesetzt */

lsULInt_t * lsULInt_split(lsULInt_t *il, int i);
/* Teilt die Liste <il> an der Stelle <i> in
 * - <il> = lsULInt_take(<il>,i)
 * - Rueckgabe lsULInt_Drop(<il>,i)
 * Falls <i> kein legaler Index ist, ist die Rueckgabe NULL */

lsPt_t * lsULInt_nsplit(lsPt_t *il_split, lsULInt_t *il, lsInt_t *is);
/* Teilt die Liste <il> bei den Indices aus <is> in
 * - Rueckgabe[0] = lsULInt_take(<il>,is[0])
 * - Rueckgabe[j] = lsULInt_Drop(lsULInt_take(<il>,is[j+1]),is[j])
 * Falls <is> leere Liste ist in Rueckgabe[0] die ganze Liste enthalten */

lsULInt_t * lsULInt_range(lsULInt_t *il, int i0, int iN);
lsULInt_t * lsULInt_Range(lsULInt_t *il, int i0, int iN);
lsULInt_t * lsULInt_rangePt(lsULInt_t *il, int i0, int iN);
lsULInt_t * lsULInt_cpyRange(lsULInt_t *il_to, lsULInt_t *il, int i0, int iN);
lsULInt_t * lsULInt_catRange(lsULInt_t *il_to, lsULInt_t *il, int i0, int iN);
/* Schneidet aus der Liste <il> die Teilliste [i0..iN] heraus */

lsULInt_t * lsULInt_insSwap(lsULInt_t *il, int index, unsigned long int i, unsigned long int i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Das bisherige Element an der Stelle <index> wird an das Ende der Liste
 * gestellt (nur fuer lvalues: es sei denn, dieses Element ist gleich <i0>).  
 * Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden <i0>
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

unsigned long int lsULInt_delSwap(lsULInt_t *il, int i);
/* Entfernt das Element mit Index <i> aus Liste <il>.
 * Dabei wird das letzte Element der Liste in entstehende Luecke geschrieben.
 * Rueckgabe ist das geloeschte Element.
 * [Fehlermeldung: Liste Null-Pointer oder kein Element vorhanden] */

lsULInt_t * lsULInt_insert(lsULInt_t *il, int index, unsigned long int i, unsigned long int i0);
/* Fuegt an der Stelle <index> ein neues Element i ein.
 * Die Elemente (einschliesslich) ab il[index] werden um einen nach hinten
 * verschoben. Falls <il> Null-Pointer, wird die Liste neu allociert.
 * Falls <index> groesser als die Laenge der Liste <il> ist, werden i0
 * Elemente bis zum <index> eingefuegt.
 * [Fehlermeldung: index < 0] */

lsULInt_t * lsULInt_insertN(lsULInt_t *il, int index, int n, unsigned long int i, unsigned long int i0);
/* Wie lsULInt_insert, fuegt aber <n>-mal das Element <i> ein */

unsigned long int lsULInt_delete(lsULInt_t *il, int index, unsigned long int undef);
/* Entfernt das Element mit Index <index> aus Liste <il>.
 * Dabei ruecken alle nachfolgenden Elemente eine Position auf.
 * Rueckgabe ist das geloeschte Element oder undef falls <index> nicht 
 * existiert. */

#if defined(LS_IS_lvalue)
lsULInt_t * lsULInt_join(lsULInt_t *il_to, lsULInt_t *il_from);
/* Kopiert alle Elemente aus <il_from>, die noch nicht in <il_to> sind, hinter
 * <il_to>. Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber lsULInt_elem realisiert.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer kopiert] */  
#endif

#if defined(LS_IS_lsInt)
lsULInt_t * lsULInt_joinInts(lsULInt_t *il_to, lsULInt_t *il_from, lsInt_t *value2index);
/* wie lsULInt_join ...
 * Der Test, ob ein Element schon in <il_to> enthalten ist, wird
 * ueber den Hash-Table <value2index> realisiert, falls kein Null-Pointer
 * uebergeben wurde (sonst ueber lsULInt_elem). <value2index> wird bei der
 * Operation automatisch aktualisiert.
 * [ACHTUNG: nur Integer-Listen] */
#endif

/*
 * ELEMENTE AUS LISTEN AUSLESEN, OBSERVATOREN ---------------------------
 */

unsigned long int lsULInt_last(lsULInt_t *il, unsigned long int undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

unsigned long int lsULInt_head(lsULInt_t *il, unsigned long int undef);
/* Rueckgabe ist das geloeschte Element oder undef falls dieses nicht 
 * existiert. */

unsigned long int lsULInt_get(lsULInt_t * il,int i);
/* Gibt das <i>te Element der Liste zurueck
 * [Fehlermeldung: Index gueltiger Bereich, Liste Null-Pointer] */

unsigned long int lsULInt_getCheck(lsULInt_t * il,int i, unsigned long int undef);
/* Gibt das <i>te Element der Liste zurueck
 * Falls index ungueltiger Bereich oder Liste Null-Pointer, wird
 * <undef> zurueckgegeben */

unsigned long int lsULInt_getFlip(int i, lsULInt_t *il, unsigned long int undef);
/* entspricht LS_GET_CHECK(il,i,undef) */

lsULInt_t *  lsULInt_getRowPt(lsULInt_t * row, lsULInt_t * il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor)
 * Setzt den Speicher von <row> auf den Speicher des <i>-ten Zeilenvektors
 * der Matrix-Liste <il>.
 * Rueckgabe: Liste <row> mit Zeiger auf Zeilenvektor 
 * ACHTUNG: Der Speicher von <row> wird versucht vorher freizugeben. */

lsULInt_t *  lsULInt_getRow(lsULInt_t *row, lsULInt_t *il, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Zeilenvektor von Matrix-Liste <il> nach <row>. */

lsULInt_t *  lsULInt_getCol(lsULInt_t *col, lsULInt_t *il, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor). 
 * Kopiert den <i>-ten Spaltenvektor von Matrix-Liste <il> nach <col>. */

lsULInt_t *  lsULInt_setRow(lsULInt_t *il, lsULInt_t *row, int i, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Zeilenvektor <row> in die <i>-te Zeile der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

lsULInt_t *  lsULInt_setCol(lsULInt_t *il, lsULInt_t *col, int j, int cols);
/* Interpretiert die Liste als Matrix mit <cols> Spalten (Zeilenvektor).
 * Kopiert den Spaltenvektor <col> in die <j>-te Spalte der Matrix-Liste <il>. 
 * ACHTUNG: Elemente, die ueber das Listenende hinausgehen werden nicht kopiert
 */

int       lsULInt_length(lsULInt_t * il);
/* Gibt Laenge der Liste zurueck
 * !!!Liste Null-Pointer ergibt Laenge 0 */


#if defined(LS_IS_lvalue)
int lsULInt_elem(lsULInt_t * il, unsigned long int i);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsULInt_neqElem(lsULInt_t *il, unsigned long int i);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsULInt_disjoint(lsULInt_t *il1, lsULInt_t *il2);
/* Testet ob die zwei Listen <il1> und <il2> gemeinsame Elemente haben
 * Rueckgabe ist 1 (disjoint) 0 (haben gemeinsames Element)
 */

lsULInt_t *lsULInt_subst(lsULInt_t *il, unsigned long int i, unsigned long int j);
/* Substituiert alle Listenelemente <i> durch <j>.
 * Rueckgabe ist die geaenderte Liste. */

#ifndef LS_IS_lsPt
int lsULInt_subBag(lsULInt_t *il_sub, lsULInt_t *il_super, unsigned long int undef);
int lsULInt_subBagIndices(lsInt_t *indices,
		     lsULInt_t *il_sub, lsULInt_t *il_super, unsigned long int undef);
#if !defined(LS_IS_lsFloat) && !defined(LS_IS_lsDouble)
int lsULInt_subBagLimitedIndices(lsInt_t *indices,
			    lsULInt_t *il_sub, lsULInt_t *il_super, 
			    lsInt_t *limit, unsigned long int undef);
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

int lsULInt_getIndex(lsULInt_t * il, unsigned long int item);
int lsULInt_getFstIndex(lsULInt_t *il, unsigned long int item);
int lsULInt_getLastIndex(lsULInt_t *il, unsigned long int item);
/* Testet ob <i> Element von Liste <il> (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsULInt_getLastNeqIndex(lsULInt_t * il, unsigned long int item);
/* Testet ob es ein Element != <i> in der Liste <il> gibt
 * (Pointer-Liste: Pointer-Vergleich)
 * Rueckgabe ist <index> sonst -1 */

int lsULInt_getFstNeqIndex(lsULInt_t * il, unsigned long int item);
/* wie lsULInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */
#endif

int lsULInt_elemFunc(lsULInt_t * il, unsigned long int i, lsULInt_cmp_t *func);
/* Testet ob <i> Element von Liste <il> nach Funktion (*func)(,)==0 gilt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsULInt_neqElemFunc(lsULInt_t *il, unsigned long int i, lsULInt_cmp_t *func);
/* Testet ob es ein Element != <i> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist 1 (gefunden) sonst 0 */

int lsULInt_getIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func);
/* Testet ob <item> Element von Liste <il> nach Funktion (*func)(,)==0 ist
 * Rueckgabe ist <index> sonst -1 */

int lsULInt_getLastNeqIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func);
/* Testet ob es ein Element != <item> in der Liste <il> nach Funktion
 * (*func)(,)!=0 gibt
 * Rueckgabe ist <index> sonst -1 */

int lsULInt_getFstNeqIndexFunc(lsULInt_t * il, unsigned long int item, lsULInt_cmp_t *func);
/* wie lsULInt_getLastNeqIndex, Suche beginnt hier beim 1. Element */

unsigned long int lsULInt_foldl(lsULInt_t *il, unsigned long int item0, lsULInt_fold_t *func);
unsigned long int 
lsULInt_foldl_2(lsULInt_t *il, unsigned long int item0, lsULInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von links auf.
 * Rueckgabe: <func>(... <func>(<func>(item0,<il>[0]),<il>[1])...,<il>[n-1])
 * Falls leere Liste Rueckgabe item0
 * lsULInt_foldl_2 wendet die Function <func>(item0,<il>[0],arg)... an.
 * [FEHLER: func ist Null-Pointer] */

unsigned long int lsULInt_foldr(lsULInt_t *il, unsigned long int item0, lsULInt_fold_t *func);
unsigned long int 
lsULInt_foldr_2(lsULInt_t *il, unsigned long int item0, lsULInt_fold_2_t *func, void *arg);
/* Faltet die List <il> mit der Funktion <func> von rechts auf.
 * Rueckgabe: <func>(<il>[0],... <func>(<il>[n-2],<func>(<il>[n-1],item0))...)
 * Falls leere Liste Rueckgabe item0 
 * lsULInt_foldr_2 wendet die Function ...<func>(<il>[n-1],item0,arg) an.
 * [FEHLER: func ist Null-Pointer] */

unsigned long int lsULInt_maxFunc(lsULInt_t *il, lsULInt_cmp_t *func);
int       lsULInt_maxIndexFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* liefert das Maximum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

unsigned long int lsULInt_minFunc(lsULInt_t *il, lsULInt_cmp_t *func);
int       lsULInt_minIndexFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* liefert das Minimum der Liste bezueglich der Vergleichsfunktion 
 * <func>(<i>,<j>) < 0 mit <i> < <j>
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

int lsULInt_bsearchLtFunc(lsULInt_t *il, unsigned long int i, lsULInt_cmp_t *func);
int lsULInt_bsearchLtFunc_2(lsULInt_t *il,unsigned long int i,lsULInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsULInt_bsearchGtFunc(lsULInt_t *il, unsigned long int i, lsULInt_cmp_t *func);
int lsULInt_bsearchGtFunc_2(lsULInt_t *il,unsigned long int i,lsULInt_cmp_2_t *func,void *arg);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste bezueglich der Vergleichsfunktion
 * <func>(<i>,<j>) < 0 mit <i> < <j>.
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.
 * ..._2 Funktion wendet die Vergleichsfkt. <func>(<i>,<j>,<arg>) an. */

int lsULInt_cmpFunc(lsULInt_t *il1, lsULInt_t *il2, lsULInt_cmp_t *func);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2> */
 
int lsULInt_cmpIndex(int i, int j, pairPt_t *arg);
int lsULInt_cmpIndexPt(int i, int j, pairPt_t *arg);
/* vergleicht die zwei Elemente mit Index <i> und <j> aus der
 * Liste l=<tplFst(arg)> anhand der Funktion f=<tplSnd(arg)>.
 * Falls <f> NULL-Pointer werden die Werte aus der Liste
 * <l> direkt verglichen (nur fuer lvalues definiert)
 * Rueckgabe -1, falls l[i] <  l[j]
 *            0, falls l[i] == l[j]
 *            1, falls l[i] >  l[j] 
 * lsULInt_cmpIndex   wendet die Funktion <f>(l[i],l[j]) an,
 * lsULInt_cmpIndexPt wendet die Funktion <f>(&l[i],&l[j]) an. */
int lsULInt_cmpValue2Index(int dummy, int j, triplePt_t *arg);
int lsULInt_cmpValue2IndexPt(int dummy, int j, triplePt_t *arg);
/* wie oben aber die Werte l[j] werden mit dem dritten Element des Triples
 * verglichen:
 * lsULInt_cmpValue2Index   wendet die Funktion <f>(*tplTrd(arg),l[j]) an,
 * lsULInt_cmpValue2IndexPt wendet die Funktion <f>(tplTrd(arg),&l[j]) an. */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
unsigned long int lsULInt_sum(lsULInt_t *il);
/* liefert die Summe ueber alle Listenelemente */

unsigned long int lsULInt_prod(lsULInt_t *il);
/* liefert das Produkt ueber alle Listenelemente */

lsULInt_t * lsULInt_scale(lsULInt_t *il, unsigned long int s);
/* skaliert jedes Element der List um Faktor s */

lsULInt_t *  lsULInt_delta(lsULInt_t *il_to, lsULInt_t *il_from, unsigned long int base);
/* Berechnet die Differenzen aufeinanderfolgender Elemente.
 * Die Differenz des ersten Elementes wird zum Werte <base> berechnet.
 * Falls <il_to> Null-Pointer ist, wird diese Liste allociert. */

unsigned long int lsULInt_max(lsULInt_t *il);
int       lsULInt_maxIndex(lsULInt_t *il);
/* liefert das Maximum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

unsigned long int lsULInt_min(lsULInt_t *il);
int       lsULInt_minIndex(lsULInt_t *il);
/* liefert das Minimum der Liste 
 * [Fehlermeldung: Liste Null-Pointer, leere Liste] */

lsULInt_t *lsULInt_rmdup(lsULInt_t *il);
/* loescht alle Elemente der Liste, die auf ein identisches Element
 * in der List folgen. Falls die Liste sortiert ist, werden alle
 * doppelten Vorkommen von Elementen geloescht.
 */

int lsULInt_bsearchLt(lsULInt_t *il, unsigned long int i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine aufsteigend sortierte Liste (<i> < <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsULInt_bsearchGt(lsULInt_t *il, unsigned long int i);
/* liefert den Index eines zu <i> identischen Listen-Eintrags fuer
 * eine absteigend sortierte Liste (<i> > <j> mit <i> vor <j>)
 * Falls <i> nicht in der Liste enthalten ist, wird der erwartete
 * Einfuegeindex (negativ kodiert: -(index+1)) zurueckgeliefert.*/

int lsULInt_cmp(lsULInt_t *il1, lsULInt_t *il2);
/* vergleicht die zwei Listen <il1> und <il2> elementweise
 * Rueckgabe -1, falls <il1> <  <il2>
 *            0, falls <il1> == <il2>
 *            1, falls <il1> >  <il2>
 * [Achtung: fuer Pointer-Listen nicht definiert] */
#endif

#if defined(LS_IS_lsInt)
lsInt_t * lsULInt_makeIndex(lsInt_t *index, lsULInt_t *il);
/* Generiert einen Index fuer Liste <il>, d.h. LS_GET(<index>,<item>) = <i>
 * mit LS_GET(<il>,<i>) = <item>
 * [ACHTUNG: bei mehrfacher Zuordnung wird der letzte Eintrag referenziert] */
#endif

/*
 * LISTE TRANSFORMIEREN ----------------------------------------
 */

lsULInt_t *lsULInt_reverse(lsULInt_t *il);
/* Dreht die Elemente einer Liste um */

lsULInt_t *  lsULInt_CpyMap(lsULInt_t * il_from, lsULInt_map_t *func);
lsULInt_t *  lsULInt_cpyMap(lsULInt_t * il_to, lsULInt_t * il_from, lsULInt_map_t *func);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>) an und schreibt das
 * Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsULInt_t *  lsULInt_map(lsULInt_t * il, lsULInt_map_t *func);
/* Wendet auf jedes Element <i> von <il> <func>(<i>) an
 * Rueckgabe ist <il> */

lsULInt_t *  lsULInt_mapSet(lsULInt_t * il, lsULInt_map_t *func);
/* wie lsULInt_map, aber Ergebnis von <func>(<i>) wird dem Listeneintrag
 * wieder zugewiesen */

lsULInt_t *  lsULInt_map_2(lsULInt_t * il, lsULInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg>) an
 * Rueckgabe ist <il> */

lsULInt_t *  lsULInt_map_3(lsULInt_t * il, lsULInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il> <func>(<i>,<arg1>,<arg2>) an
 * Rueckgabe ist <il> */

lsULInt_t *  lsULInt_mapSet_2(lsULInt_t * il, lsULInt_map_2_t *func, void *arg);
/* wie lsULInt_map, aber Ergebnis von <func>(<i>,<arg>) wird dem Listeneintrag
 * wieder zugewiesen */

lsULInt_t *  lsULInt_mapSet_3(lsULInt_t * il,lsULInt_map_3_t *func,void *arg1,void *arg2);
/* wie lsULInt_map, aber Ergebnis von <func>(<i>,<arg1>,<arg2>) wird dem 
 * Listeneintrag wieder zugewiesen */

lsULInt_t *  lsULInt_CpyMap_2(lsULInt_t * il_from, lsULInt_map_2_t *func, void *arg);
lsULInt_t *  lsULInt_cpyMap_2(lsULInt_t * il_to, lsULInt_t * il_from, 
		      lsULInt_map_2_t *func, void *arg);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsULInt_t *  lsULInt_CpyMap_3(lsULInt_t * il_from, lsULInt_map_3_t *func, 
		      void *arg1, void *arg2);
lsULInt_t *  lsULInt_cpyMap_3(lsULInt_t * il_to, lsULInt_t * il_from, 
		      lsULInt_map_3_t *func, void *arg1, void *arg2);
/* Wendet auf jedes Element <i> von <il_from> <func>(<i>,<arg1>,<arg2>) an und
 * schreibt das Ergebnis nach <il_to> (bei Null-Pointer wird <il_to> allociert)
 * Rueckgabe ist <il_to> */

lsULInt_t * lsULInt_CartProd(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func);
lsULInt_t * lsULInt_cartProd(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func);
lsULInt_t * lsULInt_cpyCartProd(lsULInt_t *il_to, 
			lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_t *func);
lsULInt_t * lsULInt_cartProd_2(lsULInt_t *il1, lsULInt_t *il2, lsULInt_fold_2_t *func,
		       void *arg);
/* Wendet auf alle Kombinationen von <il2>,<il3> Elementen die Funtion <func>
 * an. Falls <func> Null-Pointer wird (*) gerechnet.
 * Reihenfolge in <il_to> ist [func(il1[0],il2[0]), func(il1[0],il2[1]), ...]
 * Falls eine Liste Nil ist, wird die andere Liste kopiert.
 * lsULInt_CartProd gibt eine neu allocierte List zurueck
 * lsULInt_cartProd gibt il1 als Ergebnisliste zurueck
 * lsULInt_cpyCartProd gibt il_to als Egebnislist zurueck
 * lsULInt_cartProd_2 wendet func(il1[0],il2[0],arg)... an
 * [Fehler: bei Pointer-Listen muss eine Funktion uebergeben werden] */

lsULInt_t * lsULInt_sortByIndex(lsULInt_t *il, lsInt_t *index, unsigned long int undef);
/* Sortiert die Liste <il> nach der Index-Reihenfolge in <index>
 * Indices die nicht in <index> vorkommen werden geloescht,
 * solche die nicht in <il> vorkommen <undef> gesetzt, soweit danach
 * noch ein definiertes Element folgt (bei nicht-lvalues immer).
 * Ein Null-Pointer <index> veraendert die Liste nicht 
 * Ein Null-Pointer <il> wird als Null-Pointer zurueckgegeben */

lsULInt_t * lsULInt_cpySortByIndex(lsULInt_t *il_to, lsULInt_t *il_from, 
			   lsInt_t *index, unsigned long int undef);
/* wie lsULInt_sortByIndex, aber die geaenderte Liste wird nach <il_to> geschrieben.
 * Falls <il_to> Null-Pointer wird die Ausgabeliste neu allociert */ 

lsULInt_t * lsULInt_qsortLtFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_qsortGtFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_qsortLtFunc_2(lsULInt_t *il, lsULInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) < 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_qsortGtFunc_2(lsULInt_t *il, lsULInt_cmp_2_t *func, void *arg);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>,arg) > 0
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsULInt_qsortIndexLtFunc(lsInt_t *index, lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) < 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsULInt_qsortIndexGtFunc(lsInt_t *index, lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * <func>(LS_GET(<il>,<i>),LS_GET(<il>,<j>)) > 0 mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_msortLtFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) < 0
 * mit <i> vor <j> (Mergesort) */

lsULInt_t * lsULInt_msortGtFunc(lsULInt_t *il, lsULInt_cmp_t *func);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <func>(<i>,<j>) > 0
 * mit <i> vor <j> (Mergesort) */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsULInt_t * lsULInt_qsortLt(lsULInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_qsortGt(lsULInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Quicksort)
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsULInt_qsortIndexLt(lsInt_t *index, lsULInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) < LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsInt_t * lsULInt_qsortIndexGt(lsInt_t *index, lsULInt_t *il);
/* Sortiert den <index> von Liste <il> nach der Vergleichsfunktion 
 * LS_GET(<il>,<i>) > LS_GET(<il>,<j>) mit <i> vor <j> (Quicksort)
 * Falls <index> Null-Pointer wird <index> allociert.
 * [ACHTUNG: Reihenfolge gleicher Elemente kann sich aendern] */

lsULInt_t * lsULInt_msortLt(lsULInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> < <j> 
 * mit <i> vor <j> (Mergesort) */

lsULInt_t * lsULInt_msortGt(lsULInt_t *il);
/* Sortiert die Liste <il> nach der Vergleichsfunktion <i> > <j> 
 * mit <i> vor <j> (Mergesort) */
#endif

/* 
 * LISTE FILTERN ---------------------------------------------------
 */

#if defined(LS_IS_lvalue)
lsULInt_t * lsULInt_filterByValue(lsULInt_t *il, unsigned long int undef, lsInt_t *indices);
/* Filtert alle undef-Elemente aus der Liste heraus, d.h. es existieren
 * danach nur noch Werte != undef in der Liste. Die Reihenfolge der Elemente
 * wird dabei nicht veraendert. Falls indices != NULL ist, stehen in
 * <indices> nachher alle frueheren Indices der erhaltenen Elemente. */ 

lsULInt_t * lsULInt_CpyFilterByValue(lsULInt_t *il, unsigned long int undef, lsInt_t *indices);
/* wie lsULInt_filterByValue ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsULInt_t * lsULInt_cpyFilterByValue(lsULInt_t *il_to, lsULInt_t *il_from, unsigned long int undef, 
			     lsInt_t *indices);
/* wie lsULInt_CpyFilterByValue ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */
#endif

lsULInt_t * lsULInt_filterByIndex(lsULInt_t *il, lsInt_t *indices);
/* Filtert alle Elemente, deren Listen-Index nicht in <indices> enthalten ist
 * aus der Liste heraus. Die <indices> muessen sortiert sein. Doppelt
 * vorkommende Indices sind nicht erlaubt. */

lsULInt_t * lsULInt_CpyFilterByIndex(lsULInt_t *il, lsInt_t *indices);
/* wie lsULInt_filterByIndex ...
 * Die Elemente werden allerdings nicht in der alten sonderen in einer
 * neu allocierten Liste gesammelt. Die <indices> muessen nicht mehr
 * sortiert sein und duerfen auch doppelt vorkommen.
 * Rueckgabe ist die neue Liste oder NULL, falls il==NULL od. indices==NULL.
 * [ACHTUNG: bei Pointer-Listen wird nur der Pointer in die neue Liste
 *  uebertragen] */

lsULInt_t * lsULInt_cpyFilterByIndex(lsULInt_t *il_to, lsULInt_t *il_from, lsInt_t *indices);
/* wie lsULInt_CpyFilterByIndex ...
 * Die Liste, in der die Elemente gesammelt werden, kann mit <il_to>
 * uebergeben werden. */

lsULInt_t *  lsULInt_filter(lsULInt_t *il, lsULInt_filter_t *func);
/* Loescht alle Element <i> mit !<func>(<i>) aus der Liste <li>
 * [Achtung: bei Pointer-Liste keine Freigabe der Listen-Elemente] */

lsULInt_t *  lsULInt_CpyFilter(lsULInt_t *il_from, lsULInt_filter_t *func);
lsULInt_t *  lsULInt_cpyFilter(lsULInt_t *il_to, lsULInt_t *il_from, lsULInt_filter_t *func);
/* Uebertraegt alle Element <i> aus <il_from> mit <func>(<i>)==TRUE
 * in die Liste <li_to> (bei Null-Pointer wird <il_to> allociert) */

/*
 * EIN-/AUSGAGE FUNKTIONEN -------------------------------------------
 */

#if defined(LS_IS_lvalue) && !defined(LS_IS_pt)
lsULInt_t * lsULInt_sscan_chr(lsULInt_t *il, char t, char *s);
/* Uebertraegt String <s> in Liste <il>. Trennzeichen <t> muss Elemente in
 * <s> separieren. Leere Teilstrings werden uebersprungen.
 * (bei Null-Pointer wird <il> allociert)
 * Rueckgabe ist <il>
 * [Achtung: fuer Pointer-Listen nicht definiert] */

char * lsULInt_sprint_chr(char *s, lsULInt_t *il, char t);
char * lsULInt_sprintf_chr(char *s, lsULInt_t *il, const char *format, char t);
/* Uebertraegt Liste <il> in String <s>. Trennzeichen <t> separiert die
 * Elemente. <s> wird gegebenenfalls reallociert.
 * Rueckgabe ist <s> */

int lsULInt_fwrite(FILE *fp, lsULInt_t *il);
int lsULInt_fread(lsULInt_t *il, int k, FILE *fp);
/* schreibt/liest Liste als binary-Format. 
 * <k> ist die Anzahl der zu Lesenden list-items (bis EOF falls k==0).
 * Rueckgabe ist die Anzahl der geschriebenen/gelesenen list-items. 
 * [ACHTUNG: il muss initialisiert sein]
 */
#endif

#undef LS_IS_lvalue

#endif




