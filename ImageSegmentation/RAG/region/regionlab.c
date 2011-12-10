/**
* Datei:	regionlab.c
* Autor:	Gernot A. Fink
* Speed-Up      Franz Kummert
* Datum:	11.1.1996
*
* Zweck:	Regionenlabelling eines farbklassifizierten Eingabebildes
**/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_NEIGHBORS	4
#define TOP_NEIGHBOR	2

static void alias(int r_new, int r_old);
static int normalize(int r);

static int *r_alias = NULL;

int regionlab(int _xsize, int _ysize, unsigned char *color, int nbLabel, 
	      int *region)
{
  static int xsize = -1;
  static int ysize = -1;

  int nregions;		/* Ausgabe, # gelabelter Regionen */
  int rcount = 0;
  int found;
  int i, k;
  int *regionpntr, *r;
  unsigned char *pixelpntr;

  /* Beim folgenden Aufrufen, wenn die Groesse abweicht von der
     vorher behandelten */
  if ((xsize>-1) && (_xsize != xsize || _ysize != ysize)) 
  {
    if (r_alias)
      free (r_alias);
    r_alias = NULL;
  }

  if (r_alias == NULL) 
  {
    /* Einzel-Farbbild erzeugen */
    xsize = _xsize;
    ysize = _ysize;

    /* Regionen-Alias-Liste erzeugen ... */
    r_alias = malloc(xsize * ysize * sizeof(int));
    if (r_alias == NULL) 
    {
      fprintf(stderr,
	      "cannot allocate %d Bytes for `r_alias'\n",
	      xsize * ysize * sizeof(int));
      exit(1);
    }

    /* ... und initialisieren */
    for (i = xsize * ysize, r = r_alias; i > 0; i--)
      *r++ = -1;
  }


#ifdef TRACE
  for (i=0; i<ysize; ++i) 
    for (k=0; k<xsize; ++k)
      if (color[i*xsize+k] >=nbLabel ||color[i*xsize+k] < 0)
	printf("Pixel %d/%d traegt label %d max %d\n",
	       k, i, color[i*xsize+k], nbLabel);
#endif
  /* Label-Bild am Rand besetzen, damit nachfolgende Module
     keine spezielle Randbehandlung durchf"uhren muessen */
  for (i=0, pixelpntr=color; i<xsize; ++i, pixelpntr++) 
    *pixelpntr = nbLabel;
  for (i=1; i<ysize; ++i) 
  {
    color[i*xsize] = nbLabel;
    color[i*xsize-1] = nbLabel;
  }
  for (i=0, pixelpntr=(color+(ysize-1)*xsize); i<xsize; ++i,++pixelpntr)
    *pixelpntr = nbLabel;


  /* uebers Bild laufen und jeweils linken Nachbarn und die drei
     oberhalb liegenden Pixel betrachten */
  /* Sonderbehandlung fuer erste Bildzeile */
  for (k=1, region[0] = rcount++; k<xsize; ++k) 
  {
    if (color[k] == color[k-1])
      region[k] = region[k-1];
    else region[k] = rcount++;
  }
  for (i=1; i<(ysize); ++i) 
  {
    /* Sonderbehandlung fuer erstes Pixel der Zeile */
    pixelpntr = color+i*xsize;
    regionpntr = region+i*xsize;
    found = 0;
    if (*pixelpntr == *(pixelpntr-xsize)) 
    {
      *regionpntr = *(regionpntr-xsize);
      found = 1;
    }
    if (*pixelpntr == *(pixelpntr-xsize+1)) {
      if (found)
	alias(*regionpntr, *(regionpntr-xsize+1));
      else 
      {
	*regionpntr = *(regionpntr-xsize+1);
	found = 1;
      }
    }
    if (!found) *regionpntr = rcount++;

    /* Rest der Zeile */
    for (k=1, pixelpntr=(color+i*xsize+k),
	   regionpntr=(region+i*xsize+k); k<(xsize-1);
	 ++k, pixelpntr++, regionpntr++) 
    {
      found = 0;
      if (*pixelpntr == *(pixelpntr-1)) 
      {
	*regionpntr = *(regionpntr-1);
	found = 1;
      }
      if (*pixelpntr == *(pixelpntr-xsize-1))
      {
	if (found)
	  alias(*regionpntr,
		*(regionpntr-xsize-1));
	else 
	{
	  *regionpntr = *(regionpntr-xsize-1);
	  found = 1;
	}
      }
      if (*pixelpntr == *(pixelpntr-xsize))
      {
	if (found)
	  alias(*regionpntr,
		*(regionpntr-xsize));
	else 
	{
	  *regionpntr = *(regionpntr-xsize);
	  found = 1;
	}
      }
      if (*pixelpntr == *(pixelpntr-xsize+1))
      {
	if (found)
	  alias(*regionpntr,
		*(regionpntr-xsize+1));
	else 
	{
	  *regionpntr = *(regionpntr-xsize+1);
	  found = 1;
	}
      }
      if (!found) *regionpntr = rcount++;
    }

    /* Sonderbehandlung fuer letztes Pixel der Zeile */

    pixelpntr = color+i*xsize+(xsize-1);
    regionpntr = region+i*xsize+(xsize-1);
    found = 0;
    if (*pixelpntr == *(pixelpntr-1)) 
    {
      *regionpntr = *(regionpntr-1);
      found = 1;
    }
    if (*pixelpntr == *(pixelpntr-xsize-1)) 
    {
      if (found)
	alias(*regionpntr, *(regionpntr-xsize-1));
      else 
      {
	*regionpntr = *(regionpntr-xsize-1);
	found = 1;
      }
    }
    if (*pixelpntr == *(pixelpntr-xsize)) 
    {
      if (found)
	alias(*regionpntr, *(regionpntr-xsize));
      else 
      {
	*regionpntr = *(regionpntr-xsize);
	found = 1;
      }
    }
    if (!found) *regionpntr = rcount++;
  }

  /* Alle eingetragenen Regionen-Aliase normalisieren ... */
  for (i = 0, r = r_alias; i < rcount; i++, r++)
    *r = normalize(i);
  /* ... und fortlaufende Nummern vergeben */
  nregions = 0;
  for (i = 0, r = r_alias; i < rcount; i++, r++)
    if (*r < i)
      *r = r_alias[*r];
    else	*r = nregions++;

  /* Regionen-Aliase aufloesen ... */
  for (i = xsize * ysize, r = region; i > 0; i--, r++)
    *r = r_alias[*r];

  /* ... und Alias-Liste fuer naechsten Durchlauf initialisieren */
  for (i = rcount, r = r_alias; i > 0; i--)
    *r++ = -1;

  /* Label-Bild am Rand mit -1 besetzen, damit nachfolgende Module
     keine spezielle Randbehandlung durchf"uhren muessen.
     Damit geht eine Region den Bach runter, naemlich der aeussere
     Rand, der ja oben gerade ein eigenes Regionenlabel bekommen hat.
     Damit steht auch nregions um 1 zu hoch. Da aber das Regionenlabel
     dieser Region 0 ist, kann nicht einfach nregions um 1 
     runtergezaehlt werden, denn dann passen nregions und
     die auftretenden Regionenlabel nicht mehr zusammen, was in 
     nachfolgenden Modulen ausgenutzt wird. Nachfolgende Module
     muessen also mit einem nicht auftretenden Regionenlabel
     klarkommen. Das sollte unproblematisch sein...
  */
  for (i=0, r=region; i<xsize; ++i, ++r) *r = (-1);
  for (i=1; i<ysize; ++i) {
    region[i*xsize] = (-1);
    region[i*xsize-1] = (-1);
  }
  for (i=0, r=(region+(ysize-1)*xsize); i<xsize; ++i, ++r) *r = (-1);

  return(nregions);
}

static void alias(int r_new, int r_old)
	{
	static int last_new = -1, last_old = -1;
	int r_al;

	if (r_new == r_old) return;

	if (r_new < r_old) {
		r_al = r_old; r_old = r_new; r_new = r_al;
		}

	if (last_new == r_new && last_old == r_old)
		return;
	else	{
		last_new = r_new;
		last_old = r_old;
		}
		
	do	{
		r_al = r_alias[r_new];
		if (r_old == r_al)
			return;
		if (r_old > r_al) {
			r_alias[r_new] = r_old;
			r_new = r_old;
			r_old = r_al;
			}
		else	r_new = r_al;
		}
	while (r_old >= 0);
	}

static int normalize(int r)
	{
	while (r_alias[r] >= 0 && r_alias[r] < r)
		r = r_alias[r];

	return(r);
	}
