/**
* Datei:	io.c
* Autor:	Gernot A. Fink
* Datum:	29.7.1997
*
* Beschreibung:	Ein-/Ausgabefunktionen 
**/

#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "messages.h"

/** #include "basics.h" **/
#include "io.h"

static _line_len = RS_LINE_LEN_DEFAULT;

char *rs_line_read(FILE *fp, char comment_char)
	{
	static char *line = NULL;

	char *cp;
	int len;

	/* evtl. Zeilenpuffer erzeugen ... */
	if (line == NULL)
		line = rs_malloc(_line_len * sizeof(char), "line buffer");

	/* ... und solange noch Zeilen gelesen werden koennen ... */
	while (fgets(line, _line_len, fp) != NULL) {
		/* ... ggf. Kommentare ueberspringen ... */
		if (comment_char && line[0] == comment_char)
			continue;

		/* ... ggf. '\n' am Zeilenende loeschen ... */
		len = strlen(line);
		if (line[len - 1] == '\n')
			line[len - 1] = '\0';
		else	rs_warning("line longer than %d characters read!",
				_line_len);

		/* ... ggf. Kommentare am Zeilenende loeschen ... */
		if (comment_char) {
			cp = strrchr(line, comment_char);
			if (cp)
				*cp = '\0';
			}

		/* ... und Zeilen mit Inhalt zurueckgeben */
		for (cp = line; *cp; cp++)
			if (!isspace(*cp))
				return(line);
		}

	/* ... sonst Dateiende signalisieren */
	return(NULL);
	}

int rs_line_is_empty(char *line)
	{
	while (*line)
		if (!isspace(*line++))
			return(0);
	return(1);
	}

char *rs_line_skipwhite(char *line)
	{
	while (isspace(*line))
		line++;
	return(line);
	}

size_t rs_line_setlength(size_t len)
	{
	size_t old_len = _line_len;

	_line_len = len;

	return(old_len);
	}

size_t rs_line_getlength(void)
	{
	return(_line_len);
	}

int rs_fskipwhite(FILE *fp, char comment_char)
	{
	int c;

	while ((c = fgetc(fp)) != EOF) {
		if (c == comment_char) {
			while ((c = fgetc(fp)) != EOF)
				if (c == '\n')
					break;
			}
		else if (!isspace(c)) {
			ungetc(c, fp);
			break;
			}
		}

	return(c);
	}
