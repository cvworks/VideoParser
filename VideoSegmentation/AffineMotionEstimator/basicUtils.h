
//#define fprintf(fp, fmt, args...) fprintf (stdout, fmt, ## args)

#ifdef WIN32
#define fprintf(fp, fmt, ...) 
#else
#define fprintf(fp, fmt, args...) 
#endif

void error(char *s1, char *s2);
void invert_image(float *image, int nx, int ny);
void average_images(float *image1, float *image2, float *mean, int nx, int ny);
float linear_interp(float low_value, float high_value, float position);
void pad_string(char *fnCurr, int pad, int im);
void read_image_float(float *image, char *fnIn, int nx, int ny);
void read_image_strip_bytes(float *image, char *fnIn, 
			    int nx, int ny, int bytes);
void copy_image(float *to, float *from, int nx, int ny);
void ones_image(float *image, int nx, int ny);
void zero_image(float *image, int nx, int ny);
void add_image(float *image, float *add, int nx, int ny);
void subtract_image(float *image, float *sub, int nx, int ny);
void abs_image(float *image, int nx, int ny);
void divide_image(float *image, float divisor, int nx, int ny);
void mul_images(float *image1, float *image2, int nx, int ny);
float max_image_val(float *image, int nx, int ny);
float min_image_val(float *image, int nx, int ny);
void save_pgm(char *path, float *image, int nx, int ny);
void save_pgm_no_scale(char *path, float *image, int nx, int ny);
float image_abs_max(float *image, int nx, int ny);
float image_max(float *image, int nx, int ny);
float image_min(float *image, int nx, int ny);


