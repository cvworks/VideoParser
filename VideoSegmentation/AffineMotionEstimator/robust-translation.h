
float regress_1_iter(float *grad, float *Ix, float *Iy, float *It, 
		     float u, float v, int nx, int ny, float sigma,
		     float omega, float *omask, 
		     int x, int y, int dx, int dy);

void robust_regress_region(float *im1, float *im2, float *u, float *v, 
			   float *Ix, float *Iy, float *It, int nx, int ny,
			   int iters, float sigma, float sigma_init, 
			   float omega, float *omask, 
			   int x, int y, int dx, int dy, float rate);
