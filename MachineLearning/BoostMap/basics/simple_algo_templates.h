#ifndef VASSILIS_SIMPLE_ALGO_TEMPLATES_H
#define VASSILIS_SIMPLE_ALGO_TEMPLATES_H

#include <vector>

#include "basics/matrix.h"
#include "basics/vplatform.h"
#include "basics/drawing_temp.h"

#include "algorithms/spline.h"



template<class type>
type kth_smallest_ca(long k, vector<type> * elements, long * indexp);

// Puts the min value of the vector into value and its index into index.
template<class type>
long function_vector_minimum(std::vector<double> * values, long * index, double * value);

// Puts the max value of the vector into value and its index into index.
template<class type>
long function_vector_maximum(std::vector<double> * values, long * index, double * value);

// Inserts the values (start, start+1, ..., end-1, end) to the end of
// the vector. 
template<class type>
long function_insert_range(vector<type> * values, vint8 start, vint8 end);

// Creates an image that contains all pixels (i,j) of color_image for which
// i appears in row_indices and j appears in col_indices. The pixels
// appear in the result as specified by the order in row_indices and
// col_indices. 
template<class type>
v3dMatrix<type> function_select_grid(v3dMatrix<type> * the_image,
                            vector<vint8> * row_indices,
                            vector<vint8> * col_indices);

// Applies the round_number function to each entry in color_image.
template<class type>
v3dMatrix<long> function_round_matrix(v3dMatrix<type> * the_image);

// matrix_from_vector creates a matrix with one row, whose entries
// are the same and in the same order as those of input.
template<class type>
vMatrix<type> matrix_from_vector(const vector<type> * input);

// vector_from_matrix inserts all entries of input into the back
// of output, in the order in which they appear in the input.
template<class type>
long vector_from_matrix(const v3dMatrix<type> * input, vector<type> * output);

// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
v3dMatrix<type> copy_horizontal_line(const v3dMatrix<type> * input, const vint8 row);

// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
vMatrix<type> copy_row(const v3dMatrix<type> * input, const vint8 row);

// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
vMatrix<type> copy_row(const v3dMatrix<type> * input, const vint8 band, const vint8 row);

// Returns a one-col matrix, consisting of the col-th col of the input
// matrix.
template<class type>
v3dMatrix<type> copy_vertical_line(const v3dMatrix<type> * input, const vint8 col);

template<class type>
double function_image_total(const v3dMatrix<type> * input);

template<class type>
double function_image_average(const v3dMatrix<type> * input);

// same as function_image_average, except that here we ignore pixels that are zero
template<class type>
double nonzero_average(v3dMatrix<type> * input);

template<class type>
double function_image_variance(v3dMatrix<type> * input);

template<class type>
double function_image_deviation(v3dMatrix<type> * input);

template<class type>
vint8 function_put_row(v3dMatrix<type> * source, v3dMatrix<type> * target, vint8 row);

template<class type> type kth_smallest_cb(vint8 k, v3dMatrix<type> * elements, 
                                         vint8 * indexp);

template<class type> type find_rank_ascending(vint8 rank, v3dMatrix<type> * elements, 
                                              vint8 * index_pointer)
{
  return kth_smallest_cb(rank, elements, index_pointer);
}

template<class type> type kth_smallest_da(vint8 k, vArray(type) entries, 
                                        vint8 size, vint8 * indexp);

template<class type> type find_rank_ascending(vint8 rank, vArray(type) entries, 
                                              vint8 size, vint8 * index_pointer)
{
  return kth_smallest_da(rank, entries, size, index_pointer);
}

template<class type>
type function_image_maximum(v3dMatrix<type> * the_image);

template<class type>
type function_image_maximum3(v3dMatrix<type> * the_image, vint8 * row, vint8 * col);

template<class type>
type color_imageRowMax3(v3dMatrix<type> * the_image, vint8 row, vint8 * col);
 
template<class type>
type color_imageColMax3(v3dMatrix<type> * the_image, vint8 col, vint8 * max_row);
 
template<class type>
type function_image_minimum(v3dMatrix<type> * the_image);

template<class type>
type function_image_minimum3(v3dMatrix<type> * the_image, vint8 * row, vint8 * col);

template<class type>
type color_imageRowMin3(v3dMatrix<type> * the_image, vint8 row, vint8 * col);
 
template<class type>
type color_imageColMin3(v3dMatrix<type> * the_image, vint8 col, vint8 * max_row);
 
// This function puts the minimum value encountered in color_image into
// min_value and the maximum value into max_value. However, it ignores
// all values that are not in the interval [lower, upper].
template<class type>
ushort function_image_minimumMaxBounded(v3dMatrix<type> * the_image, 
                            type * min_value, type * max_value,
                            double lower, double upper);


// This function puts the minimum value encountered in color_image into
// min_value and the maximum value into max_value.
template<class type>
ushort function_image_minimumMax(v3dMatrix<type> * the_image, 
                     type * min_value, type * max_value);

// Returns number of pixels in the image that have the given value
// (the pixels in each band are considered separate).
template<class type>
long vCountValue(v3dMatrix<type> * the_image, type value);

// Returns number of pixels in the image for which value >= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountLequal(v3dMatrix<type> * the_image, type value);

// Returns number of pixels in the image for which value <= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountGequal(v3dMatrix<type> * the_image, type value);

// Returns number of pixels in the given band of the image
// that have the given value
// (the pixels in each band are considered separate).
template<class type>
long vCountBandValue(v3dMatrix<type> * the_image, long band, type value);

// Returns number of pixels in the given band of the image 
// for which value >= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountBandLequal(v3dMatrix<type> * the_image, long band, type value);

// Returns number of pixels in the given band of the image 
// for which value <= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountBandGequal(v3dMatrix<type> * the_image, long band, type value);


template<class type>
type function_image_median(v3dMatrix<type> * the_image);

template<class type1, class type2>
v3dMatrix<double> * function_image_add(v3dMatrix<type1> * the_image1, 
				 v3dMatrix<type2> * the_image2);

template<class type1, class type2>
v3dMatrix<double> * function_image_subtract(v3dMatrix<type1> * the_image1, 
				      v3dMatrix<type2> * the_image2);

template<class type1, class type2>
v3dMatrix<double> * function_image_multiply(v3dMatrix<type1> * the_image1, 
				      v3dMatrix<type2> * the_image2);

template<class type1, class type2>
v3dMatrix<double> * function_image_divide(v3dMatrix<type1> * the_image1, 
				    v3dMatrix<type2> * the_image2);

template<class type>
v3dMatrix<double> * function_image_add(v3dMatrix<type> * the_image1, 
				 double number);

template<class type>
v3dMatrix<double> * function_image_subtract(v3dMatrix<type> * the_image1, 
				      double number);

template<class type>
v3dMatrix<double> * function_image_multiply(v3dMatrix<type> * the_image1, 
				      double number);

template<class type>
v3dMatrix<double> * function_image_divide(v3dMatrix<type> * the_image1, 
				    double number);

template<class type>
v3dMatrix<type> * function_image_absolute(v3dMatrix<type> * the_image1);

template<class type1, class type2>
void function_image_absolute(v3dMatrix<type1> * the_image1, v3dMatrix<type2> * result);

template<class type>
v3dMatrix<type> * function_image_power(const v3dMatrix<type> * input, double power);

template<class type1, class type2, class type3>
void function_image_add(v3dMatrix<type1> * the_image1, 
		v3dMatrix<type2> * the_image2,
		v3dMatrix<type3> * result);

template<class type1, class type2, class type3>
void function_image_subtract(v3dMatrix<type1> * the_image1, 
		     v3dMatrix<type2> * the_image2,
		     v3dMatrix<type3> * result);

template<class type1, class type2, class type3>
void function_image_multiply(v3dMatrix<type1> * the_image1, 
		     v3dMatrix<type2> * the_image2,
		     v3dMatrix<type3> * result);

template<class type1, class type2, class type3>
void function_image_divide(v3dMatrix<type1> * the_image1, 
		   v3dMatrix<type2> * the_image2,
		   v3dMatrix<type3> * result);

template<class type1, class type2>
void function_image_add(v3dMatrix<type1> * the_image1, double number,
		v3dMatrix<type2> * result);

template<class type1, class type2>
void function_image_subtract(v3dMatrix<type1> * the_image1, double number, 
		     v3dMatrix<type2> * result);

template<class type1, class type2>
void function_image_multiply(v3dMatrix<type1> * the_image1, double number,
		     v3dMatrix<type2> * result);

template<class type1, class type2>
void function_image_divide(v3dMatrix<type1> * the_image1, double number,
		   v3dMatrix<type2> * result);

template<class type1>
v3dMatrix<type1> image_square_root(v3dMatrix<type1> * the_image1);

template<class type1, class type2>
void image_square_root(v3dMatrix<type1> * the_image1, v3dMatrix<type2> * result);

template <class type>
double_matrix make_cumulative_histogram(vMatrix<type> histogram);

template <class type>
double_matrix make_histogram(std::vector <type> * scores, double histogram_minimum,
                             double histogram_maximum, long bucket_number);

template <class type>
double_matrix make_histogram(v3dMatrix<type> scores, double histogram_minimum,
                           double histogram_maximum, long bucket_number);

template <class type>
double_matrix normalize_histogram(vMatrix<type> histogram);

template <class type>
double_matrix make_normalized_histogram(std::vector <type> * scores, double histogram_minimum,
                                        double histogram_maximum, long bucket_number);

template <class type>
long function_extract_values(vMatrix<type> image, std::vector<class_pixel> * locations,
                             std::vector<type> * result);

template <class type>
vint8_matrix maximizing_channels(v3dMatrix<type> * image);

template <class type>
vMatrix<type> matrix_from_vector(const vector<type> * data, const long vertical, const long horizontal);

// in this function, for each pixel, the value in each band is treated separately.
// we return a binary image with a zero entry for 3D locations 
// whose entries in the original image are not among the smallest.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
uchar_image find_smallest_entries(v3dMatrix<type> * image, long rank);

// in this function, for each pixel, the value in each band is treated separately.
// we return an image with a zero entry for 3D locations 
// whose entries in the original image are not among the smallest.
// the nonzero entries are equal to the entries of the corresponding locations
// in the original image.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
v3dMatrix<type> keep_smallest_entries(v3dMatrix<type> * image, long rank,
                                      type replacement);

// in this function, for each pixel, the value in each band is treated separately.
// we return a binary image with a zero entry for 3D locations 
// whose entries in the original image are not among the largest.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
uchar_image find_largest_entries(v3dMatrix<type> * image, long rank);

// in this function, for each pixel, the value in each band is treated separately.
// we return an image with a zero entry for 3D locations 
// whose entries in the original image are not among the largest.
// the nonzero entries are equal to the entries of the corresponding locations
// in the original image.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
v3dMatrix<type> keep_largest_entries(v3dMatrix<type> * image, long rank,
                                      type replacement);

template<class type> type find_rank_descending(vint8 rank, v3dMatrix<type> * elements, 
                                               vint8 * index_pointer);

template<class type> long is_in_matrix(const type value, const v3dMatrix<type> * matrix);

// if entry is in elements, we return a copy of elements,
// otherwise we return a matrix including all elements and entry
template<class type> vMatrix<type> insert_entry(const vMatrix<type> & elements, type entry);

// if entry is not in elements, we return a copy of elements,
// otherwise we return a matrix including all elements minus entry
template<class type> vMatrix<type> remove_entry(const vMatrix<type> & elements, type entry);





#ifndef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#include "simple_algo_templates.cpp"
#endif // VASSILIS_SGI_PLATFORM




#endif // VASSILIS_SIMPLE_ALGO_TEMPLATES_H

