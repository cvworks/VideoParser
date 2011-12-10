
#include "vplatform.h"

#include <math.h>

#include "precomputed.h"

#include "pc_aux.h"

#include "matrix.h"
#include "drawing.h"

#include "definitions.h"

// Calls the appropriate functions in each class so that values get 
// precomputed and we don't have to initialize each function separately
long vPrecomputeValues()
{
  vPrecomputedCos::PrecomputeValues();
  vPrecomputedSin::PrecomputeValues();
  vPrecomputedTan::PrecomputeValues();
  vPrecomputedAcos::PrecomputeValues();
  
  vPrecomputedLengths::PrecomputeValues(256, 256);
  vPrecomputedSqrt::PrecomputeValues(140000);
  vPrecomputedLogs::PrecomputeValues(140000);
  vPrecomputedAngles::PrecomputeValues(256, 256);
  vPrecomputedAngles2::PrecomputeValues(256, 256);
  vPrecomputedExp::PrecomputeValues((float) 10.0);

  // We don't precompute values for PrecomputedLineCols
  return 1;
}


// Deletes any dynamically allocated memory used to store precomputed
// values. We should call this function before the end of the program,
// so that this memory will not show up as memory leaks.
long vDeletePrecomputedValues()
{
  vPrecomputedLengths::CleanUp();
  vPrecomputedSqrt::CleanUp();
  vPrecomputedLogs::CleanUp();
  vPrecomputedAngles::CleanUp();
  vPrecomputedAngles2::CleanUp();
  precomputed_line_columns::CleanUp();
  vPrecomputedTriangles::CleanUp();
  vPrecomputedSqrtf::CleanUp();

  return 1;
}


float vPrecomputedCos::values[23000];
double vPrecomputedCos::two_pi = 2*M_PI;
double vPrecomputedCos::four_pi = 4*M_PI;
double vPrecomputedCos::number = 20000;
double vPrecomputedCos::number2 = 23000;
double vPrecomputedCos::factor = PrecomputeValues();


double vPrecomputedCos::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  two_pi = 2 * M_PI;
  four_pi = 4 * M_PI;
  number = 20000;
  number2 = 23000;
  factor = number/ four_pi;

  long i;
  for (i = 0; i < number2; i++)
  {
    double rad = ((double) i) / factor;
    values[i] = (float) cos(rad);
  }
  return factor;
}


float vPrecomputedCosf::buffer[46000];
float * vPrecomputedCosf::values = &(vPrecomputedCosf::buffer[23000]);
float vPrecomputedCosf::two_pi = 2*M_PIf;
float vPrecomputedCosf::four_pi = 4*M_PIf;
float vPrecomputedCosf::number = 20000;
float vPrecomputedCosf::number2 = 23000;
float vPrecomputedCosf::factor = PrecomputeValues();


float vPrecomputedCosf::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  number = 20000;
  number2 = 23000;
  factor = number/ four_pi;

  long i;
  for (i = (long) -number2; i < (long) number2; i++)
  {
    float rad = ((float) i) / factor;
    values[i] = (float) cos(rad);
  }
  return factor;
}


float vPrecomputedSinf::buffer[46000];
float * vPrecomputedSinf::values = &(vPrecomputedSinf::buffer[23000]);
float vPrecomputedSinf::two_pi = 2*M_PIf;
float vPrecomputedSinf::four_pi = 4*M_PIf;
float vPrecomputedSinf::number = 20000;
float vPrecomputedSinf::number2 = 23000;
float vPrecomputedSinf::factor = PrecomputeValues();


float vPrecomputedSinf::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  number = 20000;
  number2 = 23000;
  factor = number/ four_pi;

  long i;
  for (i = (long) -number2; i < (long) number2; i++)
  {
    float rad = ((float) i) / factor;
    values[i] = (float) sin(rad);
  }
  return factor;
}


float vPrecomputedSin::values[23000];
double vPrecomputedSin::two_pi = 2*M_PI;
double vPrecomputedSin::four_pi = 4*M_PI;
double vPrecomputedSin::number = 20000;
double vPrecomputedSin::number2 = 23000;
double vPrecomputedSin::factor = PrecomputeValues();


double vPrecomputedSin::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  two_pi = 2 * M_PI;
  four_pi = 4 * M_PI;
  number = 20000;
  number2 = 23000;
  factor = number/ four_pi;

  long i;
  for (i = 0; i < number2; i++)
  {
    double rad = ((double) i) / factor;
    values[i] = (float) sin(rad);
  }
  return factor;
}


float vPrecomputedTan::values[23000];
double vPrecomputedTan::two_pi = 2*M_PI;
double vPrecomputedTan::four_pi = 4*M_PI;
double vPrecomputedTan::number = 20000;
double vPrecomputedTan::number2 = 23000;
double vPrecomputedTan::factor = PrecomputeValues();


double vPrecomputedTan::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  two_pi = 2 * M_PI;
  four_pi = 4 * M_PI;
  number = 20000;
  number2 = 23000;
  factor = number/ four_pi;

  long i;
  for (i = 0; i < number2; i++)
  {
    double rad = ((double) i) / factor;
    values[i] = (float) tan(rad);
  }
  return factor;
}


float vPrecomputedAcos::values[23000];
double vPrecomputedAcos::number = 20000;
double vPrecomputedAcos::number2 = 23000;
double vPrecomputedAcos::factor = PrecomputeValues();


double vPrecomputedAcos::PrecomputeValues()
{
  static long called_before = 0;
  if (called_before != 0) return factor;
  called_before = 1;

  number = 20000;
  number2 = 23000;
  factor = number/2;

  long i;
  for (i = 0; i < number2; i++)
  {
    double cosine = ((double) i) / factor - 1;
    if (cosine < -1) cosine = -1;
    if (cosine > 1) cosine = 1;
    values[i] = (float) acos(cosine);
  }
  return factor;
}


vArray2(long) vPrecomputedLengths::values = vZero(vArray(long));
vArray2(float) vPrecomputedLengths::valuesf = vZero(vArray(float));
long vPrecomputedLengths::rows = 0;
long vPrecomputedLengths::cols = 0; 
long vPrecomputedLengths::low_row = 0;
long vPrecomputedLengths::high_row = -1;
long vPrecomputedLengths::low_col = 0;
long vPrecomputedLengths::high_col = -1;


long vPrecomputedLengths::PrecomputeValues(long in_rows, long in_cols)
{
  if ((low_row <= 0) && (high_row >= in_rows-1) &&
      (low_col <= 0) && (high_col >= in_cols-1))
  {
    return 1;
  }

  if (in_rows < 0) return 0;
  if (in_cols < 0) return 0;
  vDelete2(values, rows);
  vDelete2(valuesf, rows);
  low_row = 0;
  high_row = in_rows-1;
  rows = in_rows;

  low_col = 0;
  high_col = in_cols-1;
  cols = in_cols;

  vHyperArray2(&values, rows, cols);
  vHyperArray2(&valuesf, rows, cols);

  long row, col;
  for (row = low_row; row <= high_row; row++)
  {
    for (col = low_col; col <= high_col; col++)
    {
      float square_length = (float) (row*row + col*col);
      float length = sqrt(square_length);
      values[row][col] = (long) round_number(length);
      valuesf[row][col] = length;
    }
  }

  return 1;
}


static inline double S_Angle(double x, double y)
{
  double result;
  if (x == 0)
  {
    if (y != 0) result = M_PI / 2;
    else result = 0;
  }
  else
  {
    double tangent = y / x;
    result = atan(tangent);
    // Make sure the result is not negative
    if (result < 0) result = result + M_PI;
  }
  return result;
}



vArray2(float) vPrecomputedAngles::values = vZero(vArray(float));
long vPrecomputedAngles::rows = 0;
long vPrecomputedAngles::cols = 0; 
long vPrecomputedAngles::low_row = 0;
long vPrecomputedAngles::high_row = -1;
long vPrecomputedAngles::low_col = 0;
long vPrecomputedAngles::high_col = -1;


long vPrecomputedAngles::PrecomputeValues(long in_rows, long in_cols)
{
  if ((in_rows != 256) || (in_cols != 256))
  {
    exit_error("Error: The code right now can't handle these rows and cols\n");
  }
  if ((low_row <= -in_rows) && (high_row >= in_rows-1) &&
      (low_col <= -in_cols) && (high_col >= in_cols-1))
  {
    return 1;
  }

  if (in_rows < 0) return 0;
  if (in_cols < 0) return 0;
  vDelete2(values, 2*rows);
  low_row = -in_rows;
  high_row = in_rows-1;
  rows = in_rows;

  low_col = -in_cols+1;
  high_col = in_cols-1;
  cols = in_cols;

  vHyperArray2(&values, 2*rows, 2*cols);

  long row, col;
  for (row = low_row; row <= high_row; row++)
  {
    for (col = low_col; col <= high_col; col++)
    {
      values[row+rows][col+cols] = (float) S_Angle(col, row);
    }
  }

  return 1;
}


vArray2(float) vPrecomputedAngles2::values = vZero(vArray(float));
long vPrecomputedAngles2::rows = 0;
long vPrecomputedAngles2::cols = 0; 
long vPrecomputedAngles2::low_row = 0;
long vPrecomputedAngles2::high_row = -1;
long vPrecomputedAngles2::low_col = 0;
long vPrecomputedAngles2::high_col = -1;


long vPrecomputedAngles2::PrecomputeValues(long in_rows, long in_cols)
{
//  if ((in_rows != 256) || (in_cols != 256))
//  {
//    exit_error("Error: The code right now can't handle these rows and cols\n");
//  }
  if ((low_row <= -in_rows) && (high_row >= in_rows-1) &&
      (low_col <= -in_cols) && (high_col >= in_cols-1))
  {
    return 1;
  }

  if (in_rows < 0) return 0;
  if (in_cols < 0) return 0;
  vDelete2(values, 2*rows);
  low_row = -in_rows;
  high_row = in_rows-1;
  rows = in_rows;

  low_col = -in_cols;
  high_col = in_cols-1;
  cols = in_cols;

  vHyperArray2(&values, 2*rows, 2*cols);

  long row, col;
  for (row = low_row; row <= high_row; row++)
  {
    for (col = low_col; col <= high_col; col++)
    {
      values[row+rows][col+cols] = (float) vAngle2(col, row);
    }
  }

  return 1;
}


vArray(long) vPrecomputedSqrt::values = vZero(long);
long vPrecomputedSqrt::limit = -1;


long vPrecomputedSqrt::PrecomputeValues(long in_limit)
{
  if (in_limit <= limit) return 1;
  vdelete2(values);
  limit = in_limit;
  values = vnew(long, limit+1);
  long i;
  for (i = 0; i <= limit; i++)
  {
    values[i] = (long) round_number(sqrt((double) i));
  }

  return 1;
}


vArray(double) vPrecomputedLogs::values = vZero(double);
long vPrecomputedLogs::limit = -1;


long vPrecomputedLogs::PrecomputeValues(long in_limit)
{
  static const double log2 = log((double) 2);
  if (in_limit <= limit) return 1;
  vdelete2(values);
  limit = in_limit;
  values = vnew(double, limit+1);
  long i;
  // log[0] is undefined.
  values[0] = 0;
  for (i = 1; i <= limit; i++)
  {
    values[i] = log((double) i) / log2;
  }

  return 1;
}


long vPrecomputedBitSums::values[65536];


long vPrecomputedBitSums::PrecomputeValues()
{
  long i;
  for (i = 0; i < 65536; i++)
  {
    long counter = 0;
    long j = i;
    while(j > 0)
    {
      counter += j % 2;
      j = j / 2;
    }

    values[i] = counter;
  }
  
  return 1;
}


vArray3(short) precomputed_line_columns::left_cols = vZero(vArray2(short));
vArray3(short) precomputed_line_columns::right_cols = vZero(vArray2(short));
long precomputed_line_columns::size = -1;

long precomputed_line_columns::PrecomputeValues(long in_size)
{
  if (in_size <= size) return 1;
  CleanUp();
  size = in_size;
  long array_size = 2 * size + 1;

  left_cols = vnew(vArray2(short), array_size);
  right_cols = vnew(vArray2(short), array_size);
  left_cols = left_cols + size;
  right_cols = right_cols + size;

  long row, col;
  vPrint("\n");
  for (row = 0; row <= size; row++)
  {
    left_cols[row] = vnew(vArray(short), array_size);
    right_cols[row] = vnew(vArray(short), array_size);
    left_cols[row] = left_cols[row] + size;
    right_cols[row] = right_cols[row] + size;
    for (col = 0; col <= size; col++)
    {
      left_cols[row][col] = vLeftCols(row, col);
      right_cols[row][col] = vRightCols(row, col);
    }
    vPrint("Done with row %li of %li\r", row, size);
  }
  vPrint("\n");

  for (row = 0; row <= size; row++)
  {
    if (row != 0)
    {
      left_cols[-row] = left_cols[row];
      right_cols[-row] = right_cols[row];
    }
    for (col = 1; col <= size; col++)
    {
      long number = row+1;
      left_cols[row][-col] = vnew(short, number);
      right_cols[row][-col] = vnew(short, number);
      long i;
      for (i = 0; i < number; i++)
      {
        left_cols[row][-col][i] = -right_cols[row][col][i];
        right_cols[row][-col][i] = -left_cols[row][col][i];
      }
    }
  }

  return 1;
}


long precomputed_line_columns::CleanUp()
{
  if (size < 0) return 1;
  long i, j;
  for (i = 0; i <= size; i++)
  {
    for (j = -size; j <= size; j++)
    {
      vdelete2(left_cols[i][j]);
      vdelete2(right_cols[i][j]);
    }
    left_cols[i] = left_cols[i] - size;
    right_cols[i] = right_cols[i] - size;
    vdelete2(left_cols[i]);
    vdelete2(right_cols[i]);
  }
  left_cols = left_cols - size;
  right_cols = right_cols - size;
  vdelete2(left_cols);
  vdelete2(right_cols);

  return 1;
}

  
vArray5(vPoint) vPrecomputedTriangles::pixels = vZero(vArray4(vPoint));
long vPrecomputedTriangles::size = -1;


long vPrecomputedTriangles::PrecomputeValues(long in_size)
{
  if (in_size <= size) return 1;
  CleanUp();
  size = in_size;
  long array_size = 2 * size + 1;

  pixels = vnew(vArray4(vPoint), array_size);
  pixels = pixels + size;

  vector<vPoint> pixel_vector(array_size * array_size + 10);

  class_pointer(vint8) rows = vnew(vint8, 3);
  class_pointer(vint8) cols = vnew(vint8, 3);
  rows[0] = 0;
  cols[0] = 0;
  long i1, j1, i2, j2;
  vPrint("\n");

  for (i1 = -size; i1 <= size; i1++)
  {
    rows[1] = i1;
    pixels[i1] = vnew(vArray3(vPoint), array_size);
    pixels[i1] = pixels[i1] + size;
    for (j1 = -size; j1 <= size; j1++)
    {
      cols[1] = j1;
      pixels[i1][j1] = vnew(vArray2(vPoint), array_size);
      pixels[i1][j1] = pixels[i1][j1] + size;
      for (i2 = -size; i2 <= size; i2++)
      {
        rows[2] = i2;
        pixels[i1][j1][i2] = vnew(vArray(vPoint), array_size);
        pixels[i1][j1][i2] = pixels[i1][j1][i2] + size;
        for (j2 = -size; j2 <= size; j2++)
        {
          cols[2] = j2;
          vint8 number = vTrianglePixels2(rows, cols, &pixel_vector);
          vArray(vPoint)  triangle_pixels = vnew(vPoint, (vector_size) (number+1));
          pixels[i1][j1][i2][j2] = triangle_pixels;
          triangle_pixels[0].row = number;
          triangle_pixels[0].col = number+1;
          
          long i;
          for (i = 1; i < number+1; i++)
          {
            triangle_pixels[i] = pixel_vector[i-1];
          }
        }
      }
      vPrint("Done with %li %li     \r", i1, j1);
    }
  }
  vPrint("\n");
  delete_pointer(rows);
  delete_pointer(cols);

  return 1;
}


long vPrecomputedTriangles::CleanUp()
{
  if (size < 0) return 1;
  long i1, j1, i2, j2;
  for (i1 = -size; i1 <= size; i1++)
  {
    for (j1 = -size; j1 <= size; j1++)
    {
      for (i2 = -size; i2 <= size; i2++)
      {
        for (j2 = -size; j2 <= size; j2++)
        {
          vdelete2(pixels[i1][j1][i2][j2]);
        }
        pixels[i1][j1][i2] = pixels[i1][j1][i2] - size;
        vdelete2(pixels[i1][j1][i2]);
      }
      pixels[i1][j1] = pixels[i1][j1] - size;
      vdelete2(pixels[i1][j1]);
    }
    pixels[i1] = pixels[i1] - size;
    vdelete2(pixels[i1]);
  }
  pixels = pixels - size;
  vdelete2(pixels);

  return 1;
}


float vPrecomputedExp::raw_values[105000];
float vPrecomputedExp::number = 100000;
float vPrecomputedExp::number2 = 105000;
float vPrecomputedExp::range = 0;
float vPrecomputedExp::limit = 0;
float * vPrecomputedExp::values = 0;
long vPrecomputedExp::half_number = 0;
float vPrecomputedExp::factor = PrecomputeValues((float) 10.0);

float vPrecomputedExp::PrecomputeValues(float in_limit)
{
  long called_before = 0;
  if (called_before == 0)
  {
    called_before = 1;
    half_number = (long) (number2 / 2.0);
    values = raw_values + half_number;
  }

  if (in_limit <= 0) return 0;
  if (in_limit < limit) return factor;

  limit = in_limit;
  range = 2 * limit;
  static const float e = (float) 2.71828182845905;
  factor = number / range;

  long i;
  for (i = 0; i < number2; i++)
  {
    long index = i - half_number;
    float exponent = ((float) index) / factor;
    values[index] = (float) pow(e, exponent);
  }
  return factor;
}


vArray(float) vPrecomputedSqrtf::values = vZero(float);
vArray(float) vPrecomputedSqrtf::factor = vZero(float);
vArray(float) vPrecomputedSqrtf::factor_root = vZero(float);
long vPrecomputedSqrtf::start = -1;
long vPrecomputedSqrtf::end = -1;


long vPrecomputedSqrtf::PrecomputeValues(long in_start)
{
  if (in_start == start)
  {
    return 1;
  }

  if (in_start <= 100)
  {
    exit_error("bad argument, in_start = %li\n", in_start);
  }

  start = in_start;
  end = start * 2;
  long real_end = start * 5;

  vdelete2(values);
  vdelete2(factor);
  vdelete2(factor_root);

  values = vnew(float, real_end);
  factor = vnew(float, real_end);
  factor_root = vnew(float, real_end);
  long i;
  for (i = start; i < real_end; i++)
  {
    values[i] = sqrt((float) i);
  }

  factor[0] = -1;
  factor_root[0] = -1;
  for (i = 1; i < real_end; i++)
  {
    float i_float = (float) i;
    float i_factor = ((float) end) / i_float;
    factor[i] = i_factor;
    factor_root[i] = sqrt(i_factor);
  }

  return 1;
}


long vPrecomputedSqrtf::CleanUp()
{
  vdelete2(values);
  vdelete2(factor);
  vdelete2(factor_root);
  values = vZero(float);
  factor = vZero(float);
  factor_root = vZero(float);
  start = -1;
  end = -1;
  return 1;
}


// this is a general function, that uses precomputed information
// to figure out the pixels that occur at the line segment between the 
// two specified endpoints
// it assumes (for efficiency) that we have already precomputed that information,
// the program may crash otherwise
vint8_matrix function_line_pixels(long first_vertical, long first_horizontal,
                                 long second_vertical, long second_horizontal)
{
  vector <long> horizontal, vertical;
  long horizontal_difference = second_horizontal - first_horizontal;
  long vertical_difference = second_vertical - first_vertical;

  long counter;
  class_pointer(short) first_columns = precomputed_line_columns::LeftCols(vertical_difference, horizontal_difference);
  class_pointer(short) second_columns = precomputed_line_columns::RightCols(vertical_difference, horizontal_difference);
  long absolute_vertical = function_absolute (vertical_difference);
  long factor = 1;
  if (vertical_difference <0) factor = -1;

  for (counter = 0; counter <= absolute_vertical; counter ++)
  {
    long current_horizontal = first_columns [counter];
    long horizontal_limit = second_columns [counter];
    while (current_horizontal <= horizontal_limit)
    {
      horizontal.push_back (current_horizontal+ first_horizontal);
      vertical.push_back (counter*factor + first_vertical);
      current_horizontal ++;
    }
  }

  long number = horizontal.size ();
  vint8_matrix result (2, number);
  for (counter = 0; counter < number; counter ++)
  {
    result (0, counter) = vertical [counter];
    result (1, counter) = horizontal [counter];
  }

  return result;
}


float precomputed_inverse_tangent::buffer[50000];
float * precomputed_inverse_tangent::values = &(precomputed_inverse_tangent::buffer[25000]);
float precomputed_inverse_tangent::half_pi = M_PIf / 2.0f;
float precomputed_inverse_tangent::number2 = 25000;
float precomputed_inverse_tangent::limit = 150;
float precomputed_inverse_tangent::factor = 150;
float precomputed_inverse_tangent::number = precomputed_inverse_tangent::factor * precomputed_inverse_tangent::limit;


long precomputed_inverse_tangent::precompute_values()
{
  static long called_before = 0;
  if (called_before != 0) return 1;
  called_before = 1;
  
  long smallest = (long) round_number(-number2);
  long largest = -smallest-1;

  long i;
  for (i = smallest; i <= largest; i++)
  {
    float vertical = ((float) i) / factor;
    values[i] = atan2(vertical, 1.0f);
  }

  return 1;
}


