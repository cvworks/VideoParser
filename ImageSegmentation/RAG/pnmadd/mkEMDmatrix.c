#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "png.h"
#include "pnmemd.h"
#include <list/list.h>
#include <list/string+.h>

char *program;

int main(int argc, char *argv[])
{
  pgm_t pgm1;
  pgm_t pgm2;
  FILE *fp;
  FILE *fp_out;
  FILE *row_filelist_fp = NULL;
  char  line[1024];
  char *path="";
  int  i,k,c;
  int  n_features = 50;
  char *row_filelist_name = NULL;

  lsPt_t filenames;
  lsPt_t row_filenames;
  lsPt_t pgms;
  lsDouble_t emd_matrix;
  
  lsPt_nil(&filenames);
  lsPt_nil(&row_filenames);
  pnm_pgm_init(&pgm1);
  pnm_pgm_init(&pgm2);


  pnm_png_verbose = 0;

  program = ((strrchr(argv[0], '/')) ?
	     strrchr(argv[0], '/') + 1 : argv[0]);

  while ((c = getopt(argc, argv, "hd:n:f:")) != EOF) {
    switch (c) {
	case 'd':
	    path = optarg;
	    break;
	case 'n':
	    n_features = atoi(optarg);
	    break;
	case 'f':
	    row_filelist_name = optarg;
	    break;
	default:
	    ;
    }
  }  

  if (argc - optind < 2) {
      fprintf(stderr, "USAGE: %s [-d<path>-n<emd-features>-f<file-list>] <file-list> <output>\n",program);
      exit(1);
  }

  if (row_filelist_name) {
      row_filelist_fp = fopen(row_filelist_name,"r");
      if (!row_filelist_fp) {
	  fprintf(stderr, "%s: cannot open file '%s'.\n", program,
		  row_filelist_name);
	  exit(1);
      }
  }

  if (strcmp(argv[optind],"-")) {
      fp = fopen(argv[optind],"r");
      if (!fp) {
	  fprintf(stderr, "%s: cannot open file '%s'.\n",program,
		  argv[optind]);
	  exit(1);
      }
  } else {
      fp = stdin;
  }
  
  if (strcmp(argv[++optind],"-")) {
      fp_out = fopen(argv[optind],"w");
      if (!fp_out) {
	  fprintf(stderr, "%s: cannot open file '%s'.\n",program,
		  argv[optind]);
	  exit(1);
      }
  } else {
      fp_out = stdout;
  }

  while (fgets(line,1024,fp)) {
      
      char *p_end = strpbrk(line,"\n\t ");
      if (p_end) *p_end = '\0';

      lsPt_add(&filenames, strCpy(line));
  }
  fprintf(stderr,"%s: %d filenames read.\n", program, LS_N(&filenames));

  if (row_filelist_fp) {
      while (fgets(line,1024,row_filelist_fp)) {
	  
	  char *p_end = strpbrk(line,"\n\t ");
	  if (p_end) *p_end = '\0';

	  lsPt_add(&row_filenames, strCpy(line));
      }
  } 
      
  lsPt_nil(&pgms);
  LS_FORALL_ITEMS(&filenames,i) {

      char *filename = strCat(path, LS_GET(&filenames,i));
      pgm_t *pgm = pnm_pgm_create();

      fp = fopen(filename,"r");
      if (!fp) {
	  fprintf(stderr, "%s: cannot open png-file '%s'.\n",program,filename);
	  exit(1);
      }
      if (pnm_png_verbose > 0) {
	  fprintf(stderr,"%s: reading file '%s'.\n",program, filename);
      }
      pnm_pgm_readImage(pgm,fp,argv[0]);
      fclose(fp);

      lsPt_add(&pgms,pgm);
  }
  fprintf(stderr,"%s: %d images read.\n", program, LS_N(&pgms));

  lsDouble_nil(&emd_matrix);
  if (!row_filelist_fp) {
      lsDouble_realloc(&emd_matrix, LS_N(&pgms) * LS_N(&pgms));
  } else {
      lsDouble_realloc(&emd_matrix, LS_N(&pgms) * LS_N(&row_filenames));
  }

  fprintf(stderr,"%s: computing emd with feature weight mass %d.\n",
	  program, n_features);
  k=0;

  if (!row_filelist_fp) {
      LS_FORALL_ITEMS(&pgms,i) {
	  int j;
	  pgm_t *pgm1 = LS_GET(&pgms,i);
	  
	  fprintf(stderr,"%s: computing %d '%s' ...\n", program, i, 
		  LS_GET(&filenames,i));
	  
	  LS_FORALL_ITEMS(&pgms,j) {
	      pgm_t *pgm2 = LS_GET(&pgms,j);
	      
	      float emd = pnm_pgm_emd(pgm1,pgm2, n_features, 255);
	      LS_GET(&emd_matrix,k++) = emd;
	      /*fprintf(stderr," %g:%lf", emd, LS_GET(&emd_matrix,k-1));*/
	  }
      }
  } else {
      pgm_t *pgm1 = pnm_pgm_create();

      LS_FORALL_ITEMS(&row_filenames,i) {
	  int j;

	  char *row_file = LS_GET(&row_filenames,i);
	  FILE *row_fp = fopen(row_file,"r");
	  if (!row_fp) {
	      fprintf(stderr, "%s: cannot open png-file '%s'.\n",
		      program, row_file);
	      exit(1);
	  }
	  if (pnm_png_verbose > 0) {
	      fprintf(stderr,"%s: reading file '%s'.\n",program, row_file);
	  }
	  pnm_pgm_readImage(pgm1, row_fp, argv[0]);
	  fclose(row_fp);
	  
	  fprintf(stderr,"%s: computing %d '%s' ...\n", program, i, 
		  LS_GET(&row_filenames,i));

	  LS_FORALL_ITEMS(&pgms,j) {
	      pgm_t *pgm2 = LS_GET(&pgms,j);
	      
	      float emd = pnm_pgm_emd(pgm1,pgm2, n_features, 255);
	      LS_GET(&emd_matrix,k++) = emd;
	      /*fprintf(stderr," %g:%lf", emd, LS_GET(&emd_matrix,k-1));*/
	  }
      }
  }	  
  LS_N(&emd_matrix) = k;

  fprintf(stderr,"%s: writing matrix (%d x %d) %d doubles ...\n", program,
	  LS_N(&emd_matrix) / LS_N(&pgms), LS_N(&pgms), LS_N(&emd_matrix));
  {
      int params[3];
      params[0] = LS_N(&pgms); params[1] = 0; 
      params[2] = (LS_N(&emd_matrix) / LS_N(&pgms)) - 1;
      fwrite(params, sizeof(int), 3, fp_out);
      lsDouble_fwrite(fp_out, &emd_matrix);
  }
  exit(0);
}

      


  





