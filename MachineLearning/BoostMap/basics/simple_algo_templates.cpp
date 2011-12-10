// Some of these functions used to be in algo_templates.h, but I 
// decided to make a separate file with the most "portable" functions.

#include "vplatform.h"

#ifdef VASSILIS_PROCESS_FILE
#undef VASSILIS_PROCESS_FILE

#include <iostream>
#include <algorithm>

#include <math.h>

#include "basics/matrix.h"
#include "basics/algebra.h"
#include "basics/simple_algo.h"

#include "basics/definitions.h"

using namespace std;
// Finds the k-th smallest element in the array or vector.
template<class type>
type kth_smallest_ca(long k, vector<type> * elements, long * indexp)
{
  long size = elements->size();
  vMatrix<type> scratch_matrix(3, size);
  vArray(type) scratch1 = scratch_matrix.Matrix2()[0];
  vArray(type) scratch2 = scratch_matrix.Matrix2()[1];
  vArray(type) scratch3 = scratch_matrix.Matrix2()[2];

  long i;
  for (i = 0; i < size; i++)
  {
    scratch1[i] = (*elements)[i];
  }
  long in_size = size;
  long small_found = 0;

  vArray(type) in = scratch1;
  vArray(type) out1 = scratch2;
  vArray(type) out2 = scratch3;
  long index1, index2;
  type pivot;

  while(small_found < k)
  {
    vint8 pivot_index = function_random_vint8(0, in_size-1);
    pivot = in[pivot_index];

    index1 = 0;
    index2 = 0;
    vint8 i;
    for (i = 0; i < in_size; i++)
    {
      type number = in[i];
      if (number <= pivot)
      {
        out1[index1++] = number;
      }
      else
      {
        out2[index2++] = number;
      }
    }

    // Check for the pathological case where all elements are 
    // equal to the pivot.
    if (index1 == in_size)
    {
      long break_flag = 1;
      long j;
      for (j = 0; j < index1; j++)
      {
        if (out1[j] < pivot)
        {
          break_flag = 0;
          break;
        }
      }
      if (break_flag == 1) break;
    }


    if (small_found + index1 <= k)
    {
      small_found = small_found + index1;
      vArray(type) temp = in;
      in = out2;
      out2 = temp;
      in_size = index2;
    }
    else
    {
      vArray(type) temp = in;
      in = out1;
      out1 = temp;
      in_size = index1;
    }
    
  }

  type result = pivot;

  *indexp = -1;
  for (i = 0; i < size; i++)
  {
    if ((*elements)[i] == result)
    {
      *indexp = i;
      break;
    }
  }

  if (*indexp == -1)
  {
    exit_error("Error: Bug in kth_smallest_ca\n");
  }
  return result;
}


// Puts the min value of the vector into value and its index into index.
template<class type>
long function_vector_minimum(std::vector<type> * values, vint8 * index, type * value)
{
  long size = values->size();
  if (size == 0) return 0;
  long i;
  long min_index = 0;
  type min_value = (*values)[0];
  for (i = 1; i < size; i++)
  {
    type temp_value = (*values)[i];
    if (temp_value < min_value) 
    {
      min_value = temp_value;
      min_index = i;
    }
  }
  *index = min_index;
  *value = min_value;
  return 1;
}


// Puts the max value of the vector into value and its index into index.
template<class type>
long function_vector_maximum(std::vector<type> * values, long * index, type * value)
{
  long size = values->size();
  if (size == 0) return 0;
  long i;
  long max_index = 0;
  type max_value = (*values)[0];
  for (i = 1; i < size; i++)
  {
    type temp_value = (*values)[i];
    if (temp_value > max_value) 
    {
      max_value = temp_value;
      max_index = i;
    }
  }
  *index = max_index;
  *value = max_value;
  return 1;
}


// Inserts the values (start, start+1, ..., end-1, end) to the end of
// the vector. 
template<class type>
long function_insert_range(vector<type> * values, vint8 start, vint8 end)
{
  vint8 i;
  for (i = start; i <= end; i++)
  {
    values->push_back((type) i);
  }
  return 1;
}


// Creates an image that contains all pixels (i,j) of color_image for which
// i appears in row_indices and j appears in col_indices. The pixels
// appear in the result as specified by the order in row_indices and
// col_indices. 
template<class type>
v3dMatrix<type> function_select_grid(v3dMatrix<type> * the_image,
                            vector<vint8> * row_indices,
                            vector<vint8> * col_indices)
{
  vint8 bands = the_image->Bands();
  vint8 rows = row_indices->size();
  vint8 cols = col_indices->size();
  v3dMatrix<type> result(rows, cols, bands);

  vArray3(type) in_data = the_image->Matrix3();
  vArray3(type) out_data = result.Matrix3();

  vint8 i, j;
  for (i = 0; i < rows; i++)
  {
    vint8 row = (*row_indices)[(vector_size) i];
    for (j = 0; j < cols; j++)
    {
      vint8 col = (*col_indices)[(vector_size) j];
      if (the_image->check_bounds(row, col) <= 0)
      {
        exit_error("Error in function_select_grid: %li %li %li %li\n",
                        i, j, row, col);
      }
      vint8 k;
      for (k = 0; k < bands; k++)
      {
        out_data[k][i][j] = in_data[k][row][col];
      }
    }
  }

  return result;
}


// Applies the round_number function to each entry in color_image.
template<class type>
v3dMatrix<long> function_round_matrix(v3dMatrix<type> * the_image)
{
  long rows = the_image->Rows();
  long cols = the_image->Cols();
  long bands = the_image->Bands();

  v3dMatrix<long> result(rows, cols, bands);
  vArray(type) in = the_image->Matrix();
  vArray(long) out = result.Matrix();

  long i;
  long size = rows * cols * bands;
  for (i = 0; i < size; i++)
  {
    out[i] = round_number(in[i]);
  }

  return result;
}


// matrix_from_vector creates a matrix with one row, whose entries
// are the same and in the same order as those of input.
template<class type>
vMatrix<type> matrix_from_vector(const vector<type> * input)
{
  long size = input->size();
  if (size == 0)
  {
    return vMatrix<type>();
  }
  
  vMatrix<type> result(1, size);
  vArray(type) output = result.Matrix();
  
  long i;
  for (i = 0; i < size; i++)
  {
    output[i] = (*input)[i];
  }

  return result;
}


// vector_from_matrix inserts all entries of input into the back
// of output, in the order in which they appear in the input.
template<class type>
long vector_from_matrix(const v3dMatrix<type> * input, vector<type> * output)
{
  output->clear();

  vArray(type) data = input->Matrix();
  vint8 size = input->Size();

  vint8 i;
  for (i = 0; i < size; i++)
  {
    output->push_back(data[i]);
  }

  return 1;
}


// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
v3dMatrix<type> copy_horizontal_line(const v3dMatrix<type> * input, const vint8 row)
{
  vint8 rows = input->Rows();
  vint8 cols = input->Cols();
  vint8 bands = input->Bands();

  if ((row < 0) || (row >= rows))
  {
    function_warning("Warning: bad row argument %li (%li) to copy_horizontal_line\n", row, rows);
    return v3dMatrix<type>();
  }

  v3dMatrix<type> result(1, cols, bands);
  vArray3(type) in = input->Matrix3();
  vArray3(type) out = result.Matrix3();

  vint8 band, col;
  for (band = 0; band < bands; band++)
  {
    for (col = 0; col < cols; col++)
    {
      out[band][0][col] = in[band][row][col];
    }
  }
  
  return result;
}



// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
vMatrix<type> copy_row(const v3dMatrix<type> * input, const vint8 row)
{
  vint8 rows = input->Rows();
  vint8 cols = input->Cols();
  vint8 bands = input->Bands();

  if (bands != 1){
	function_warning("Warning: bad 2D matrix. Bands = %li \n", bands);
    return vMatrix<type>();
  }

  if ((row < 0) || (row >= rows))
  {
    function_warning("Warning: bad row argument %li (%li) to copy_row\n", row, rows);
    return vMatrix<type>();
  }

  vMatrix<type> result(1, cols);
  vArray3(type) in = input->Matrix3();
  vArray2(type) out = result.Matrix2();

  vint8 col;

  for (col = 0; col < cols; col++){
    out[0][col] = in[0][row][col];
  }

  
  return result;
}

// Returns a one-row matrix, consisting of the row-th row of the input
// matrix.
template<class type>
vMatrix<type> copy_row(const v3dMatrix<type> * input, const vint8 band, const vint8 row)
{
  vint8 rows = input->Rows();
  vint8 cols = input->Cols();
  vint8 bands = input->Bands();

  if (band >= bands){
	function_warning("Warning: band argument %li outside of range %li \n", (long)band, (long)bands);
    return vMatrix<type>();
  }

  if ((row < 0) || (row >= rows))
  {
    function_warning("Warning: bad row argument %li (%li) to copy_row\n", row, rows);
    return vMatrix<type>();
  }

  vMatrix<type> result(1, cols);
  vArray3(type) in = input->Matrix3();
  vArray2(type) out = result.Matrix2();

  vint8 col;

  for (col = 0; col < cols; col++){
    out[0][col] = in[band][row][col];
  }

  
  return result;
}

// Returns a one-col matrix, consisting of the col-th col of the input
// matrix.
template<class type>
v3dMatrix<type> copy_vertical_line(const v3dMatrix<type> * input, const vint8 col)
{
  vint8 rows = input->Rows();
  vint8 cols = input->Cols();
  vint8 bands = input->Bands();

  if ((col < 0) || (col >= cols))
  {
    function_warning("Warning: bad col argument %li (%li) to copy_vertical_line\n", col, cols);
    return v3dMatrix<type>();
  }

  v3dMatrix<type> result(rows, 1, bands);
  vArray3(type) in = input->Matrix3();
  vArray3(type) out = result.Matrix3();

  vint8 band, row;
  for (band = 0; band < bands; band++)
  {
    for (row = 0; row < rows; row++)
    {
      out[band][row][0] = in[band][row][col];
    }
  }
  
  return result;
}

template<class type>
double function_image_total(const v3dMatrix<type> * input)
{
  double sum = 0;
  vint8 size = input->Size();
  vArray(type) matrix = input->Matrix();

  vint8 i;
  for (i = 0; i < size; i++)
  {
    sum = sum + matrix[i];
  }

  return sum;
}


template<class type>
double function_image_average(const v3dMatrix<type> * input)
{
  double size = (double) input->Size();
  if (size == 0)
  {
    function_warning("\nWarning: empty image passed to function_image_average\n");
    return 0;
  }

  double sum = function_image_total(input);
  double mean = sum / size;

  return mean;
}


template<class type>
v3dMatrix<type> * function_image_power(const v3dMatrix<type> * input, double power)
{
  vint8 vertical = input->vertical();
  vint8 horizontal = input->horizontal();
  vint8 channels = input->channels();
  v3dMatrix<type> * output = new v3dMatrix<type>(vertical, horizontal, channels);

  vint8 size = input->Size();
  vArray(type) input_matrix = input->Matrix();
  vArray(type) output_matrix = output->Matrix();

  vector_size i;
  for (i = 0; i < size; i++)
  {
    output_matrix[i] = (type) (pow((double) input_matrix[i], power));
  }

  return output;
}


// same as function_image_average, except that here we ignore pixels that are zero
template<class type>
double nonzero_average(v3dMatrix<type> * input)
{
  vint8 size = input->Size();
  if (size == 0)
  {
    function_warning("\nWarning: empty image passed to function_image_average\n");
    return 0;
  }

  double sum = 0.0f;
  vint8 number = 0;
  vArray(type) matrix = input->Matrix();

  vector_size counter;
  for (counter = 0; counter < size; counter++)
  {
    type entry = matrix[counter];
    if (entry == (type) 0)
    {
      continue;
    }
    sum = sum + entry;
    number++;
  }

  if (number == 0)
  {
    return 0.0;
  }

  double mean = sum / number;
  return mean;
}


template<class type>
double function_image_variance(v3dMatrix<type> * input)
{
  double size = (double) input->Size();
  if (size <= 1)
  {
    function_warning("\nWarning: empty or single-entry image passed to function_image_average\n");
    return 0;
  }

  double mean = function_image_average(input);
  v3dMatrix<double> * centered = function_image_subtract(input, mean);
  v3dMatrix<double> * squared = function_image_multiply(centered, centered);
  double sum = function_image_total(squared);
  double variance = sum / (size-1);
  vdelete(centered);
  vdelete(squared);
  return variance;
}


template<class type>
double distribution_mean(const vector<type> & input)
{
  long size = (long) input.size();
  if (size == 0)
  {
    exit_error("error: empty or single-entry image passed to distribution_mean\n");
    return 0;
  }

  long counter;
  double total = 0.0f;
  for (counter = 0; counter < size; counter++)
  {
    const type entry = input[counter];
    total += (double) entry;
  }

  double result = total / (double) (size);
  return result;
}


template<class type>
double distribution_covariance(const vector<type> & input, type mean)
{
  long size = (long) input.size();
  if (size <= 1)
  {
    exit_error("error: empty or single-entry image passed to distribution_covariance\n");
    return 0;
  }

  long counter;
  double total = 0.0f;
  for (counter = 0; counter < size; counter++)
  {
    const type entry = input[counter];
    const type difference = mean - entry;
    total += (double) (difference * difference);
  }

  double result = total / (double) (size-1);
  return result;
}



template<class type>
double function_image_deviation(v3dMatrix<type> * input)
{
  double variance = function_image_variance(input);
  double std = sqrt(variance);
  return std;
}


// this function is kind of
// the reverse of copy_horizontal_line. Assumes that source only has one row,
// and writes that row into row row of target.
template<class type>
vint8 function_put_row(v3dMatrix<type> * source, v3dMatrix<type> * target, vint8 row)
{
  if (source->Rows() != 1)
  {
    exit_error("error: in function_put_row, source->rows = %li\n",
                    source->Rows());
  }

  if (source->Bands() != target->Bands())
  {
    exit_error("error: in function_put_row, different bands: %li %li\n",
                    source->Bands(), target->Bands());
  }

  if (source->Cols() != target->Cols())
  {
    exit_error("error: in function_put_row, different cols: %li %li\n",
                    source->Cols(), target->Cols());
  }

  if (target->Rows() <= row)
  {
    exit_error("error: in function_put_row, row = %li, target rows = %li\n",
                    row, target->Rows());
  }

  vArray3(type) source_data = source->Matrix3();
  vArray3(type) target_data = target->Matrix3();

  vint8 bands = source->Bands();
  vint8 cols = source->Cols();
  vint8 band, col;
  for (band = 0; band < bands; band++)
  {
    vArray(type) source_row = source_data[band][0];
    vArray(type) target_row = target_data[band][row];
    for (col = 0; col < cols; col++)
    {
      target_row[col] = source_row[col];
    }
  }

  return 1;
}


template<class type> type kth_smallest_cb(vint8 k, v3dMatrix<type> * elements, 
                                          vint8 * indexp)
{
  vint8 size = elements->AllBandSize();
  vArray(type) entries = elements->Matrix();

  return kth_smallest_da(k, entries, (vint8) size, indexp);
}


template<class type> type kth_smallest_da(vint8 k, vArray(type) entries, 
                                        vint8 size, vint8 * indexp)
{
  if ((k < 1) || (k > size))
  {
    *indexp = -1;
    return (type) 0;
  }

  vMatrix<type> scratch_matrix(3, size);
  vArray(type) scratch1 = scratch_matrix.Matrix2()[0];
  vArray(type) scratch2 = scratch_matrix.Matrix2()[1];
  vArray(type) scratch3 = scratch_matrix.Matrix2()[2];

  vint8 i;
  for (i = 0; i < size; i++)
  {
    scratch1[i] = entries[i];
  }
  vint8 in_size = size;
  vint8 small_found = 0;

  vArray(type) in = scratch1;
  vArray(type) out1 = scratch2;
  vArray(type) out2 = scratch3;
  vint8 index1, index2;
  type pivot;

  while(small_found < k)
  {
    vint8 pivot_index = function_random_vint8(0, in_size-1);
    pivot = in[pivot_index];

    index1 = 0;
    index2 = 0;
    vint8 i;
    for (i = 0; i < in_size; i++)
    {
      type number = in[i];
      if (number <= pivot)
      {
        out1[index1++] = number;
      }
      else
      {
        out2[index2++] = number;
      }
    }

    // Check for the pathological case where all elements are 
    // equal to the pivot.
    if (index1 == in_size)
    {
      vint8 break_flag = 1;
      vint8 j;
      for (j = 0; j < index1; j++)
      {
        if (out1[j] < pivot)
        {
          break_flag = 0;
          break;
        }
      }
      if (break_flag == 1) break;
    }


    if (small_found + index1 <= k)
    {
      small_found = small_found + index1;
      vArray(type) temp = in;
      in = out2;
      out2 = temp;
      in_size = index2;
    }
    else
    {
      vArray(type) temp = in;
      in = out1;
      out1 = temp;
      in_size = index1;
    }
  }

  type result = pivot;

  if (indexp != 0)
  {
    *indexp = -1;
    for (i = 0; i < size; i++)
    {
      if (entries[i] == result)
      {
        *indexp = i;
        break;
      }
    }

    if (*indexp == -1)
    {
      exit_error("Error: Bug in kth_smallest_da\n");
    }
  }

  return result;
}


template<class type>
type function_image_maximum(v3dMatrix<type> * the_image)
{
  vint8 size = the_image->AllBandSize();
  vArray(type) values = the_image->Matrix();
  type max = values[0];

  for (vint8 i = 1; i < size; i++)
    if (values[i] > max) max = values[i];
  return max;
}
 
 
// Return the row and col location of the max entry in the image.
// Now that I am looking at it (3/14/2003) it seems it has a bug,
// and it would only work for one-band images.
template<class type>
type function_image_maximum3(v3dMatrix<type> * the_image, vint8 * row, vint8 * col)
{
  if (the_image->Bands() != 1) 
  {
    exit_error("function_image_maximum3: function cannot handle more than one band.\n");
  }
  vint8 size = the_image->AllBandSize();
  vArray(type) values = the_image->Matrix();
  type max = values[0];
  vint8 max_i = 0;

  for (vint8 i = 1; i < size; i++)
    if (values[i] > max) 
    {
      max = values[i];
      max_i = i;
    }
  *row = max_i / the_image->Cols();
  *col = max_i % the_image->Cols();
  return max;
}

template<class type>
type color_imageRowMax3(v3dMatrix<type> * the_image, vint8 row, vint8 * max_col)
{
  vint8 size = the_image->AllBandSize();
  vArray2(type) values2 = the_image->Matrix2(0);
  type max = values2[row][0];
  *max_col = 0;
  vint8 cols = the_image->Cols();
  vint8 col;

  for (col = 1; col < cols; col++)
  {
    if (values2[row][col] > max) 
    {
      max = values2[row][col];
      *max_col = col;
    }
  }
  return max;
}
 

template<class type>
type color_imageColMax3(v3dMatrix<type> * the_image, vint8 col, vint8 * max_row)
{
  vint8 size = the_image->AllBandSize();
  vArray2(type) values2 = the_image->Matrix2(0);
  type max = values2[0][col];
  *max_row = 0;
  vint8 rows = the_image->Rows();
  vint8 row;

  for (row = 1; row < rows; row++)
  {
    if (values2[row][col] > max) 
    {
      max = values2[row][col];
      *max_row = row;
    }
  }
  return max;
}

 

template<class type>
type function_image_minimum(v3dMatrix<type> * the_image)
{
  vint8 size = the_image->AllBandSize();
  vArray(type) values = the_image->Matrix();
  type min = values[0];

  for (vint8 i = 1; i < size; i++)
    if (values[i] < min) min = values[i];
  return min;
}


template<class type>
type function_image_minimum3(v3dMatrix<type> * the_image, vint8 * row, vint8 * col)
{
  vint8 size = the_image->AllBandSize();
  vArray(type) values = the_image->Matrix();
  type min = values[0];
  vint8 min_i = 0;

  for (vint8 i = 1; i < size; i++)
    if (values[i] < min) 
    {
      min = values[i];
      min_i = i;
    }
  *row = min_i / the_image->Cols();
  *col = min_i % the_image->Cols();
  return min;
}

template<class type>
type color_imageRowMin3(v3dMatrix<type> * the_image, vint8 row, vint8 * min_col)
{
  vint8 size = the_image->AllBandSize();
  vArray2(type) values2 = the_image->Matrix2(0);
  type min = values2[row][0];
  *min_col = 0;
  vint8 cols = the_image->Cols();
  vint8 col;

  for (col = 1; col < cols; col++)
  {
    if (values2[row][col] < min) 
    {
      min = values2[row][col];
      *min_col = col;
    }
  }
  return min;
}
 

template<class type>
type color_imageColMin3(v3dMatrix<type> * the_image, vint8 col, vint8 * min_row)
{
  vint8 size = the_image->AllBandSize();
  vArray2(type) values2 = the_image->Matrix2(0);
  type min = values2[0][col];
  *min_row = 0;
  vint8 rows = the_image->Rows();
  vint8 row;

  for (row = 1; row < rows; row++)
  {
    if (values2[row][col] < min) 
    {
      min = values2[row][col];
      *min_row = row;
    }
  }
  return min;
}

 

// This function puts the minimum value encountered in color_image into
// min_value and the maximum value into max_value. However, it ignores
// all values that are not in the interval [lower, upper].
//
// It returns 0 if no pixels were found in that interval.
template<class type>
ushort function_image_minimumMaxBounded(v3dMatrix<type> * the_image, 
                            type * min_value, type * max_value,
                            double lower, double upper)
{
  vint8 size = the_image->AllBandSize();
  vArray(type) values = the_image->Matrix();
  type current_max = (type) (lower - 1);
  type current_min = (type) (upper + 1);

  for (vint8 i = 0; i < size; i++)
  {
    type value = values[i];
    if (value < lower) continue;
    if (value > upper) continue;
    
    if (value < current_min) current_min = value;
    if (value > current_max) current_max = value;
  }

  // current_min > upper only in the case where no pixels in the image
  // were in the interval [lower, upper].
  if (current_min > upper) return 0;
  *min_value = current_min;
  *max_value = current_max;
  return 1;
}


// Returns number of pixels in the image that have the given value
// (the pixels in each band are considered separate).
template<class type>
long vCountValue(v3dMatrix<type> * the_image, type value)
{
  vArray(type) values = the_image->Matrix();
  vint8 size = the_image->AllBandSize();

  vector_size i;
  long result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] == value) result++;
  }
  return result;
}


// Returns number of pixels in the image for which value >= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountLequal(v3dMatrix<type> * the_image, type value)
{
  vArray(type) values = the_image->Matrix();
  vint8 size = the_image->AllBandSize();

  vector_size i;
  long result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] <= value) result++;
  }
  return result;
}


// Returns number of pixels in the image for which value <= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountGequal(v3dMatrix<type> * the_image, type value)
{
  vArray(type) values = the_image->Matrix();
  vint8 size = the_image->AllBandSize();

  vector_size i;
  long result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] >= value) result++;
  }
  return result;
}


// Returns number of pixels in the given band of the image
// that have the given value
// (the pixels in each band are considered separate).
template<class type>
vint8 vCountBandValue(v3dMatrix<type> * the_image, long band, type value)
{
  vint8 bands = the_image->Bands();
  if ((band < 0) || (band >= bands)) return -1;

  vArray(type) values = the_image->Matrix(band);
  vint8 size = the_image->Size();

  vector_size i;
  vint8 result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] == value) result++;
  }
  return result;
}


// Returns number of pixels in the given band of the image 
// for which value >= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountBandLequal(v3dMatrix<type> * the_image, long band, type value)
{
  long bands = the_image->Bands();
  if ((band < 0) || (band >= bands)) return -1;

  vArray(type) values = the_image->Matrix(band);
  long size = the_image->Size();

  long i;
  long result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] <= value) result++;
  }
  return result;
}


// Returns number of pixels in the given band of the image 
// for which value <= given value
// (the pixels in each band are considered separate).
template<class type>
long vCountBandGequal(v3dMatrix<type> * the_image, long band, type value)
{
  long bands = the_image->Bands();
  if ((band < 0) || (band >= bands)) return -1;

  vArray(type) values = the_image->Matrix(band);
  long size = the_image->Size();

  long i;
  long result = 0;
  for (i = 0; i < size; i++)
  {
    if (values[i] >= value) result++;
  }
  return result;
}


// This function puts the minimum value encountered in color_image into
// min_value and the maximum value into max_value. It returns 0 if
// the size of the image is 0.
template<class type>
ushort function_image_minimumMax(v3dMatrix<type> * the_image, 
                     type * min_value, type * max_value)
{
  vint8 size = the_image->AllBandSize();
  if (size == 0) return 0;
  vArray(type) values = the_image->Matrix();
  type current_max = values[0];
  type current_min = values[0];

  for (long i = 1; i < size; i++)
  {
    type value = values[i];
    if (value < current_min) current_min = value;
    if (value > current_max) current_max = value;
  }

  *min_value = current_min;
  *max_value = current_max;
  return 1;
}

  
template<class type1, class type2>
v3dMatrix<double> * function_image_add(v3dMatrix<type1> * the_image1, 
				 v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_add(the_image1, the_image2, result);
  return result;
}

template<class type1, class type2>
v3dMatrix<double> * function_image_subtract(v3dMatrix<type1> * the_image1, 
				      v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_subtract(the_image1, the_image2, result);
  return result;
}


template<class type1, class type2>
v3dMatrix<float> * function_image_subtract_float(v3dMatrix<type1> * the_image1, 
				      v3dMatrix<type2> * the_image2)
{
  long rows = the_image1->Rows();
  long cols = the_image1->Cols();
  long bands = the_image1->Bands();
  v3dMatrix<float> * result = new v3dMatrix<float>(rows, cols, bands);
  function_image_subtract(the_image1, the_image2, result);
  return result;
}


template<class type1, class type2>
v3dMatrix<double> * function_image_multiply(v3dMatrix<type1> * the_image1, 
				      v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_multiply(the_image1, the_image2, result);
  return result;
}


template<class type1, class type2>
v3dMatrix<float> * function_image_multiply_float(v3dMatrix<type1> * the_image1, 
				                                   v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<float> * result = new v3dMatrix<float>(rows, cols, bands);
  function_image_multiply(the_image1, the_image2, result);
  return result;
}


template<class type1, class type2>
v3dMatrix<double> * function_image_divide(v3dMatrix<type1> * the_image1, 
				    v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_divide(the_image1, the_image2, result);
  return result;
}


template<class type1, class type2>
v3dMatrix<float> * function_image_divide_float(v3dMatrix<type1> * the_image1, 
				    v3dMatrix<type2> * the_image2)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<float> * result = new v3dMatrix<float>(rows, cols, bands);
  function_image_divide(the_image1, the_image2, result);
  return result;
}


template<class type>
v3dMatrix<double> * function_image_add(v3dMatrix<type> * the_image1, 
				 double number)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_add(the_image1, number, result);
  return result;
}


template<class type>
v3dMatrix<double> * function_image_subtract(v3dMatrix<type> * the_image1, 
				      double number)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_subtract(the_image1, number, result);
  return result;
}


template<class type>
v3dMatrix<double> * function_image_multiply(v3dMatrix<type> * the_image1, 
				      double number)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_multiply(the_image1, number, result);
  return result;
}


template<class type>
v3dMatrix<double> * function_image_divide(v3dMatrix<type> * the_image1, 
				    double number)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, bands);
  function_image_divide(the_image1, number, result);
  return result;
}


template<class type1, class type2, class type3>
void function_image_add(v3dMatrix<type1> * the_image1, 
		v3dMatrix<type2> * the_image2,
		v3dMatrix<type3> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == the_image2->Rows());
  assert(cols == the_image2->Cols());
  assert(bands == the_image2->Bands());
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix2 = the_image2->Matrix();
  vArray(type3) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
    matrix3[i] = ((type3) matrix1[i]) + ((type3) matrix2[i]);
}


template<class type1, class type2, class type3>
void function_image_subtract(v3dMatrix<type1> * the_image1, 
		     v3dMatrix<type2> * the_image2,
		     v3dMatrix<type3> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == the_image2->Rows());
  assert(cols == the_image2->Cols());
  assert(bands == the_image2->Bands());
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix2 = the_image2->Matrix();
  vArray(type3) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
    matrix3[i] = ((type3) matrix1[i]) - ((type3) matrix2[i]);
}


template<class type1, class type2, class type3>
void function_image_multiply(v3dMatrix<type1> * the_image1, 
		     v3dMatrix<type2> * the_image2,
		     v3dMatrix<type3> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == the_image2->Rows());
  assert(cols == the_image2->Cols());
  assert(bands == the_image2->Bands());
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix2 = the_image2->Matrix();
  vArray(type3) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
    matrix3[i] = ((type3) matrix1[i]) * ((type3) matrix2[i]);
}


template<class type1, class type2, class type3>
void function_image_divide(v3dMatrix<type1> * the_image1, 
		   v3dMatrix<type2> * the_image2,
		   v3dMatrix<type3> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == the_image2->Rows());
  assert(cols == the_image2->Cols());
  assert(bands == the_image2->Bands());
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix2 = the_image2->Matrix();
  vArray(type3) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
    matrix3[i] = ((type3) matrix1[i]) / ((type3) matrix2[i]);
}


template<class type1, class type2>
void function_image_add(v3dMatrix<type1> * the_image1, double number,
		            v3dMatrix<type2> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
  {
    matrix3[i] = (type2) (matrix1[i] + number);
  }
}


template<class type1, class type2>
void function_image_subtract(v3dMatrix<type1> * the_image1, double number, 
		     v3dMatrix<type2> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());

  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
  {
    matrix3[i] = (type2) (matrix1[i] - number);
  }
}


template<class type1, class type2>
void function_image_multiply(v3dMatrix<type1> * the_image1, double number,
		     v3dMatrix<type2> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
  {
    matrix3[i] = (type2) (matrix1[i] * number);
  }
}


template<class type1, class type2>
void function_image_divide(v3dMatrix<type1> * the_image1, double number,
		   v3dMatrix<type2> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
  {
    matrix3[i] = (type2) (matrix1[i] / number);
  }
}


template<class type1>
v3dMatrix<type1> image_square_root(v3dMatrix<type1> * the_image1)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  v3dMatrix<type1> * result = new v3dMatrix<type1>(rows, cols, bands);

  image_square_root(the_image1, result);
  return result;
}


template<class type1, class type2>
void image_square_root(v3dMatrix<type1> * the_image1, v3dMatrix<type2> * result)
{
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix2 = result->Matrix();

  vint8 i;
  vint8 size = the_image1->AllBandSize();
  for (i = 0; i < size; i++)
    matrix2[i] = (type1) sqrt(matrix1[i]);
}


template<class type1, class type2>
void function_image_absolute(v3dMatrix<type1> * the_image1, v3dMatrix<type2> * result)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();
  assert(rows == result->Rows());
  assert(cols == result->Cols());
  assert(bands == result->Bands());
  
  vArray(type1) matrix1 = the_image1->Matrix();
  vArray(type2) matrix3 = result->Matrix();
  vint8 size = the_image1->AllBandSize();
  vint8 i;
  for (i = 0; i < size; i++)
  {
    type1 value = matrix1[i];
    if (value >= 0) matrix3[i] = (type2) value;
    else matrix3[i] = (type2) -value;
  }
}


template<class type>
v3dMatrix<type> * function_image_absolute(v3dMatrix<type> * the_image1)
{
  vint8 rows = the_image1->Rows();
  vint8 cols = the_image1->Cols();
  vint8 bands = the_image1->Bands();

  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  function_image_absolute (the_image1, result);
  return result;
}


template <class type>
double_matrix make_cumulative_histogram(vMatrix<type> histogram)
{
  vint8 size = histogram.Size();
  double_matrix result(1 , size);
  result (0) = histogram (0);

  vint8 counter;
  for (counter = 1; counter < size; counter++)
  {
    result(counter) = result(counter - 1) + (double) histogram (counter);
  }

  return result;
}


template <class type>
double_matrix make_histogram (std::vector <type> * scores, double histogram_minimum,
                            double histogram_maximum, long bucket_number)
{
//  float range = histogram_maximum - histogram_minimum;
//  float bucket_size = range / (float) bucket_number;

  double_matrix result(1, bucket_number);
  function_enter_value(& result, (double) 0);

  long counter;
  long number = scores->size();
  for (counter = 0; counter < number; counter++)
  {
    double entry =(*scores)[counter];
 //   float normalized = (entry - histogram_minimum) / bucket_size;
 //   long index = round_number(floor(normalized));
    long index = function_histogram_bucket(entry, histogram_minimum, 
                                           histogram_maximum, bucket_number);
    if (result.check_bounds(0, index))
    {
      result(index) = result(index) + 1.0;
    }
    else
    {
      exit_error("\nerror: unexpected entry %lf in make_histogram\n", (double) entry);
    }
  }

  return result;
}


template <class type>
vint8_matrix make_histogram(v3dMatrix<type> scores, double histogram_minimum,
                           double histogram_maximum, long bucket_number)
{
//  float range = histogram_maximum - histogram_minimum;
//  float bucket_size = range / (float) bucket_number;

  double_matrix result(1, bucket_number);
  function_enter_value(& result, (double) 0);

  long counter;
  long number = scores->size();
  for (counter = 0; counter < number; counter++)
  {
    double entry = scores(counter);
    long index = function_histogram_bucket(entry, histogram_minimum, 
                                           histogram_maximum, bucket_number);
    if (result.check_bounds(0, index))
    {
      result(index) = result(index) + 1.0;
    }
    else
    {
      exit_error("\nerror: unexpected entry %lf in make_histogram\n", (double) entry);
    }
  }

  return result;
}


template <class type>
double_matrix normalize_histogram(vMatrix<type> histogram)
{
  double_matrix converted(&histogram);
  double total = function_image_total(& converted);
  if (total <= 0)
  {
    return converted;
  }

  double_matrix result = converted / total;
  return result;
}



template <class type>
double_matrix make_normalized_histogram(std::vector <type> * scores, double histogram_minimum,
                                        double histogram_maximum, long bucket_number)
{
  double_matrix histogram = make_histogram(scores, histogram_minimum, histogram_maximum, bucket_number);
  double_matrix result = normalize_histogram(histogram);
  return result;
}


template <class type>
long function_extract_values(vMatrix<type> image, std::vector<class_pixel> * locations,
                             std::vector<type> * result)
{
  long number = locations->size();
  long counter;
  for (counter = 0; counter < number; counter++)
  {
    class_pixel pixel =(*locations)[counter];
    if(image.check_bounds(pixel.vertical, pixel.horizontal))
    {
      type value = image(pixel.vertical, pixel.horizontal);
      result->push_back(value);
    }
  }

  return 1;
}


template <class type>
vint8_matrix maximizing_channels(v3dMatrix<type> * image)
{
  vint8 horizontal_size = image->horizontal ();
  vint8 vertical_size = image->vertical ();
  vint8 channels = image->channels ();

  if (channels == 0)
  {
    return vint8_matrix ();
  }
  
  vint8_matrix result (vertical_size, horizontal_size);
  long vertical, horizontal, channel;
  for (vertical = 0; vertical < vertical_size; vertical ++)
  {
    for (horizontal = 0; horizontal < horizontal_size; horizontal ++)
    {
      type maximum = (*image) (0, vertical, horizontal);
      long maximum_index = 0;
      for (channel = 1; channel < channels; channel ++)
      {
        type current = (*image) (channel, vertical, horizontal);
        if (current > maximum)
        {
          maximum = current;
          maximum_index = channel;
        }
      }
      result (vertical, horizontal) = maximum_index;
    }
  }

  return result;
}


template <class type>
vMatrix<type> maximal_image (v3dMatrix<type> * image)
{
  vint8 horizontal_size = image->horizontal ();
  vint8 vertical_size = image->vertical ();
  vint8 channels = image->channels ();

  if (channels == 0)
  {
    return vMatrix<type>();
  }
  
  vMatrix<type> result (vertical_size, horizontal_size);
  long vertical, horizontal, channel;
  for (vertical = 0; vertical < vertical_size; vertical ++)
  {
    for (horizontal = 0; horizontal < horizontal_size; horizontal ++)
    {
      type maximum = (*image) (0, vertical, horizontal);
      for (channel = 1; channel < channels; channel ++)
      {
        type current = (*image) (channel, vertical, horizontal);
        if (current > maximum)
        {
          maximum = current;
        }
      }
      result (vertical, horizontal) = maximum;
    }
  }

  return result;
}


template <class type>
vMatrix<type> matrix_from_vector(const vector<type> * data_pointer, const long vertical, const long horizontal)
{
  vector<type> & data = *data_pointer;
  long size = vertical * horizontal;
  if (size != data.size())
  {
    exit_error("error: incompatible sizes in matrix_from_vector\n");
  }

  vMatrix<type> result(vertical, horizontal);
  class_pointer(type) entries = result.flat();

  long counter;
  for (counter = 0; counter < size; counter++)
  {
    entries[counter] = data[counter];
  }

  return result;
}


// in this function, for each pixel, the value in each band is treated separately.
// we return a binary image with a zero entry for 3D locations 
// whose entries in the original image are not among the smallest.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
uchar_image find_smallest_entries(v3dMatrix<type> * image, long rank)
{
  type threshold = find_rank_ascending(rank, image, 0);
  
  vint8 vertical = image->vertical();
  vint8 horizontal = image->horizontal();
  vint8 channels = image->channels();
  uchar_image result(vertical, horizontal, channels);
  
  class_pointer(type) source = image->flat();
  class_pointer(uchar) target = result.flat();
  vint8 size = result.all_channel_size();

  vector_size counter;
  for (counter = 0; counter < size; counter++)
  {
    type value = source[counter];
    if (value <= threshold)
    {
      target[counter] = (uchar) 255;
    }
    else
    {
      target[counter] = (uchar) 0;
    }
  }

  return result;
}


// in this function, for each pixel, the value in each band is treated separately.
// we return an image with an entry=replacement for 3D locations 
// whose entries in the original image are not among the smallest.
// the other entries are equal to the entries of the corresponding locations
// in the original image.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
v3dMatrix<type> keep_smallest_entries(v3dMatrix<type> * image, long rank, type replacement)
{
  type threshold = find_rank_ascending(rank, image, 0);
  
  vint8 vertical = image->vertical();
  vint8 horizontal = image->horizontal();
  vint8 channels = image->channels();
  v3dMatrix<type>  result(vertical, horizontal, channels);
  
  class_pointer(type) source = image->flat();
  class_pointer(type) target = result.flat();
  vint8 size = result.all_channel_size();

  vector_size counter;
  for (counter = 0; counter < size; counter++)
  {
    type value = source[counter];
    if (value <= threshold)
    {
      target[counter] = value;
    }
    else
    {
      target[counter] = replacement;
    }
  }

  return result;
}


// in this function, for each pixel, the value in each band is treated separately.
// we return a binary image with a zero entry for 3D locations 
// whose entries in the original image are not among the largest.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
uchar_image find_largest_entries(v3dMatrix<type> * image, long rank)
{
  type threshold = find_rank_descending(rank, image, 0);
  
  vint8 vertical = image->vertical();
  vint8 horizontal = image->horizontal();
  vint8 channels = image->channels();
  uchar_image result(vertical, horizontal, channels);
  
  class_pointer(type) source = image->flat();
  class_pointer(uchar) target = result.flat();
  vint8 size = result.all_channel_size();

  vector_size counter;
  for (counter = 0; counter < size; counter++)
  {
    type value = source[counter];
    if (value >= threshold)
    {
      target[counter] = (uchar) 255;
    }
    else
    {
      target[counter] = (uchar) 0;
    }
  }

  return result;
}


// in this function, for each pixel, the value in each band is treated separately.
// we return an image with an entry=replacement for 3D locations 
// whose entries in the original image are not among the largest.
// the other entries are equal to the entries of the corresponding locations
// in the original image.
// the result should have about rank pixels nonzero, and the only exceptions
// should occur in cases of ties.
template <class type>
v3dMatrix<type> keep_largest_entries(v3dMatrix<type> * image, long rank, type replacement)
{
  type threshold = find_rank_descending(rank, image, 0);
  
  vint8 vertical = image->vertical();
  vint8 horizontal = image->horizontal();
  vint8 channels = image->channels();
  v3dMatrix<type> result(vertical, horizontal, channels);
  
  class_pointer(type) source = image->flat();
  class_pointer(type) target = result.flat();
  vint8 size = result.all_channel_size();

  vector_size counter;
  for (counter = 0; counter < size; counter++)
  {
    type value = source[counter];
    if (value >= threshold)
    {
      target[counter] = value;
    }
    else
    {
      target[counter] = replacement;
    }
  }

  return result;
}


template<class type> type find_rank_descending(vint8 k, v3dMatrix<type> * elements, 
                                         vint8 * indexp)
{
  vint8 size = elements->AllBandSize();
  vArray(type) entries = elements->Matrix();

  return find_rank_descending(k, entries, size, indexp);
}


template<class type> type find_rank_descending(vint8 k, vArray(type) entries, 
                                        vint8 size, vint8 * indexp)
{
  vMatrix<type> scratch_matrix(4, size);
  vArray(type) scratch1 = scratch_matrix.Matrix2()[0];
  vArray(type) scratch2 = scratch_matrix.Matrix2()[1];
  vArray(type) scratch3 = scratch_matrix.Matrix2()[2];
  vArray(type) scratch4 = scratch_matrix.Matrix2()[3];

  vint8 i;
  for (i = 0; i < size; i++)
  {
    scratch1[i] = entries[i];
  }
  vint8 in_size = size;
  long large_found = 0;

  vArray(type) in = scratch1;
  vArray(type) out1 = scratch2;
  vArray(type) out2 = scratch3;
  vArray(type) out3 = scratch4;
  long index1, index2, index3;
  type pivot;

  while(large_found < k)
  {
    vint8 pivot_index = function_random_vint8(0, in_size-1);
    pivot = in[pivot_index];

    index1 = 0;
    index2 = 0;
    index3 = 0;
    vint8 i;
    for (i = 0; i < in_size; i++)
    {
      type number = in[i];
      if (number > pivot)
      {
        out1[index1++] = number;
      }
      else if (number == pivot)
      {
        out3[index3++] = number;
      }
      else
      {
        out2[index2++] = number;
      }
    }

    // Check for the pathological case where all elements are 
    // equal to the pivot.
    if (index3 == in_size)
    {
      break;
    }

    if (large_found + index1 + index3 <= k)
    {
      large_found = large_found + index1 + index3;
      vArray(type) temp = in;
      in = out2;
      out2 = temp;
      in_size = index2;
    }
    else if (large_found + index1 <= k)
    {
      large_found = large_found + index1;
      for (int i = 0; i < index3; i++)
      {
        out2[i + index2] = out3[i];
      }

      vArray(type) temp = in;
      in = out2;
      out2 = temp;
      
      in_size = index2 + index3;
    }
    else
    {
      vArray(type) temp = in;
      in = out1;
      out1 = temp;
      in_size = index1;
    }
  }

  type result = pivot;

  if (indexp != 0)
  {
    *indexp = -1;
    for (i = 0; i < size; i++)
    {
      if (entries[i] == result)
      {
        *indexp = i;
        break;
      }
    }

    if (*indexp == -1)
    {
      exit_error("Error: Bug in kth_smallest_da\n");
    }
  }

  return result;
}


template<class type> long is_in_matrix(const type value, const v3dMatrix<type> * matrix)
{
  long size = matrix->all_channel_size();
  const class_pointer(type) entries = matrix->flat();
  long counter; 
  for (counter = 0; counter < size; counter++)
  {
    if (value == entries[counter])
    {
      return 1;
    }
  }

  return 0;
}


// if entry is in elements, we return a copy of elements,
// otherwise we return a matrix including all elements and entry
template<class type> vMatrix<type> insert_entry(const vMatrix<type> & elements, 
                                                type entry)
{
  if (is_in_matrix(entry, & elements))
  {
    return vMatrix<type>(&elements);
  }

  long length = elements.length();
  vMatrix<type> result(1, length +1);
  
  long counter;
  for (counter = 0; counter < length; counter++)
  {
    result(counter) = elements(counter);
  }

  result(length) = entry;
  return result;
}


// if entry is not in elements, we return a copy of elements,
// otherwise we return a matrix including all elements minus entry
template<class type> vMatrix<type> remove_entry(const vMatrix<type> & elements, 
                                                type entry)
{
  if (!is_in_matrix(entry, & elements))
  {
    return vMatrix<type>(&elements);
  }

  long length = elements.length();
  long total = 0;  
  long counter;
  for (counter = 0; counter < length; counter++)
  {
    if (elements(counter) == entry)
    {
      total++;
    }
  }

  if (total == length)
  {
    return vMatrix<type>();
  }
  vMatrix<type> result(1, length - total);

  long index = 0;
  for (counter = 0; counter < length; counter++)
  {
    if (elements(counter) == entry)
    {
      continue;
    }
    result(index) = elements(counter);
    index++;
  }

  return result;
}








// If the cpp template file is included as part of the header file,
// we must undefine certain things.
#include "basics/undefine.h"

#endif // VASSILIS_PROCESS_FILE
