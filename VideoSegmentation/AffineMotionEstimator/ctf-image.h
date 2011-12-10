void pyramid_regress(float *im1, float *im2, int level,
		     float *a0, float *a1, float *a2, float *a3, 
		     float *a4, float *a5, float *Ix, float *Iy, float *It, 
		     int nx, int ny, int iters, float sigma, float sigma_init, 
		     float omega, float *omask, float *weights, int filter, 
		     int affine, float rate);
