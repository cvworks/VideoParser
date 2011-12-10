/**
* Datei:	io.h
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Definitionen fuer Ein-/Ausgabefunktionen 
**/

#ifndef __RS_IO_H_INCLUDED__
#define __RS_IO_H_INCLUDED__

#include <stdio.h>

#define RS_LINE_LEN_DEFAULT	8192

char *rs_line_read(FILE *fp, char comment_char);
int rs_line_is_empty(char *line);
char *rs_line_skipwhite(char *line);

size_t rs_line_setlength(size_t len);
size_t rs_line_getlength(void);

int rs_fskipwhite(FILE *fp, char comment_char);

#endif /* __RS_IO_H_INCLUDED__ */
