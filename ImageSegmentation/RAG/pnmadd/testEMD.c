#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "png.h"
#include "pnmemd.h"

int main(int argc, char *argv[])
{
  pgm_t pgm1;
  pgm_t pgm2;
  FILE *fp;
  int c;
  int n_feature = 50;

  pnm_pgm_init(&pgm1);
  pnm_pgm_init(&pgm2);

  pnm_png_verbose = 0;

  while ((c= getopt(argc, argv, "s:")) != EOF) {
      switch (c) {
	case 's':
		n_feature = atoi(optarg);
	default:
      }
  }
  if (argc - optind < 2) {
      fprintf(stderr, "USAGE: %s [-s<n_feature>] [png|ppm] [png|pgm]\n",argv[0]);
      exit(1);
  }
  fp = fopen(argv[optind++],"r");
  if (!fp) {
    fprintf(stderr, "%s: cannot open png-file '%s'.\n",argv[0],argv[optind-1]);
    exit(1);
  }
  pnm_pgm_readImage(&pgm1,fp,argv[0]);

  fp = fopen(argv[optind++],"r");
  if (!fp) {
    fprintf(stderr, "%s: cannot open png-file '%s'.\n",argv[0],argv[optind-1]);
    exit(1);
  }
  pnm_pgm_readImage(&pgm2,fp,argv[0]);
  {
      float emd = pnm_pgm_emd(&pgm1,&pgm2, n_feature, 255);
  
      printf ("EMD[%d] = %g. AFF = %g\n",n_feature,emd, 1.0 / (1.0 + emd));
  }
  exit(0);
}
