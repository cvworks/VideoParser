#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pseudo.h"

int main(int argc, char *argv[])
{
  int index, n = atoi(argv[1]);
  int r,g,b;
  for (index = 0; index < n; index++) {
    pnm_getPseudoColor(&r,&g,&b,index);
    printf("%d=%03d,%03d,%03d\n",index,r,b,g);
  }
  exit(0);
}
