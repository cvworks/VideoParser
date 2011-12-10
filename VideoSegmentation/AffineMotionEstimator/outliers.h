void outliers(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
	      float a3, float a4, float a5, int nx, int ny, float sigma, 
	      float *outlier_im);
void good_points(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
		 float a3, float a4, float a5, int nx, int ny, float sigma, 
		 float *outlier_im);
void robust_to_weights(float *Ix, float *Iy, float *It, float a0, float a1, float a2, 
	      float a3, float a4, float a5, int nx, int ny, float sigma, 
	      float *weight_im);


