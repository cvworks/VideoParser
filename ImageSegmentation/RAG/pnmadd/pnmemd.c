#include <math.h>
#include "pnmemd.h"
#include "label.h"

pnm_labelmap_t *pnm_labelmap_init(pnm_labelmap_t *pgml,
				  const int *map, int cols, int rows,
				  int n_labels,
				  const int *labels)
{
    int i;
    int *pixel_row;
    if (!pgml)
	pgml = (pnm_labelmap_t *) malloc(sizeof(pnm_labelmap_t));

    pgml->map = (int **) malloc(rows * sizeof(int *));
    for (i=0, pixel_row = (int *) map; i < rows; i++, pixel_row += cols)
	pgml->map[i] = pixel_row;
    pgml->cols = cols;
    pgml->rows = rows;
    pgml->n_labels = n_labels;
    pgml->labels = labels;

    return (pgml);
}

pnm_labelmap_t *pnm_labelmap_create(const int *map, int cols, int rows,
				    int n_labels,
				    const int *labels)
{
    return (pnm_labelmap_init(NULL,map,cols,rows,n_labels,labels));
}

void pnm_labelmap_free(pnm_labelmap_t *pgml)
{
    if (!pgml) return;
    if (pgml->map) free(pgml->map);
}
	
void pnm_labelmap_destroy(pnm_labelmap_t *pgml)
{
    if (!pgml) return;
    pnm_labelmap_free(pgml);
    free(pgml);
}

float pnm_dist(emd_feature_t *F1, emd_feature_t *F2)
{
  int dX = F1->X - F2->X, dY = F1->Y - F2->Y, dZ = F1->Z - F2->Z;
  return sqrt(dX*dX + dY*dY + dZ*dZ);
}

float pnm_ppm_emd(ppm_t * ppm1, ppm_t * ppm2, int num, pixel bg)
{
    int i,nb1,nb2,nb_min,size1,size2;
    float fl,nb,scale1,scale2;
    emd_signature_t s1,s2;
    nb1 = pnm_ppm_numOfPix(ppm1,bg);
    nb2 = pnm_ppm_numOfPix(ppm2,bg);
    
    nb_min = (nb1<nb2) ? ((nb1<num) ? nb1 : num) : ((nb2<num) ? nb2 : num);

    pnm_ppm_signature(&s1,ppm1,nb_min,bg);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s1.n,nb_min,num);
     */
    pnm_ppm_signature(&s2,ppm2,nb_min,bg);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s2.n,nb_min,num);
     */
    fl = emd_normalized(&s1, &s2, pnm_dist, 0, 0);

    emd_signature_free(&s1);
    emd_signature_free(&s2);
    return fl;
}

float pnm_pgm_emd(pgm_t * pgm1, pgm_t * pgm2, int num, gray bg)
{
    int i,nb1,nb2,nb_min,size1,size2;
    float fl,nb,scale1,scale2;
    emd_signature_t s1,s2;
    nb1 = pnm_pgm_numOfPix(pgm1,bg);
    nb2 = pnm_pgm_numOfPix(pgm2,bg);
    
    nb_min = (nb1<nb2) ? ((nb1<num) ? nb1 : num) : ((nb2<num) ? nb2 : num);

    pnm_pgm_signature(&s1,pgm1,nb_min,bg);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s1.n,nb_min,num);
     */
    pnm_pgm_signature(&s2,pgm2,nb_min,bg);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s2.n,nb_min,num);
     */
    fl = emd(&s1, &s2, pnm_dist, 0, 0);
    /*fl = emd_normalized(&s1, &s2, pnm_dist, 0, 0);*/

    emd_signature_free(&s1);
    emd_signature_free(&s2);
    return fl;
}

float pnm_labelmap_emd(pnm_labelmap_t *pgml1, 
		       pnm_labelmap_t *pgml2,
		       int num)
{
    int i,nb1,nb2,nb_min,size1,size2;
    float fl,nb,scale1,scale2;
    emd_signature_t s1,s2;
    nb1 = pnm_labelmap_numOfPix(pgml1);
    nb2 = pnm_labelmap_numOfPix(pgml2);
    
    nb_min = (nb1<nb2) ? ((nb1<num) ? nb1 : num) : ((nb2<num) ? nb2 : num);

    pnm_labelmap_signature(&s1,pgml1,nb_min);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s1.n,nb_min,num);
     */
    pnm_labelmap_signature(&s2,pgml2,nb_min);
    /*fprintf(stderr, "Signature: n=%d, nb_min=%d, num=%d.\n",s2.n,nb_min,num);
     */
    fl = emd(&s1, &s2, pnm_dist, 0, 0);
    /*fl = emd_normalized(&s1, &s2, pnm_dist, 0, 0);*/

    emd_signature_free(&s1);
    emd_signature_free(&s2);
    return fl;
}

int pnm_ppm_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		 float *x_center, float *y_center, ppm_t *ppm, pixel bg)
{
    int i,n;
    *x_min =ppm->cols;
    *y_min =ppm->rows;
    *x_max =0;
    *y_max =0;
    *x_center = 0.0;
    *y_center = 0.0;

    for (i=0,n=0;i <ppm->rows;i++){
	int j;
	pixel * current_row = ppm->pixels[i];
	for (j=0;j<ppm->cols;j++){
	    if (!PPM_EQUAL(bg,current_row[j])) {
		*x_center += j;
		*y_center += i;
		n++;
		if (j< *x_min){
		    *x_min = j;
		}
		if (i< *y_min){
		    *y_min = i;
		}
		if (j>*x_max){
		    *x_max = j;
		}
		*y_max = i;
		
	    }
	}
    }
    if (n > 0) {
	*x_center /= (float) n;
	*y_center /= (float) n;
    }
    return n;
}

int pnm_ppm_numOfPix(ppm_t *ppm, pixel bg)
{
    int i,n;

    for (i=0,n=0;i <ppm->rows;i++){
	int j;
	pixel * current_row = ppm->pixels[i];
	for (j=0;j<ppm->cols;j++){
	    if (!PPM_EQUAL(current_row[j],bg)) n++;
	}
    }
    return (n);
}

int pnm_pgm_numOfPix(pgm_t *pgm, gray  bg)
{
    int i,n;

    for (i=0,n=0;i <pgm->rows;i++){
	int j;
	gray * current_row = pgm->pixels[i];
	for (j=0;j<pgm->cols;j++){
	    if (bg !=  current_row[j]) n++;
	}
    }
    return (n);
}

int pnm_labelmap_numOfPix(pnm_labelmap_t *pgml)
{
    int i,n;
    int labelMax = -1;
    int *labelFlags = NULL;

    if (!pgml) return (0);

    /* generate lookup-table for labels */
    labelFlags = pnm_label_createLookup(&labelMax, pgml->n_labels, pgml->labels);

    /* count pixels */
    for (i=0,n=0;i < pgml->rows;i++){
	int j;
	int *current_row = pgml->map[i];
	for (j=0;j<pgml->cols;j++){
	    if (pnm_label_lookup(labelMax, labelFlags, current_row[j])) n++;
	}
    }
    free(labelFlags);

    return (n);
}
	    
int pnm_pgm_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		 float *x_center, float *y_center, pgm_t *pgm, gray  bg)
{
    int i,n;
    *x_min =pgm->cols;
    *y_min =pgm->rows;
    *x_max =0;
    *y_max =0;
    *x_center = 0.0;
    *y_center = 0.0;
    for (i=0,n=0;i <pgm->rows;i++){
	int j;
	gray * current_row = pgm->pixels[i];
	for (j=0;j<pgm->cols;j++){
	    if (bg !=  current_row[j]) {
		*x_center += j;
		*y_center += i;
		n++;
		if (j< *x_min){
		    *x_min = j;
		}
		if (i< *y_min){
		    *y_min = i;
		}
		if (j>*x_max){
		    *x_max = j;
		}
		*y_max = i;
		
	    }
	}
    }
    if (n > 0) {
	*x_center /= (float) n;
	*y_center /= (float) n;
    }
    return n;
}

int pnm_labelmap_bbox(int *x_min, int *y_min, int *x_max, int *y_max,
		      float *x_center, float *y_center,
		      pnm_labelmap_t *pgml)
{
    int i,n;
    int labelMax=-1, *labelFlags=NULL;
    
    labelFlags = pnm_label_createLookup(&labelMax, pgml->n_labels, pgml->labels);

    *x_min =pgml->cols;
    *y_min =pgml->rows;
    *x_max =0;
    *y_max =0;
    *x_center = 0.0;
    *y_center = 0.0;
    for (i=0,n=0;i <pgml->rows;i++){
	int j;
	int * current_row = pgml->map[i];
	for (j=0;j<pgml->cols;j++){
	    if (pnm_label_lookup(labelMax, labelFlags,current_row[j])) {
		*x_center += j;
		*y_center += i;
		n++;
		if (j< *x_min){
		    *x_min = j;
		}
		if (i< *y_min){
		    *y_min = i;
		}
		if (j>*x_max){
		    *x_max = j;
		}
		*y_max = i;
		
	    }
	}
    }
    if (n > 0) {
	*x_center /= (float) n;
	*y_center /= (float) n;
    }
    free(labelFlags);

    return n;
}


float pnm_ppm_sigWeight(ppm_t *ppm, float x, float y, float x_max,
			float y_max, float scale2, pixel bg)
{
    int i,i_max,n=0;
    float result =0,bpix=0;
    int first_y=1;

    // Rand-Pixel duerfen nur zum Teil gezaehlt werden !! -- swachsmu
    //size = (size<(x2-x1)*(y2-y1))? (x2-x1)*(y2-y1) :size;
    
    /* Innerer ganzer Kasten */
    for (i=(int)y+1, i_max=(int)y_max; i<i_max; i++){	
	int j, j_max;
	pixel *current_row = ppm->pixels[i];
	for (j=(int)x+1, j_max=(int)x_max; j<j_max; j++){
	    bpix += (!PPM_EQUAL(current_row[j],bg));
	    n++;
	}
    }
    n=0;
    /* Hier ist irgendwo noch ein Fehler -- ffrenser */
    {
	int j;
	float dx1, dx2, dy1, dy2;

	pixel *current_row = ppm->pixels[(int)y];

        /* obere fehlende Teilzeile addieren */
	dy1 = ceil(y) - y;
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy1 * (!PPM_EQUAL(current_row[i],bg));
	    n++;
	}
	n=0;
	current_row = ppm->pixels[(int)y_max];

	/* untere fehlende Teilzeile addieren */
	dy2 = y_max - floor(y_max);
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy2 * (!PPM_EQUAL(current_row[i],bg));
	    n++;
	}
	n=0;
	
        /* linke fehlende Teilspalte  addieren */
	dx1 = (ceil(x) - x);
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x; i<i_max; i++){
		bpix += dx1 * (!PPM_EQUAL(ppm->pixels[i][j],bg));
		n++;
	}

	n=0;

	/* rechte fehlende Teilspalte  addieren */
	dx2 = (x_max - floor(x_max));
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x_max; i<i_max; i++){
	    bpix += dx2 * (!PPM_EQUAL(ppm->pixels[i][j],bg));
	    n++;
	}
	n=0;
	/* linke obere Ecke */
	bpix += (dy1 * dx1) * (!PPM_EQUAL(ppm->pixels[(int)y][(int)x],bg));
	/* rechte obere Ecke */
	bpix += (dy1 * dx2) * (!PPM_EQUAL(ppm->pixels[(int)y][(int)x_max],bg));
	/* linke untere Ecke */
	bpix += (dy2 * dx1) * (!PPM_EQUAL(ppm->pixels[(int)y_max][(int)x],bg));
	/* rechte untere Ecke */
	bpix += (dy2 * dx2) * (!PPM_EQUAL(ppm->pixels[(int)y_max][(int)x_max],bg));

	result = bpix / scale2;

	return result;
    }
}

float pnm_pgm_sigWeight(pgm_t *pgm, float x, float y, float x_max,
			float y_max, float scale2, gray bg)
{
    int i,i_max,n=0;
    float result =0,bpix=0;
    int first_y=1;

    //size = (size<(x2-x1)*(y2-y1))? (x2-x1)*(y2-y1) :size;
    
    /* Innerer ganzer Kasten */
    for (i=(int)y+1, i_max=(int)y_max; i<i_max; i++){	
	int j, j_max;
	gray *current_row = pgm->pixels[i];
	for (j=(int)x+1, j_max=(int)x_max; j<j_max; j++){
	    bpix += (current_row[j] != bg);
	    n++;
	}
    }
    n=0;
    /* Hier ist irgendwo noch ein Fehler -- ffrenser */
    {
	int j;
	float dx1, dx2, dy1, dy2;

	gray *current_row = pgm->pixels[(int)y];

        /* obere fehlende Teilzeile addieren */
	dy1 = ceil(y) - y;
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy1 * (current_row[i] != bg);
	    n++;
	}
	n=0;
	current_row = pgm->pixels[(int)y_max];

	/* untere fehlende Teilzeile addieren */
	dy2 = y_max - floor(y_max);
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy2 * (current_row[i] != bg);
	    n++;
	}
	n=0;
	
        /* linke fehlende Teilspalte  addieren */
	dx1 = (ceil(x) - x);
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x; i<i_max; i++){
		bpix += dx1 * (pgm->pixels[i][j] != bg);
		n++;
	}

	n=0;

	/* rechte fehlende Teilspalte  addieren */
	dx2 = (x_max - floor(x_max));
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x_max; i<i_max; i++){
	    bpix += dx2 * (pgm->pixels[i][j] != bg);
	    n++;
	}
	n=0;

	/* linke obere Ecke */
	bpix += (dy1 * dx1) * (pgm->pixels[(int)y][(int)x] != bg);
	/* rechte obere Ecke */
	bpix += (dy1 * dx2) * (pgm->pixels[(int)y][(int)x_max] != bg);
	/* linke untere Ecke */
	bpix += (dy2 * dx1) * (pgm->pixels[(int)y_max][(int)x] != bg);
	/* rechte untere Ecke */
	bpix += (dy2 * dx2) * (pgm->pixels[(int)y_max][(int)x_max] != bg);

	result = bpix / scale2;

	return result;
    }
}

float pnm_labelmap_sigWeight(pnm_labelmap_t *pgml, 
			     float x, float y, float x_max,
			     float y_max, float scale2)

{
    int i,i_max,n=0;
    float result =0,bpix=0;
    int first_y=1;
    int labelMax=-1, *labelFlags=NULL;

    labelFlags = pnm_label_createLookup(&labelMax, pgml->n_labels, pgml->labels);

    //size = (size<(x2-x1)*(y2-y1))? (x2-x1)*(y2-y1) :size;
    
    /* Innerer ganzer Kasten */
    for (i=(int)y+1, i_max=(int)y_max; i<i_max; i++){	
	int j, j_max;
	int *current_row = pgml->map[i];
	for (j=(int)x+1, j_max=(int)x_max; j<j_max; j++){
	    bpix += pnm_label_lookup(labelMax, labelFlags,current_row[j]);
	    n++;
	}
    }
    n=0;
    /* Hier ist irgendwo noch ein Fehler -- ffrenser */
    {
	int j;
	float dx1, dx2, dy1, dy2;

	int *current_row = pgml->map[(int)y];

        /* obere fehlende Teilzeile addieren */
	dy1 = ceil(y) - y;
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy1 * pnm_label_lookup(labelMax, labelFlags,current_row[i]);
	    n++;
	}
	n=0;
	current_row = pgml->map[(int)y_max];

	/* untere fehlende Teilzeile addieren */
	dy2 = y_max - floor(y_max);
	for (i=(int)x+1, i_max=(int)x_max; i<i_max; i++){
	    bpix += dy2 * pnm_label_lookup(labelMax, labelFlags,current_row[i]);
	    n++;
	}
	n=0;
	
        /* linke fehlende Teilspalte  addieren */
	dx1 = (ceil(x) - x);
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x; i<i_max; i++){
		bpix += dx1 * pnm_label_lookup(labelMax, labelFlags, 
					  pgml->map[i][j]);
		n++;
	}

	n=0;

	/* rechte fehlende Teilspalte  addieren */
	dx2 = (x_max - floor(x_max));
	for (i=(int)y+1, i_max=(int)y_max, j=(int)x_max; i<i_max; i++){
	    bpix += dx2 * pnm_label_lookup(labelMax, labelFlags,
				      pgml->map[i][j]);
	    n++;
	}
	n=0;

	/* linke obere Ecke */
	bpix += (dy1 * dx1) 
	    * pnm_label_lookup(labelMax, labelFlags, 
			  pgml->map[(int)y][(int)x]);
	/* rechte obere Ecke */
	bpix += (dy1 * dx2) 
	    * pnm_label_lookup(labelMax, labelFlags, 
			  pgml->map[(int)y][(int)x_max]);
	/* linke untere Ecke */
	bpix += (dy2 * dx1) 
	    * pnm_label_lookup(labelMax, labelFlags,
			  pgml->map[(int)y_max][(int)x]);
	/* rechte untere Ecke */
	bpix += (dy2 * dx2) 
	    * pnm_label_lookup(labelMax, labelFlags,
			  pgml->map[(int)y_max][(int)x_max]);

	result = bpix / scale2;

	free(labelFlags);

	return result;
    }
}

emd_signature_t *pnm_ppm_signature(emd_signature_t *sig, ppm_t *ppm,
				   int num, pixel bg)
{
    int t,nb,x_max,x_min,y_max,y_min,x_next,y_next;
    float scale2, scale, y, max_x ,max_y,sum_weight=0.0,gpx,gpy;

    if (!sig)
	sig = (emd_signature_t *) malloc(sizeof(emd_signature_t));
    sig->n=0;

    nb = pnm_ppm_bbox (&x_min, &y_min, &x_max,&y_max, &gpx,&gpy,
		       ppm, bg);
    if (nb <= 0) return (sig);

    scale2 = (nb > num) ? (float) nb / (float) num : 1.0;
    scale = sqrt(scale2);
    
    t = (x_max - x_min) * (y_max - y_min); 
    sig->Features = (emd_feature_t*) calloc (t,sizeof(emd_feature_t));
    sig->Weights = (float*) calloc (t,sizeof(float));
    
    /*Normierung des Schwerpunkts*%
     *gpx = gpx/scale;
     *gpy = gpy/scale;
     */

    for (y=y_min, y_next=y_min; y_next!=y_max; y+=scale){
	float x;
	y_next = (y_max < y+scale)? y_max : y+scale;	/*ist das ende erreicht?*/
	for (x=x_min, x_next=x_min; x_next != x_max; x+=scale){
	    float weight;

	    x_next = (x_max < x+scale)? x_max : x+scale;	/*ist das ende erreicht?*/
	    weight = pnm_ppm_sigWeight(ppm,x,y,x_next,y_next,scale2,bg);

	    if (sig->n >= t) {
		fprintf (stderr, "pnm_ppm_signature: signature too big.");
		exit(1);
	    }
	    if (weight > 0.0) {
		emd_feature_t f;
		sum_weight += weight;
		
		/*Schwerpunkt- und Koordinatennormierung*/
		
		f.X = (int) floor((x-gpx)/scale);
		f.Y = (int) floor((y-gpy)/scale);
		f.Z = 0;
		sig->Features[sig->n]=f;
		sig->Weights[sig->n]=weight;
		(sig->n)++;
	    }
	}
    }
    if (sig->n > 0) {
	fprintf(stderr,"pnm_labelmap_signature: realloc %d.\n", sig->n);
	sig->Features = (emd_feature_t*) 
	    realloc (sig->Features, sig->n * sizeof(emd_feature_t));
	sig->Weights = (float*) 
	    realloc (sig->Weights, sig->n * sizeof(float));
    } else {
	fprintf(stderr,"pnm_labelmap_signature: realloc %d.\n", sig->n);
	if (sig->Features) free(sig->Features);
	if (sig->Weights) free(sig->Weights);
	sig->Features = NULL;
	sig->Weights = NULL;
    }
    return sig;
}

emd_signature_t *pnm_pgm_signature(emd_signature_t *sig, pgm_t *pgm, 
				   int num, gray bg)
{
    int t,nb,x_max,x_min,y_max,y_min;
    float scale, scale2, y, x_next , y_next,sum_weight=0.0,gpx,gpy;

    if (!sig)
	sig = (emd_signature_t *) malloc(sizeof(emd_signature_t));
    sig->n=0;

    nb = pnm_pgm_bbox (&x_min, &y_min, &x_max,&y_max, &gpx,&gpy,
		       pgm, bg);
    /*fprintf(stderr, "(%d,%d-%d,%d), (%g,%g)\n",
     *	    x_min, y_min, x_max, y_max, gpx, gpy);
     */
    if (nb <= 0) return (sig);

    scale2 = (nb > num) ? (float) nb / (float) num : 1.0;
    scale = sqrt(scale2);

    /*fprintf(stderr, "Scale: %d / %d = %g -> %g\n", nb, num, scale2, scale);*/

    t = (x_max - x_min) * (y_max - y_min); 
    sig->Features = (emd_feature_t*) calloc (t,sizeof(emd_feature_t));
    sig->Weights = (float*) calloc (t,sizeof(float));
     
    /*Normierung des Schwerpunkts*%
     *gpx = gpx/scale;
     *gpy = gpy/scale;
     */
    for (y=y_min, y_next=y_min; y_next!=y_max; y+=scale){
	float x;
	y_next = (y_max < y+scale)? y_max : y+scale;	/*ist das ende erreicht?*/
	for (x=x_min, x_next=x_min; x_next != x_max; x+=scale){
	    float weight;

	    x_next = (x_max < x+scale)? x_max : x+scale;	/*ist das ende erreicht?*/
	    weight = pnm_pgm_sigWeight(pgm,x,y,x_next,y_next,scale2,bg);

	    if (sig->n >= t) {
		fprintf (stderr, "pnm_pgm_signature: signature too big.");
		exit(1);
	    }
	    if (weight > 0.0) {
		emd_feature_t f;
		sum_weight += weight;
		
		/*Schwerpunkt- und Koordinatennormierung */
		
		f.X = (int) floor((x-gpx)/scale);
		f.Y = (int) floor((y-gpy)/scale);
		f.Z = 0;
		sig->Features[sig->n]=f;
		sig->Weights[sig->n]=weight;

		/*fprintf(stderr,"feature: (%g,%g) %d %d %g\n", x,y,
		 *	sig->Features[sig->n].X,
		 *	sig->Features[sig->n].Y, sig->Weights[sig->n]);
		 */	
		(sig->n)++;
	    }
	}
    }
    sig->Features = (emd_feature_t*) 
	realloc (sig->Features, sig->n * sizeof(emd_feature_t));
    sig->Weights = (float*) 
	realloc (sig->Weights, sig->n * sizeof(float));

    return sig;
}


emd_signature_t *
pnm_labelmap_signature(emd_signature_t *sig, pnm_labelmap_t *pgml, int num)
{
    int t,nb,x_max,x_min,y_max,y_min;
    float scale, scale2, y, x_next , y_next,sum_weight=0.0,gpx,gpy;

    if (!sig)
	sig = (emd_signature_t *) malloc(sizeof(emd_signature_t));
    sig->n=0;

    nb = pnm_labelmap_bbox (&x_min, &y_min, &x_max,&y_max, &gpx,&gpy, pgml);

    /*fprintf(stderr, "(%d,%d-%d,%d), (%g,%g)\n",
     *	    x_min, y_min, x_max, y_max, gpx, gpy);
     */
    if (nb <= 0) return (sig);

    scale2 = (nb > num) ? (float) nb / (float) num : 1.0;
    scale = sqrt(scale2);

    /*fprintf(stderr, "Scale: %d / %d = %g -> %g\n", nb, num, scale2, scale);*/

    t = (x_max - x_min) * (y_max - y_min); 
    sig->Features = (emd_feature_t*) calloc (t,sizeof(emd_feature_t));
    sig->Weights = (float*) calloc (t,sizeof(float));
     
    /*Normierung des Schwerpunkts*%
     *gpx = gpx/scale;
     *gpy = gpy/scale;
     */
    for (y=y_min, y_next=y_min; y_next!=y_max; y+=scale){
	float x;
	y_next = (y_max < y+scale)? y_max : y+scale;	/*ist das ende erreicht?*/
	for (x=x_min, x_next=x_min; x_next != x_max; x+=scale){
	    float weight;

	    x_next = (x_max < x+scale)? x_max : x+scale;	/*ist das ende erreicht?*/
	    weight = pnm_labelmap_sigWeight(pgml,x,y,x_next,y_next,scale2);

	    if (sig->n >= t) {
		fprintf (stderr, "pnm_pgm_signature: signature too big.");
		exit(1);
	    }
	    if (weight > 0.0) {
		emd_feature_t f;
		sum_weight += weight;
		
		/*Schwerpunkt- und Koordinatennormierung */
		
		f.X = (int) floor((x-gpx)/scale);
		f.Y = (int) floor((y-gpy)/scale);
		f.Z = 0;
		sig->Features[sig->n]=f;
		sig->Weights[sig->n]=weight;

		/*fprintf(stderr,"feature: (%g,%g) %d %d %g\n", x,y,
		 *	sig->Features[sig->n].X,
		 *	sig->Features[sig->n].Y, sig->Weights[sig->n]);
		 */	
		(sig->n)++;
	    }
	}
    }
    sig->Features = (emd_feature_t*) 
	realloc (sig->Features, sig->n * sizeof(emd_feature_t));
    sig->Weights = (float*) 
	realloc (sig->Weights, sig->n * sizeof(float));

    return sig;
}






