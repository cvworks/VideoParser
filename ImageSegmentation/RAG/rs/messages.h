/**
* Datei:	messages.h
* Autor:	Gernot A. Fink
* Datum:	30.7.1997
*
* Beschreibung:	Definitionen fuer Fehler- und andere Meldungen
**/

#ifndef __RS_MESSAGES_H_INCLUDED__
#define __RS_MESSAGES_H_INCLUDED__

extern char *program;		/* Programmname */

void rs_msg(char *format, ...);
void rs_warning(char *format, ...);
void rs_error(char *format, ...);

#endif /* __RS_MESSAGES_H_INCLUDED__ */
