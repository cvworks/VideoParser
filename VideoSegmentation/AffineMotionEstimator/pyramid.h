
void sub_sample_image(float *image, float *result, int nx, int ny);
void convolve3(float *input, int nx, int ny, float filter[3][3],
	       int filter_size, float output, int sample);
void reduce_image(float *image, float *reduced, int nx, int ny);
void reduce_flow(float *input, float *output, int nx, int ny);
void project_with_interp(float *image, float *proj, int nx, int ny, float scale);
void project_image(float *image, float *proj, int nx, int ny);
void project_flow(float *image, float *proj, int nx, int ny);

