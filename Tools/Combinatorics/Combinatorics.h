/* 24.12.2008 last modification: 02.02.2009
Copyright (c) 2009 by Siegfried Koepf

This file is distributed under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.
For information on usage and redistribution and for a disclaimer of all
warranties, see the file COPYING in this distribution.

combinatorial generation functions public interfaces
*/

#pragma once

//return values of generation functions
#define GEN_NEXT  0 //ok, print and continue
#define GEN_TERM  1 //ok, terminate
#define GEN_EMPTY 2 //ok, print EMPTY SET and continue
#define GEN_ERROR 3 //an error occured, print an error message and terminate

//combinatorial generation functions
int gen_comb_norep_init(unsigned char *vector, const unsigned char n, const unsigned char k);
int gen_comb_norep_next(unsigned char *vector, const unsigned char n, const unsigned char k);

int gen_comb_rep_init(unsigned char *vector, const unsigned char n, const unsigned char k);
int gen_comb_rep_next(unsigned char *vector, const unsigned char n, const unsigned char k);

int gen_neck_init(unsigned char *vector, const unsigned char m, const unsigned char n);
int gen_neck_next(unsigned char *vector, const unsigned char m, const unsigned char n);

int gen_k_part_init(unsigned char *vector, const unsigned char n, const unsigned char k);
int gen_k_part_next(unsigned char *vector, const unsigned char k);

int gen_part_init(unsigned char *vector, const unsigned char n, unsigned char *k);
int gen_part_next(unsigned char *vector, unsigned char *k);

int gen_perm_rep_init(const unsigned char n);
int gen_perm_rep_next(unsigned char *vector, const unsigned char n);

int gen_vari_rep_init(unsigned char *vector, const unsigned char m, const unsigned char n);
int gen_vari_rep_next(unsigned char *vector, const unsigned char m, const unsigned char n);

