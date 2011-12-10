/*
  This is the main routine that does the gradient descent on the
  affine parameters.
*/
float affine_regress_1_iter(float *Ix, float *Iy, float *It, 
			    float a0, float *a1, float *a2, float a3, 
			    float *a4, float *a5, int nx, int ny, float sigma,
			    float omega, float *omask);

/* 
   This just does the iterations calling affine_regress_1_iter
   while lowering the sigma value according the annealing schedule.
*/
void regress_affine(float *im1, float *im2, float a0, float *a1, float *a2, 
		    float a3, float *a4, float *a5, float *Ix, float *Iy, 
		    float *It, int nx, int ny, int iters, float sigma, 
		    float sigma_init, float omega, float *omask, float rate);
