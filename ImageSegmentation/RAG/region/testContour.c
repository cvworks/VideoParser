#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "regioninfo.h"
#include <pnmadd/pnmadd.h>
#include <pnmadd/pseudo.h>

#define USAGE "\
Usage: %s -[hco:imV] <ppm_input>\n\
"

#define HELP_MSG "\
\t<ppm_input>\tppm colorMap or '-' for <stdin> (\"\" for no ppm_input)\n\
\t-c\t\twrite file set of region contours (default: seg_%%d.pgm).\n\
\t-o <format>\tset name of region contour files to <format>.\n\
\t-i <label>\tignore regions with color/region label <label>.\n\
\t-m <int>\tignore regions with more than <int> marginal pixels.\n\
\t-V\t\tprint version\n\
\t-h\t\tprint this message.\n\
"
char *program;
char *version = "version 0.3";

int main(int argc, char *argv[])
{
  int c;
  int segment_ignore = -1;
  int segment_ignore_threshold = -1;

  char *contour_filename = NULL;
  int   write_contour_files = 0;
  char *regionMap_file = NULL;
  
  char          *map_name = NULL;
  FILE          *map_fp = NULL;
  ppm_t          ppm_map;

  unsigned char *colorMap = NULL;
  int            n_colors = 0;
  int            regionMap_flag = 0;
  int           *regionMap = NULL;
  int            n_regions = 0;

  pnm_ppm_init(&ppm_map);

  //** process options and arguments ...

  program = ((strrchr(argv[0], '/')) ?
	     strrchr(argv[0], '/') + 1 : argv[0]);
  while ((c = getopt(argc, argv, "rco:i:m:Vh")) != EOF) {
    switch (c) {
    case 'h':
      fprintf(stderr, USAGE, program);
      fprintf(stderr, HELP_MSG);
      exit(0);
    case 'r':
      regionMap_flag = 1;
      break;
    case 'i':
      segment_ignore = (*optarg == '-') ? -2 : atoi(optarg);
      break;
    case 'm':
      segment_ignore_threshold = atoi(optarg);
      break;
    case 'o':
      contour_filename = optarg;
      break;
    case 'c':
      write_contour_files = 1;
      break;
    case 'V':
      fprintf(stderr,"%s: %s.",program,version);
      break;
    default:
      fprintf(stderr, USAGE, program);
      exit(1);
    }
  }
  if (optind >= argc) {
    fprintf(stderr, USAGE, program);
    exit(1);
  }

  //** read map ...

  if (strcmp(argv[optind],"-")) {
    if (*argv[optind]) {
      map_name = argv[optind];
      map_fp = fopen(map_name,"r");
      if (!map_fp) {
	fprintf(stderr, "%s: could not open file '%s'.\n", program, map_name);
	exit(1);
      }
     } else 
       map_fp = NULL;
  } else
    map_fp = stdin;

  if (map_fp) {
    ppm_map.pixels = ppm_readppm(map_fp, 
				 &ppm_map.cols,
				 &ppm_map.rows,
				 &ppm_map.maxval);
    if (!ppm_map.pixels) {
      fprintf(stderr, "%s: error while reading ppm image from '%s'.\n",
	      program, (map_fp == stdin) ? "<stdin>":map_name);
      exit(1);
    }
  }
  //** transform ppm_map to colorMap or regionMap

  if (map_fp) {
    if (regionMap_flag) {
      regionMap = (int *) malloc(ppm_map.cols*ppm_map.rows * sizeof(int));
      n_regions = pnm_ppmPseudoColor2intMap(regionMap, &ppm_map);
      //** make sure that marginal pixels have no region label
      pnm_setIntMargin(regionMap, ppm_map.cols, ppm_map.rows, -1);
      pnm_setIntMargin(regionMap+ppm_map.cols+1,ppm_map.cols-1,
		       ppm_map.rows, -1);
    } else {
      colorMap = (unsigned char *) 
	malloc(ppm_map.cols*ppm_map.rows * sizeof(unsigned char));
      n_colors = pnm_ppmPseudoColor2charMap(colorMap, &ppm_map);

      //** make regionMap from colorMap
      regionMap = (int *) malloc(ppm_map.cols*ppm_map.rows * sizeof(int));
      n_regions = regionlab(ppm_map.cols, ppm_map.rows, colorMap, n_colors,
			    regionMap);
      pnm_setIntMargin(regionMap+ppm_map.cols+1,ppm_map.cols-1,
		       ppm_map.rows, -1);
    }
  }

  {
    region_info_t *regions = (region_info_t *)
      malloc(n_regions * sizeof(region_info_t));
    region_info_t **regionPtrs = (region_info_t **)
      malloc(n_regions * sizeof(region_info_t *));
    int i;
    for (i=0; i < n_regions; i++) {
      region_info_init(regionPtrs[i]=&regions[i]);
    }
    region_countpixel(regionPtrs, regionMap, ppm_map.cols, ppm_map.rows,
		      colorMap);

    for (i=1; i < n_regions; i++) {
      region_contourByMaskOp(regionPtrs[i],
                             regionMap, ppm_map.cols, ppm_map.rows,
			     i, NULL, 1,1);
      /*region_contour(regionPtrs[i],
       *	     regionMap,  ppm_map.cols, ppm_map.rows, i, NULL);
       */
      region_contour2ppm(ppm_map.pixels, ppm_map.cols, 
			 regionPtrs[i]->n_contour,
			 regionPtrs[i]->contour, 0);
    }
    if (contour_filename) { 
      FILE *fp;
      fp = fopen(contour_filename,"w");
      ppm_writeppm(fp, ppm_map.pixels, ppm_map.cols, ppm_map.rows, 255, 1);
    }
  }
    
  exit(0);
}
