#ifndef VASSILIS_SIMPLE_ALGO_H
#define VASSILIS_SIMPLE_ALGO_H

#include <deque>
#include <vector>
#include <math.h>

#include "algebra.h"
#include "simple_image.h"


// This is an efficient version of vPickWithoutRepetitions from
// auxiliaries.cpp
vMatrix<vint8> sample_without_replacement(vint8 low, vint8 high, vint8 number);

vMatrix<vint8> vComplement(v3dMatrix<vint8> * matrix, vint8 start, vint8 end);

// Returns a matrix containing all integers from low to high, in random 
// order, i.e. a random permutation of the 
// tuple (low, low+1, ..., high-1, high).
vMatrix<vint8> vPermutation(vint8 low, vint8 high);


vMatrix<vint8> function_range_matrix(vint8 low, vint8 high);

integer_matrix integer_range_matrix(integer low, integer high);

//static inline vint8_matrix function_range_matrix (vint8 low, vint8 high)
//{
//  return vRangeMatrix(low, high);
//}

// returns a matrix with entres low, low+1, ..., high.
vMatrix<float> function_range_matrix(float low, float step, float high);

//static inline float_matrix function_range_matrix (float low, float step, float high)
//{
//  return vRangeMatrix(low, step, high);
//}


// eventually this function should go to algo_templates or general_templates.
// result[i] = j if values[j] is the i-th smallest entry in values.
vint8 function_string_ranks(vector<char *> * values, vector<vint8> * result);

// counts the number of pixels that are positive.  If the image has multiple 
// bands, then for each pixel we consider the some over all bands.
vint8 count_positive_pixels(general_image* image);

// counts the number of entries that are positive.  If the image has multiple
// bands, then the entry at each band is considered to be a separate entry.
vint8 count_positive_entries(general_image* image);

// the entry of each band is considered to be a separate entry.
uchar_image* threshold_entries (general_image* image, double threshold);

// the entry of each band is considered to be a separate entry.
uchar_image* inverse_threshold_entries (general_image* image, double threshold);

vint8 image_dimensions_equal (general_image* first_image, general_image* second_image);

// Each column of the result is an eigenvector of the input,
// if the input is a 2x2 matrix.
vMatrix<double> v2x2Eigenvectors(v3dMatrix<double> * input);

// An interactive function to test v2x2Eigenvectors.
vint8 vTest2x2Eigenvectors();

float_matrix convert_simple_float(simple_float_matrix * simple_image);

static inline vint8 function_histogram_bucket(double value, double minimum, 
                               double maximum, vint8 number)
{
  if (value >= maximum)
  {
    return number - 1;
  }
  if (value < minimum)
  {
    return 0;
  }

  double range = maximum - minimum;
  double bucket_size = range / (double) number;
  double normalized = (value - minimum) / bucket_size;
  vint8 result = round_number(floor(normalized));
  return result;
}


// the inverse of function_histogram_bucket
static inline double function_histogram_value(vint8 bucket, double minimum, 
                               double maximum, vint8 number)
{
  if (bucket >= number)
  {
    return maximum;
  }
  if (bucket <= 0)
  {
    return minimum;
  }

  double range = maximum - minimum;
  double bucket_size = range / (double) number;

  double initial = (double) bucket;
  double result = initial * bucket_size + minimum;
  return result;
}




#endif // VASSILIS_SIMPLE_ALGO_H


