#include <stdlib.h>
#include <stdio.h>
#include "xpm.h"
#include "pnmadd.h"

int main(int argc, char *argv[])
{
  ppm_t ppm;
  pnm_xpm_t xpm;
  pnm_xpm_init(&xpm);
  FILE *fp;
  
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s <ppm> > <xpm>\n",argv[0]);
    exit(1);
  }
  if (*argv[1] == '-') {
    fp = stdin;
  } else {
    fp = fopen(argv[1],"r");
  }
  ppm.pixels = ppm_readppm(fp, &ppm.cols, &ppm.rows, &ppm.maxval);
  { 
    ppm_t * ppm2 =  pnm_ppm_scale(NULL,&ppm,3);
    //pnm_ppm2xpm(&xpm, ppm2);
    pnm_xpm_fbox(&xpm, 400, 400, ppm.pixels[0][0]);
    pnm_xpm_insertPPM(&xpm, ppm2, 0, 0);
    pnm_xpm_fprint(stdout, &xpm);
  }
  exit(0);
}




