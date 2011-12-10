/* 24.12.2008 last modification: 02.02.2009
Copyright (c) 2009 by Siegfried Koepf

This file is distributed under the terms of the GNU General Public License
version 3 as published by the Free Software Foundation.
For information on usage and redistribution and for a disclaimer of all
warranties, see the file COPYING in this distribution.

testing
gen_perm_rep_init() and gen_perm_rep_next()

compile
gcc -o perm_rep_example perm_rep_example.c perm_rep.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "_generate.h"

int main(void)
{
unsigned char vector[]    = {1,2,2,3}; //where the current figure is stored
unsigned char n;                       //length of figures
int           gen_result  = 0;         //return value of generation function
unsigned int  set_counter;             //counting generated sequences
int           x;                       //iterator

n = (unsigned char)(sizeof(vector) / sizeof(unsigned char));


set_counter = 0;
printf("perm_rep(%u)\n", n);

//initialize
gen_result = gen_perm_rep_init(n);

if(gen_result == GEN_EMPTY)
 {
 set_counter++;
 printf("{} (%u)\n", set_counter);
 }

//generate all successors
while(gen_result == GEN_NEXT)
 {
 set_counter++;

 for(x = 0; x < n; x++)
  printf("%u ", vector[x]);

 printf("(%u)\n", set_counter);

 gen_result = gen_perm_rep_next(vector, n);
 }

return(EXIT_SUCCESS);
}
