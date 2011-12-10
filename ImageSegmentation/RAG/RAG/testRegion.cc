#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pnmadd/pnmadd.h>
#include <pnmadd/pseudo.h>

#include <LEDA/graph/graph.h>
#include <LEDA/graphics/graphwin.h> 

#include "RAGraph.h"

int main(int argc, char *argv[])
{   
   RAGraph r;
  
  cin >> r;
  r.initAfterRead();
  cout << r;
  
  exit(0);
}
