/*
 * string+.c
 *
 * zu <string.h> ergaenzende String-Funktionen
 *
 * Sven Wachsmuth, 10.2.2000
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string+.h"

char * strCpy(char *s)
{
  char *ss = NULL;
  if (s) {
    ss = (char *) malloc((strlen(s)+1) * sizeof(char));
    strcpy(ss,s);
  }
  return (ss);
}

char * strCat(char *s, char *r)
{
  char *ss = NULL;
  if (s || r) {
    int n = ((s) ? strlen(s) : 0) + ((r) ? strlen(r) : 0) + 1;
    ss = (char *) malloc(n * sizeof(char));
    if (s) strcpy(ss,s); else *ss = '\0';
    if (r) strcat(ss,r);
  }
  return (ss);
}

char * strchrsplit(char **s, char t)
{
  char *p = *s;
  char *q;

  /** Falls String Null-Pointer oder String-Ende ... 
      ... gebe Null zurueck */
  if (!p || !*p) return NULL;

  /** Falls Trennzeichen Null-Character ...
      ... gebe den ganzen String zurueck und setze *s auf NULL */
  if (t == '\0') {
    *s = NULL;
    return (p);
  }
  /** Sonst trenne den String p=*s beim ersten Trennzeichen in ...
      ... (q=Rueckgabewert,<Trennzeichen>,*s) */
  q = strchr(p,t);
  if (q) {
    if (*q) /* Noch kein String-Ende */
      *s = q+1; 
    else    /* String-Ende */
      *s = NULL;

    *q = '\0'; /* String-Trennung */
  } else    /* String-Ende */
    *s = NULL;

  return (p);
}
  
char * strWrap(char *s)
{
  int wrap = 0;
  int n;
  char *q,*p = s;
  char *r;
  
  if (!s || !*s) {
    r = (char *) malloc(3*sizeof(char));
    return strcpy(r,"\"\"");
  }
  n = 0;
  wrap = !isalpha(*(p++));
  if (!wrap)
    while (*p && !(wrap = !(isalnum(*p) || *p=='_' || *p=='\"'))) {
      if (*p == '\"') n++; 
      p++;
    }
  if (wrap) 
    n += strlen(s)+2;
  else 
    n = strlen(s);

  r = (char *) malloc((n+1)*sizeof(char));
  if (wrap) {
    strcpy(r,"\"");
    q = s;
    while (p = strchrsplit(&q,'\"')) {
      strcat(r,p);
      if (q) strcat(r,"\\\"");
    }
    if (wrap)
      strcat(r,"\"");
  } else 
    strcpy(r,s);

  return (r);
}
  
