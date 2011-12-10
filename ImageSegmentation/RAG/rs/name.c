/**
* Datei:	name.c
* Autor:	Gernot A. Fink
* Datum:	5.8.1997
*
* Beschreibung:	Namensverwaltung
**/

#include <string.h>

#include "messages.h"
#include "memory.h"
#include "name.h"

char *rs_name_create(char *s)
	{
	int len;
	char *name;

	if (!s)
		return(NULL);

	len = strlen(s);
	name = rs_malloc(len + 1, "name");
	strcpy(name, s);

	return(name);
	}

char *rs_name_prepare(int len)
	{
	char *name;

	name = rs_malloc(len + 1, "name");

	return(name);
	}

void rs_name_destroy(char *name)
	{
	rs_free(name);
	}
