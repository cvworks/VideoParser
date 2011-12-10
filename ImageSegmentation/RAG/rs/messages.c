/**
* Datei:	messages.c
* Autor:	Gernot A. Fink
* Datum:	30.7.1997
*
* Beschreibung:	Fehler- und andere Meldungen
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "messages.h"

void rs_msg(char *format, ...)
	{
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "%s: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);

	va_end(ap);
	}

void rs_warning(char *format, ...)
	{
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "%s: WARNING: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);

	va_end(ap);
	}

void rs_error(char *format, ...)
	{
	va_list ap;

	va_start(ap, format);

	fprintf(stderr, "%s: ", program);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);

	va_end(ap);

	exit(1);
	}
