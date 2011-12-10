
#include "vplatform.h"
#ifdef VASSILIS_SGI_PLATFORM
#include <iostream>
#else 
#include <iostream>
#endif
#include <math.h>

#include "basics/algebra.h"
#include "basics/drawing_temp.h"
#include "basics/precomputed.h"
#include "basics/simple_algo.h"

#include "basics/definitions.h" 
// This is an efficient version of vPickWithoutRepetitions from
// auxiliaries.cpp
vMatrix<vint8> sample_without_replacement(vint8 low, vint8 high, vint8 number)
{
  vint8 range = high - low + 1;
  if (number >= range)
  {
    return vMatrix<vint8>::Range(low, high);
  }

  vMatrix<vint8> scratch_matrix(1, range);
  vArray(vint8) scratch = scratch_matrix.Matrix();
  // set bad_value into a value that scratch_matrix will
  // not receive except at initialization.
  vint8 bad_value = number;

  // If number > range/2, it is more efficient to pick the numbers
  // that we will not include.
  vint8 number_to_pick;
  function_enter_value(&scratch_matrix, bad_value);
  
  if (number > range/2)
  {
    number_to_pick = range - number;
  }
  else
  {
    number_to_pick = number;
  }

  vMatrix<vint8> result_matrix(1, bad_value);
  vArray(vint8) result = result_matrix.Matrix();
  vint8 i;
  for (i = 0; i < number_to_pick; i++)
  {
    // pick a random number in the given range
    vint8 pick = function_random_vint8(low, high);
    // check if that number has been picked. If it has been picked before,
    // then scratch[pick - low] will hold the value of i for which it
    // has been picked, and result will hold pick at that value.
    vint8 index = scratch[pick - low];
    if (((vint8) index < i) &&
        (result[index] == pick))
    {
      i--;
      continue;
    }

    // if the number has not been picked before, store it in result, 
    // and also mark it in scratch with i (the order in which it has been
    // picked).
    result[i] = pick;
    scratch[pick - low] = i;
  }

  if (number == number_to_pick) 
  {
    return result_matrix;
  }
  else // In this case we return the complement of what we have picked.
  {
    vint8 counter = 0;
    for (i = low; i <= high; i++)
    {
      if (scratch[i-low] == number)
      {
        result[counter] = i;
        counter++;
      }
    }
    
    if (counter != number)
    {
      exit_error("Error: bug in sample_without_replacement, counter = %li, number = %li\n",
                      counter, number);
    }
  }

  return result_matrix;
}


// It is assumed that matrix has integer values between start and end. 
// We return a matrix consisting of all integer values between 
// start and end that do not appear in the argument matrix.
vMatrix<vint8> vComplement(v3dMatrix<vint8> * matrix, vint8 start, vint8 end)
{
  vint8 range = end - start + 1;
  Bitmap bitmap(range);
  vint8 size = matrix->Size();
  vArray(vint8) data = matrix->Matrix();
  vint8 counter = 0;

  vint8 i;
  for (i = 0; i < size; i++)
  {
    vint8 entry = data[i];
    if ((entry < start) || (entry > end))
    {
      exit_error("Error: entry = %li, start = %li, end = %li\n",
                      entry, start, end);
    }
    vint8 position = entry - start;
    vint8 value = bitmap.Read(position);
    if (value == 0)
    {
      bitmap.Write(position, 1);
      counter++;
    }
  }

  if (counter == range)
  {
    exit_error("Error: complement is empty, vMatrix can't handle that\n");
  }

  vint8 complement_size = range - counter;
  vMatrix<vint8> result(1, complement_size);
  vArray(vint8) result_data = result.Matrix();
  counter = 0;
  for (i = start; i <= end; i++)
  {
    vint8 position = i - start;
    vint8 value = bitmap.Read(position);
    if (value == 0)
    {
      result_data[counter] = i;
      counter++;
    }
  }

  return result;
}


// Returns a matrix containing all integers from low to high, in random 
// order, i.e. a random permutation of the 
// tuple (low, low+1, ..., high-1, high). We assign to each number
// from low to high a random float attribute, and we sort on that
// attribute. Naturally the complexity is n log n, where n = high - low + 1.
// I don't know if it can be done faster.
vMatrix<vint8> vPermutation(vint8 low, vint8 high)
{
  vint8 range = high - low + 1;
  vector<class_couple> objects((vector_size) range);
  vint8 i;
  // Assign to each number a random float attribute, based on 
  // which we can sort the numbers and get a random order.
  for (i = 0; i < range; i++)
  {
    vint8 object = i + low;
    vint8 random_vint8 = function_random_vint8();
    float random_float = (float) random_vint8;
    objects[(vector_size) i] = class_couple(random_float, (void *) (long) object);
  }

  std::sort(objects.begin(), objects.end(), couple_less());
  vMatrix<vint8> result(1, range);
  for (i = 0; i < range; i++)
  {
    vint8 number = (vint8) (long) (objects[(vector_size) i].object);
    result(i) = number;
  }

  return result;
}



//! returns a matrix with entries low, low+1, ..., high.
vMatrix<vint8> function_range_matrix(vint8 low, vint8 high)
{
  if (high < low)
  {
    exit_error("error: function_range_matrix low > high indices\n");
  }

  vint8 range = high - low + 1;
  vMatrix<vint8> result(1, range);
  vint8 i;
  for (i = low; i <= high; i++)
  {
    result(i - low) = i;
  }

  return result;
}


integer_matrix integer_range_matrix(integer low, integer high)
{
  if (high < low)
  {
    exit_error("error: asdfjasldkrjfsjrfwe;lr\n");
  }

  integer range = high - low + 1;
  vMatrix<integer> result(1, range);
  integer i;
  for (i = low; i <= high; i++)
  {
    result(i - low) = i;
  }

  return result;
}


// returns a matrix with entres low, low+1, ..., high.
vMatrix<float> function_range_matrix(float low, float step, float high)
{
  if (high < low)
  {
    exit_error("error: asdfjasldkrjfsjrfwe;lr\n");
  }

  vint8 range = round_number((high - low) / step + 1);
  vMatrix<float> result(1, range);

  vint8 index = 0;
  float value;
  for (value = low; value <= high; value = value + step)
  {
    if (index >= range)
    {
      exit_error("error: index > range\n");
    }
    result(index) = value;
    index = index + 1;
  }
  if (index != range)
  {
    exit_error("error: index != range\n");
  }

  return result;
}


// eventually this function should go to algo_templates or general_templates.
// result[i] = j if values[j] is the i-th smallest entry in values.
vint8 function_string_ranks(vector<char *> * values, vector<vint8> * result)
{
  vint8 number = values->size();
  if (number == 0)
  {
    return 0;
  }

  // create a vector that holds value-index pairs, so that we 
  // can sort based on value, but still know the associated index.
  vector<class_string_couple> pairs((vector_size) number);
  vint8 i;

  for (i = 0; i < number; i++)
  {
    pairs[(vector_size) i] = class_string_couple((*values)[(vector_size) i], (void *) (long) i);
  }
  std::sort(pairs.begin(), pairs.end(), string_couple_less());

  result->erase(result->begin(), result->end());
  result->reserve((vector_size) number);
  result->insert(result->begin(), (vector_size) number, 0);

  for (i = 0; i < number; i++)
  {
    vint8 index = (vint8) (long) (pairs[(vector_size) i].object);
    (*result)[(vector_size) i] = index;
  }

  return 1;
}




// counts the number of pixels that are positive.  If the image has multiple 
// bands, then for each pixel we consider the some over all bands.
vint8 count_positive_pixels(general_image* image)
{
  vint8 channels = image->channels ();
  if (channels == 1)
  {
    return count_positive_entries (image);
  }
  
  double_image converted (image);
  matrix_pointer (double) entries = converted.Matrix2();
  vint8 size = converted.Size ();
  vint8 result = 0;  

  vint8 counter, channel;
  for (counter = 0; counter < size; counter++)
  {
    double total = 0.0;
    for (channel = 0; channel < channels; channel++)
    {
      total += entries [channel] [counter];
    }
    if (total > 0)
    {
      result ++;
    }
  }

  return result;
}


// counts the number of entries that are positive.  If the image has multiple
// bands, then the entry at each band is considered to be a separate entry.
vint8 count_positive_entries(general_image* image)
{
  double_image converted (image);
  class_pointer (double) entries = converted.Matrix();
  vint8 size = converted.AllBandSize();
  vint8 result = 0;  

  vint8 counter;
  for (counter = 0; counter < size; counter++)
  {
    if (entries [counter] > 0)
    {

      result ++;
    }
  }

  return result;
}


// the entry of each band is considered to be a separate entry.
uchar_image* threshold_entries (general_image* image, double threshold)
{
  double_image converted (image);
  vint8 vertical = converted.vertical ();
  vint8 horizontal = converted.horizontal ();
  vint8 channels = converted.channels ();

  uchar_image* result_image = new uchar_image (vertical, horizontal, channels);
  class_pointer (uchar) result = result_image->Matrix ();
  class_pointer (double) entries = converted.Matrix();
  vint8 size = converted.AllBandSize();

  vint8 counter;
  for (counter = 0; counter < size; counter++)
  {
    if (entries [counter] >= threshold)
    {
      result[counter] = 1;
    }
    else
    {
      result [counter] = 0;
    }
  }

  return result_image;
}


// the entry of each band is considered to be a separate entry.
uchar_image* inverse_threshold_entries (general_image* image, double threshold)
{
  double_image converted (image);
  vint8 vertical = converted.vertical ();
  vint8 horizontal = converted.horizontal ();
  vint8 channels = converted.channels ();

  uchar_image* result_image = new uchar_image (vertical, horizontal, channels);
  class_pointer (uchar) result = result_image->Matrix ();
  class_pointer (double) entries = converted.Matrix();
  vint8 size = converted.AllBandSize();

  vint8 counter;
  for (counter = 0; counter < size; counter++)
  {
    if (entries [counter] <= threshold)
    {
      result[counter] = 1;
    }
  }

  return result_image;
}


vint8 image_dimensions_equal(general_image* first_image, general_image* second_image)
{
  if ((first_image == 0) || (second_image == 0))
  {
    return 0;
  }

  if ((first_image->vertical() != second_image->vertical()) || 
      (first_image->horizontal() != second_image->horizontal()) || 
      (first_image->channels() != second_image->channels()))
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


vMatrix<double> v2x2Eigenvectors(v3dMatrix<double> * input)
{
  assert(input->Rows() == 2);
  assert(input->Cols() == 2);
  assert(input->Bands() == 1);
  vMatrix<double> result(2,2);
  vArray2(double) result2 = result.Matrix2();
  vArray2(double) input2 = input->Matrix2(0);
  double a = input2[0][0];
  double b = input2[0][1];
  double c = input2[1][0];
  double d = input2[1][1];

  if (c != 0)
  {
    double two_c = 2*c;
    double sum1 = a*a + 4*b*c - 2*a*d + d*d;
    double root1 = sqrt(sum1);
    double d_minus_a = d - a;
    result2[0][0] = -(d_minus_a + root1)/two_c;
    result2[1][0] = 1;
    result2[0][1] = -(d_minus_a - root1)/two_c;
    result2[1][1] = 1;
  }

  else if (a != d)
  {
    result2[0][0] = 1;
    result2[1][0] = 0;
    result2[0][1] = b/(d-a);
    result2[1][1] = 1;
  }

  else 
  {
    result2[0][0] = 1;
    result2[1][0] = 0;
    result2[0][1] = 0;
    result2[1][1] = 0;
  }

  return result;
}


// An interactive function to test v2x2Eigenvectors.
vint8 vTest2x2Eigenvectors()
{
  while(1)
  {
    double a, b, d;
    a = b = d = 0;
    vPrint("Input a, b, d: ");
    vScan("%lf %lf %lf", &a, &b, &d);
    if (a == -1973) break;
    vMatrix<double> input(2,2);
    input(0,0) = a;
    input(0,1) = b;
    input(1,0) = b;
    input(1,1) = d;

    vMatrix<double> eigenvectors = v2x2Eigenvectors(&input);
    vPrint("Eigenvectors: ");
    eigenvectors.Print();
    vPrint("\n");
  }
  return 1;
}


float_matrix convert_simple_float(simple_float_matrix * simple_image)
{
  long vertical_size = simple_image->vertical_size;
  long horizontal_size = simple_image->horizontal_size;
  float_matrix result(vertical_size, horizontal_size);

  // in principle we could just copy the 1D array simple_image->data
  // does the 1D array result.matrix. the reason I'm doing it
  // in this slightly more inefficient way is that I want to use this function
  // to test simple_float_matrix::get index
  long vertical, horizontal;
  for (vertical = 0; vertical < vertical_size; vertical++)
  {
    for (horizontal = 0; horizontal < horizontal_size; horizontal++)
    {
      long index = simple_image->get_index(vertical, horizontal);
      result(vertical, horizontal) = simple_image->data[index];
    }
  }

  return result;
}

