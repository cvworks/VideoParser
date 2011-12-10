#ifndef VASSILIS_PRECOMPUTED_H
#define VASSILIS_PRECOMPUTED_H

#include "auxiliaries.h"
#include "algebra.h"

//#include "definitions.h"


// Calls the appropriate functions in each class so that values get 
// precomputed and we don't have to initialize each function separately
long vPrecomputeValues();


// Deletes any dynamically allocated memory used to store precomputed
// values. We should call this function before the end of the program,
// so that this memory will not show up as memory leaks.
long vDeletePrecomputedValues();


class vPrecomputedCos
{
private:
  static float values[23000];
  static double two_pi;
  static double four_pi;
  static double number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static double number2;
  static double factor;

public:
  static double PrecomputeValues();

  // It is the responsibility of the caller to ensure that rad is between
  // 0 and 2*pi.
  static inline double Cos2(double rad)
  {
    long index = (long) (rad * factor);
    return values[index];
  }

  static inline double Cos(double rad)
  {
    float double_index = (float) (rad * factor);
    long index = (long) double_index;
    long inc_index = index+1;
    float weight1 = inc_index-double_index;
    float weight2 = double_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline double Factor()
  {
    return factor;
  }
};


class vPrecomputedCosf
{
private:
  static float buffer[46000];
  static float * values;
  static float two_pi;
  static float four_pi;
  static float number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static float number2;
  static float factor;

public:
  static float PrecomputeValues();

  // It is the responsibility of the caller to ensure that rad is between
  // -2*pi and 2*pi.
  static inline float Cos2(float rad)
  {
    long index = (long) (rad * factor);
    return values[index];
  }

  static inline float Cos(float rad)
  {
    float float_index = (float) (rad * factor);
    long index = (long) float_index;
    long inc_index = index+1;
    float weight1 = inc_index-float_index;
    float weight2 = float_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline float Factor()
  {
    return factor;
  }
};


class vPrecomputedSinf
{
private:
  static float buffer[46000];
  static float * values;
  static float two_pi;
  static float four_pi;
  static float number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static float number2;
  static float factor;

public:
  static float PrecomputeValues();

  // It is the responsibility of the caller to ensure that rad is between
  // -2*pi and 2*pi.
  static inline float Sin2(float rad)
  {
    long index = (long) (rad * factor);
    return values[index];
  }

  static inline float Sin(float rad)
  {
    float float_index = (float) (rad * factor);
    long index = (long) float_index;
    long inc_index = index+1;
    float weight1 = inc_index-float_index;
    float weight2 = float_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline float Factor()
  {
    return factor;
  }
};


class vPrecomputedSin
{
private:
  static float values[23000];
  static double two_pi;
  static double four_pi;
  static double number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static double number2;
  static double factor;

public:
  static double PrecomputeValues();

  // It is the responsibility of the caller to ensure that rad is between
  // 0 and 2*pi.
  static inline double Sin2(double rad)
  {
    long index = (long) (rad * factor);
    return values[index];
  }

  static inline double Sin(double rad)
  {
    float double_index = (float) (rad * factor);
    long index = (long) double_index;
    long inc_index = index+1;
    float weight1 = inc_index-double_index;
    float weight2 = double_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline double Factor()
  {
    return factor;
  }
};


class vPrecomputedTan
{
private:
  static float values[23000];
  static double two_pi;
  static double four_pi;
  static double number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static double number2;
  static double factor;

public:
  static double PrecomputeValues();

  // It is the responsibility of the caller to ensure that rad is between
  // 0 and 2*pi.
  static inline double Tan2(double rad)
  {
    long index = (long) (rad * factor);
    return values[index];
  }

  static inline double Tan(double rad)
  {
    float double_index = (float) (rad * factor);
    long index = (long) double_index;
    long inc_index = index+1;
    float weight1 = inc_index-double_index;
    float weight2 = double_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline double Factor()
  {
    return factor;
  }
};


class vPrecomputedAcos
{
private:
  static float values[23000];
  static double number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static double number2;
  static double factor;

public:
  static double PrecomputeValues();

  // It is the responsibility of the caller to ensure that the argument
  // is between -1 and 1.
  static inline double Acos2(double cosine)
  {
    long index = (long) ((cosine+1) * factor);
    return values[index];
  }

  static inline double Acos(double cosine)
  {
    float double_index = (float) ((cosine+1) * factor);
    long index = (long) double_index;
    long inc_index = index+1;
    float weight1 = inc_index-double_index;
    float weight2 = double_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline double Factor()
  {
    return factor;
  }
};


// Precompute the length of vector (row, col), where
// everything is expressed as an integer.
class vPrecomputedLengths
{
public:
  static vArray2(long) values;
  static vArray2(float) valuesf;
  static long rows, cols, low_row, high_row, low_col, high_col;

public:
  static long PrecomputeValues(long in_rows, long in_cols);
  static inline long Length(long row, long col)
  {
    return values[row][col];
  }

  static inline float Lengthf(long row, long col)
  {
    return valuesf[row][col];
  }

  // Same as Length, but checks for valid row and col (so it doesn't
  // crash if they are wrong).
  static inline long LengthR(long row, long col)
  {
    if ((row < low_row) || (row > high_row) ||
        (col < low_col) || (col > high_col))
    {
      exit_error("Bad arguments to LengthR\n");
      return -1;
    }
   
    return values[row][col];
  }

  static inline float LengthfR(long row, long col)
  {
    if ((row < low_row) || (row > high_row) ||
        (col < low_col) || (col > high_col))
    {
      exit_error("Bad arguments to LengthR\n");
      return -1;
    }
   
    return valuesf[row][col];
  }

  static inline long CleanUp()
  {
    vDelete2(values, rows);
    vDelete2(valuesf, rows);
    values = vZero(vArray(long));
    valuesf = vZero(vArray(float));
    return 1;
  }
};


// Precompute the angle that vector (col, row) makes with
// the x axis, in radians, between 0 and PI.
class vPrecomputedAngles
{
public:
  static vArray2(float) values;
  static long rows, cols, low_row, high_row, low_col, high_col;

public:
  static long PrecomputeValues(long in_rows, long in_cols);
  static inline float Angle(long row, long col)
  {
    return values[row+rows][col+cols];
  }

  // Same as Angle, but checks for valid row and col (so it doesn't
  // crash if they are wrong).
  static inline float AngleR(long row, long col)
  {
    if ((row < low_row) || (row > high_row) ||
        (col < low_col) || (col > high_col))
    {
      exit_error("Bad arguments to AngleR\n");
      return -1;
    }
   
    return values[row+rows][col+cols];
  }

  static inline long CleanUp()
  {
    vDelete2(values, 2*rows);
    values = vZero(vArray(float));
    return 1;
  }
};


// Precompute the angle that vector (col, row) makes with
// the x axis, in radians, between 0 and 2*PI.
class vPrecomputedAngles2
{
public:
  static vArray2(float) values;
  static long rows, cols, low_row, high_row, low_col, high_col;

public:
  static long PrecomputeValues(long in_rows, long in_cols);
  static inline double Angle(long row, long col)
  {
    return values[row+rows][col+cols];
  }

  // Same as Angle, but checks for valid row and col (so it doesn't
  // crash if they are wrong).
  static inline float AngleR(long row, long col)
  {
    if ((row < low_row) || (row > high_row) ||
        (col < low_col) || (col > high_col))
    {
      exit_error("Bad arguments to AngleR\n");
      return -1;
    }
   
    return values[row+rows][col+cols];
  }

  static inline long CleanUp()
  {
    vDelete2(values, 2*rows);
    values = vZero(vArray(float));
    return 1;
  }
};


class vPrecomputedSqrt
{
private:
  static vArray(long) values;
  static long limit;

public:
  static long PrecomputeValues(long limit);
  static inline long Sqrt(long number)
  {
    return values[number];
  }

  // Same as Sqrt, but checks for valid number
  static inline long SqrtR(long number)
  {
    if ((number < 0) || (number > limit))
    {
      return -1;
    }
    return values[number];
  }

  static inline long CleanUp()
  {
    vdelete2(values);
    values = vZero(long);
    return 1;
  }
};


// Base-2 logs
class vPrecomputedLogs
{
private:
  static vArray(double) values;
  static long limit;

public:
  static long PrecomputeValues(long limit);
  static inline double Log(long number)
  {
    return values[number];
  }

  // Same as Log, but checks for valid number
  static inline double LogR(long number)
  {
    if ((number < 0) || (number > limit))
    {
      return -1;
    }
    return values[number];
  }

  static inline long CleanUp()
  {
    vdelete2(values);
    values = vZero(double);
    return 1;
  }
};


// For every number from 0 to 65535 we precalculate the number of
// ones it has in its binary form. For examples, BitSums(1) = 1,
// BitSums(254) = 7. The answer is always from 0 to 16.
class vPrecomputedBitSums
{
private:
  static long values[65536];

public:
  static long PrecomputeValues();

  static inline long BitSums(long number)
  {
    return values[number];
  }

  static inline long BitSumsR(long number)
  {
    if (number < 0) return -1;
    if (number >= 65536) return -1;
    return values[number];
  }

  static inline const long * Values()
  {
    return values;
  }
};


// For every line from (0, 0) to (i, j), where
// |i| < size and |j| < size 
// precompute, for every row of the line, the leftmost 
// and rightmost col that contains a line pixel.
class precomputed_line_columns
{
public:
  static vArray3(short) left_cols;
  static vArray3(short) right_cols;
  static long size;

public:
  static long PrecomputeValues(long size);
  static inline vArray(short) LeftCols(long row, long col)
  {
    return left_cols[row][col];
  }

  static inline vArray(short) RightCols(long row, long col)
  {
    return right_cols[row][col];
  }

  // Same as LeftCols and RightCols, but check for valid row and col 
  // (so it doesn't crash if they are wrong).
  static inline vArray(short) LeftColsR(long row, long col)
  {
    if ((vAbs(row) >= size) || (vAbs(col) >= size))
    {
      exit_error("Bad arguments to LeftColsR: %li %li %li\n",
                      row, col, size);
    }
   
    return left_cols[row][col];
  }

  static inline vArray(short) RightColsR(long row, long col)
  {
    if ((vAbs(row) >= size) || (vAbs(col) >= size))
    {
      exit_error("Bad arguments to LeftColsR: %li %li %li\n",
                      row, col, size);
    }
   
    return right_cols[row][col];
  }

  static long CleanUp();
};


// For any triangle with one vertex at (0,0), one
// at (i1,j1) and one at (i2, j2), precompute the pixels
// that belong to the triangle.
// The array of pixels for each triangle starts with a fake pixel, 
// whose row contains the number of points in the triangle.
class vPrecomputedTriangles
{
public:
  static vArray5(vPoint) pixels;
  static long size;

public:
  static long PrecomputeValues(long size);
  static inline vArray(vPoint) Pixels(long i1, long j1, long i2, long j2)
  {
    return pixels[i1][j1][i2][j2];
  }

  // Same as Pixels, but check for valid row and col 
  // (so it doesn't crash if they are wrong).
  static inline vArray(vPoint) PixelsR(long i1, long j1, long i2, long j2)
  {
    if ((vAbs(i1) >= size) || (vAbs(j1) >= size) ||
        (vAbs(i2) >= size) || (vAbs(j2) >= size))
    {
      exit_error("Bad arguments to Pixels: %li %li %li %li %li\n",
                      i1, j1, i2, j2, size);
    }
   
    return pixels[i1][j1][i2][j2];
  }

  static long CleanUp();
};


class vPrecomputedExp
{
private:
  static float raw_values[105000];
  static float * values;
  static float e;
  static float limit;
  static float range;
  static long half_number;
  // The actual size of the array is number2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static float number;
  static float number2;
  static float factor;

public:
  static float PrecomputeValues(float in_limit);

  // It is the responsibility of the caller to ensure that the argument
  // is between -limit and limit.
  static inline float Exp2(float exponent)
  {
    vint8 index = round_number(exponent * factor);
    return values[index];
  }

  static inline float Exp(float exponent)
  {
    //if (exponent > 88)
    //{
    //  //function_warning("Warning: exponent = %f, will give infinite result\n",
    //    //        exponent);
    //  vPrint("Warning: exponent = %f, will give infinite result\n",
    //            exponent);
    //}
    //
    //return exp(exponent);

    static const float e = (float) 2.71828182845905;
    if (vAbs(exponent) > limit)
    {
      return pow(e, exponent);
    }
    if (exponent > 88)
    {
      function_warning("Warning: exponent = %f, will give infinite result\n",
                exponent);
    }
    float float_index = exponent * factor;
    long index = (long) float_index;
    long inc_index = index+1;
    float weight1 = inc_index-float_index;
    float weight2 = float_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline double Exp(double exponent)
  {
    //if (exponent > 88)
    //{
    //  //function_warning("Warning: exponent = %f, will give infinite result\n",
    //    //        exponent);
    //  vPrint("Warning: exponent = %f, will give infinite result\n",
    //            exponent);
    //}
    //
    //return exp(exponent);


    static const double e = (double) 2.71828182845905;
    if (vAbs(exponent) > limit)
    {
      return pow(e, exponent);
    }
    if (exponent > 88)
    {
      function_warning("Warning: exponent = %f, will give infinite result\n",
                exponent);
    }
    double float_index = exponent * factor;
    long index = (long) float_index;
    long inc_index = index+1;
    double weight1 = inc_index-float_index;
    double weight2 = float_index-index;
    return weight1 * values[index] + weight2 * values[inc_index];
  }

  static inline const float * Values()
  {
    return values;
  }

  static inline float Factor()
  {
    return factor;
  }
};


// Here we also want to compute square roots of floats
class vPrecomputedSqrtf
{
private:
  static vArray(float) values;
  static vArray(float) factor;
  static vArray(float) factor_root;
  static long start;
  static long end;
  static long size;

public:
  static long PrecomputeValues(long in_start);

  static inline float Sqrtf(float number)
  {
    if (number <= 0.5)
    {
      return sqrt(number);
    }

    vint8 index = round_number(number);
    float scaled = number * factor[index];
    vint8 index2 = (vint8) scaled;
    vint8 index3 = index2 + 1;

    float weight2 = ((float) index3) - scaled;
    float weight3 = scaled - (float) index2;
    float result = values[index2] * weight2 + values[index3] * weight3;
    result = result / factor_root[index];
    return result;
  }


  // Same as Sqrt, but checks for valid number
  static inline float SqrtfR(float number)
  {
    if ((number < 0) || (number > end))
    {
      return -1;
    }

    return Sqrtf(number);
  }

  static long CleanUp();
};


// this is a general function, that uses precomputed information
// to figure out the pixels that occur at the line segment between the 
// two specified endpoints
// it assumes (for efficiency) that we have already precomputed that information,
// the program may crash otherwise
vint8_matrix function_line_pixels(long first_vertical, long first_horizontal,
                                 long second_vertical, long second_horizontal);


class precomputed_inverse_tangent
{
private:
  static float buffer[50000];
  static float * values;
  static float half_pi;
  static float number;
  static float limit;

  // The size of the array is number2*2, not number, to allow for 
  // rounding errors that may lead to an index that is too high.
  static float number2;
  static float factor;

public:
  static long precompute_values();

  // It is the responsibility of the caller to ensure that
  // neither vertical nor horizontal is zero.
  static inline float value(float vertical, float horizontal)
  {
    float ratio = vertical/horizontal;
    float first_constant, second_constant;
    if ((ratio > -1) && (ratio < 1))
    {
      ratio = 1.0f / ratio;
      first_constant = half_pi;
      second_constant = 1.0f;
    }
    else
    {
      first_constant = 0.0f;
      second_constant = 0.0f;
    }

    float result;
    if ((ratio > limit) || (ratio < -limit))
    {
      result = half_pi;
    }
    else
    {
      result = values[(long) (ratio*factor)];
    }

    result = first_constant - second_constant * result;
    return result;
  }

  static inline const float * get_values()
  {
    return values;
  }

};







#endif // VASSILIS_PRECOMPUTED_H
