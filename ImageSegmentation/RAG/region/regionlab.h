/**
* Datei:	regionlab.h
* Autor:	Gernot A. Fink
* Speed-Up      Franz Kummert
* Datum:	11.1.1996
*
* Zweck:	Regionenlabelling eines farbklassifizierten Eingabebildes
**/
#ifndef _REGIONLAB_H
#define _REGIONLAB_H

#ifdef __cplusplus
extern "C" {
#endif

int regionlab(int _xsize, int _ysize, unsigned char *color, int nbLabel, 
	      int *region);
/*  _xsize: Bildgroesse x
 *  _ysize: Bildgroesse y
 *   color: farbklassifiziertes Bild (als Zeilenvektor [_xsize * _ysize])
 * nbLabel: Anzahl Farben im Bild
 *  region: regionengelabeltes Bild (als Zeilenvektor [_xsize * _ysize])
 */
#ifdef __cplusplus
}
#endif

#endif
