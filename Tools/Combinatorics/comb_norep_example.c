/* 24.12.2008 last modification: 02.02.2009
Copyright (c) 2009 by Siegfried Koepf

This file is distributed under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.
For information on usage and redistribution and for a disclaimer of all
warranties, see the file COPYING in this distribution.

testing
gen_comb_norep_init() and gen_comb_norep_next()

compile
gcc -o comb_norep_example comb_norep_example.c comb_norep.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "_generate.h"

int main(void)
{
unsigned char n           = 5;    //length of alphabet
unsigned char k           = 3;    //length of figures
unsigned char *vector     = NULL; //where the current figure is stored
int           gen_result;         //return value of generation functions
unsigned int  set_counter;        //counting generated subsets
int           x;                  //iterator


//alloc memory for vector
vector = (unsigned char *)malloc(sizeof(unsigned char) * k);
if(vector == NULL)
 {
 fprintf(stderr, "error: insufficient memory\n");
 exit(EXIT_FAILURE);
 }


set_counter = 0;
printf("comb_norep(%u, %u)\n", n, k);

//initialize
gen_result = gen_comb_norep_init(vector, n, k);

if(gen_result == GEN_ERROR)
 {
 fprintf(stderr, "error: couldn't initialize");
 return(EXIT_FAILURE);
 }

if(gen_result == GEN_EMPTY)
 {
 set_counter++;
 printf("{} (%u)\n", set_counter);
 }

//generate all successors
while(gen_result == GEN_NEXT)
 {
 set_counter++;

 for(x = 0; x < k; x++)
  printf("%u ", vector[x]);

 printf("(%u)\n", set_counter);

 gen_result = gen_comb_norep_next(vector, n, k);
 }

return(EXIT_SUCCESS);
}
