/*
 * string+.h
 *
 * zu <string.h> ergaenzende String-Funktionen
 *
 * Sven Wachsmuth, 10.2.2000
 *
 */


#ifndef LS_STRING_PLUS_H
#define LS_STRING_PLUS_H

/*
 * strCpy
 *
 * Allociert einen String <ss> der Laenge von <s> und kopiert <s> nach <ss>
 * Rueckgabe: allocierter String <ss>
 */
char * strCpy(char *s);

/*
 * strCat
 *
 * Allociert einen String <ss> der Laenge von <s>+<r> und kopiert <s> gefolgt
 *   von <r> nach <ss>
 * Rueckgabe: allocierter String <ss>
 */
char * strCat(char *s, char *r);

/* 
 * strchrsplit
 *
 * Trennt den String <*s> beim Trennzeichen <t> in zwei neue Strings (<p>,<q>)
 * Rueckgabe: <p> , *s = <q> (*s = NULL falls kein Trennzeichen)
 *            NULL, falls <*s> Null-Pointer oder String-Ende
 */
char * strchrsplit(char **s, char t);

/*
 * strWrap
 *
 * Umschliesst den string <s> durch "<s>" falls er nicht der regexpr
 * [a-zA-Z][a-zA-Z_\-\"0-9]* entspricht. 
 * Maskiert in diesem Fall '"' durch '\"'.
 * Rueckgabe ist immer eine Kopie des Strings.
 */ 
char * strWrap(char *s);

#endif
