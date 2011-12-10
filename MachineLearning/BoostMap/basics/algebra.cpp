
#ifdef VASSILIS_PROCESS_FILE
#undef VASSILIS_PROCESS_FILE
#include "vplatform.h"
#include "algebra.h"
#include "math.h"

#include "basics/definitions.h"


template<class type>
void vMatrix<type>::initialize() 
{
  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
}


// This constructor is used so that a function that returns a vMatrix type
// can return an uninitialized object, that is not valid, in cases of failure.
template<class type>
vMatrix<type>::vMatrix() :
v3dMatrix<type>()
{
  initialize();
  SetRange();
}


template<class type>
vMatrix<type>::vMatrix(const vint8 in_rows, const vint8 in_cols) :
v3dMatrix<type>(in_rows, in_cols, 1)
{
  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();
}


template<class type>
vMatrix<type>::vMatrix(const vint8 in_rows_low, const vint8 in_rows_high, 
                         const vint8 in_cols_low, const vint8 in_cols_high) :
v3dMatrix<type>(in_rows_high - in_rows_low + 1, in_cols_high - in_cols_low + 1, 1)
{
  rows_low = in_rows_low;
  rows_high = in_rows_high;
  cols_low = in_cols_low;
  cols_high = in_cols_high;
  SetRange();
}

template<class type>
vMatrix<type>::vMatrix(const general_image * source) :
v3dMatrix<type>(source, 0)
{
  initialize();
  if (source->valid() <= 0)
  {
    SetRange();
    return;
  }

  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();
}

template<class type>
vMatrix<type>::vMatrix(const general_image & source) :
v3dMatrix<type>(&source, 0)
{
  initialize();
  if (source.valid() <= 0)
  {
    SetRange();
    return;
  }

  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();
}

template<class type>
vMatrix<type>::vMatrix(const general_image * source, vint8 channel) :
v3dMatrix<type>(source, channel)
{
  initialize();
  if (source->valid() <= 0)
  {
    SetRange();
    return;
  }

  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();
}

template<class type>
vMatrix<type>::vMatrix(const vector<double> * const numbers, const vint8 in_cols) :
v3dMatrix<type>(numbers->size() / in_cols, in_cols, 1)
{
  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();

  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    this->matrix[i] = (type) (*numbers)[i];
  }
}

  
template<class type>
vMatrix<type>::vMatrix(const v3dMatrix<type> * source, const vint8 band) :
v3dMatrix<type>(source->Rows(), source->Cols(), 1)
{
  if ((band < 0) || (band >= source->Bands()))
  {
    exit_error("Error: bad band %li in vMatrix constructor\n", band);
  }

  vArray(type) source_band = source->Matrix(band);
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    this->matrix[i] = source_band[i];
  }

  rows_low = 0;
  rows_high = this->rows - 1;
  cols_low = 0;
  cols_high = this->cols - 1;
  SetRange();
}

  
template<class type>
void vMatrix<type>::SetRange()
{
  if (this->size == 0)
  {
    data = function_zero(class_pointer(type));
    return;
  }

  data = vnew(vArray(type), (vector_size) this->rows) - rows_low;
  for (vint8 i = rows_low; i <= rows_high; i++)
  {
    data[i] = this->matrix3[0][i - rows_low] - cols_low;
  }
}

  
template<class type>
vMatrix<type>::~vMatrix()
{
  this->remove_reference();
}


template<class type>
void vMatrix<type>::delete_unique()
{
  data = data + rows_low;
  vdelete2(data);
  v3dMatrix<type>::delete_unique();
}


template<class type>
void vMatrix<type>::Print(const char * str, const char * format) const
{
  vint8 i, j;
  if (str != 0)
  {
    vPrint( "%s:  (%li x %li)\n", str, (long) this->rows, (long) this->cols );
  }
  else
  {
    vPrint("(%li x %li)\n", (long) this->rows, (long) this->cols);
  }

  for(i = rows_low; i <= rows_high; i++ )
  {
    vPrint( "  " );
    for(j = cols_low; j <= cols_high; j++ )
    {
      if (format == 0)
      {
        vPrint(" %.6lf", (double) (data[i][j]) );
      }
      else
      {
        function_print(format,  (double) (data[i][j]) );
      }
    }
    vPrint( "\n" );
  }
  vPrint( "\n" );
}


// This is good for matrices whose entries are 
// integers (i.e. of types uchar, short, int, etc).
template<class type>
void vMatrix<type>::PrintInt(const char * str, const char * format) const
{
  vint8 i, j;
  if (str != 0)
  {
    vPrint( "%s:  (%d x %d)\n", str, (long) this->rows, (long) this->cols );
  }
  else
  {
    vPrint("(%d x %d)\n", (long) this->rows, (long) this->cols);
  }

  for( i = rows_low; i <= rows_high; i++ )
  {
    vPrint( "  " );
    for( j = cols_low; j <= cols_high; j++ )
    {
      if (format == 0)
      {
        vPrint(" %li", (long) round_number((double) data[i][j]) );
      }
      else
      {
        function_print(format, (long) round_number((double) data[i][j]) );
      }
    }
    vPrint( "\n" );
  }
  vPrint( "\n" );
}


template<class type>
void vMatrix<type>::PrintRange(vint8 top, vint8 bottom, vint8 left, vint8 right, 
                                const char * str) const
{
  if (top < 0) 
  {
    top = 0;
  }
  if (bottom >= this->rows) 
  {
    bottom = this->rows - 1;
  }
  if (left < 0) 
  {
    left = 0;
  }
  if (right >= this->cols) 
  {
    right = this->cols;
  }

  vint8 i, j;
  if (str != 0)
  {
    vPrint( "%s:  (%d x %d)\n", str, (long) this->rows, (long) this->cols );
  }
  else
  {
    vPrint("(%d x %d)\n", (long) this->rows, (long) this->cols);
  }

  for(i = top; i <= bottom; i++ )
  {
    vPrint( "  " );
    for(j = left; j <= right; j++)
    {
      vPrint(" %.6lf", (double) (data[i][j]) );
    }
    vPrint( "\n" );
  }
  vPrint( "\n" );
}


template<class type>
void vMatrix<type>::PrintRangeInt(vint8 top, vint8 bottom, vint8 left, vint8 right, 
                                   const char * str) const
{
  if (top < 0) 
  {
    top = 0;
  }
  if (bottom >= this->rows) 
  {
    bottom = this->rows - 1;
  }
  if (left < 0) 
  {
    left = 0;
  }
  if (right >= this->cols) 
  {
    right = this->cols;
  }

  vint8 i, j;
  if (str != 0)
  {
    vPrint( "%s:  (%d x %d)\n", str, (long) this->rows, (long) this->cols );
  }
  else
  {
    vPrint("(%d x %d)\n", (long) this->rows, (long) this->cols);
  }

  for(i = top; i <= bottom; i++ )
  {
    vPrint( "  " );
    for(j = left; j <= right; j++ )
    {
      vPrint(" %li ", (vint8) (data[i][j]) );
    }
    vPrint( "\n" );
  }
  vPrint( "\n" );
}


// These functions are useful in debugging, when we want to compare
// a matlab matrix to a C++ matrix. C++ indices start at zero,
// matlab indices start at one. These functions take in the 
// "matlab" version of the indices, adjust them, and call PrintRange
// and PrintRangeInt.
template<class type>
void vMatrix<type>::PrintRangem(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                 const char * str) const
{
  PrintRange(top-1, bottom-1, left-1, right-1, str);
}


template<class type>
void vMatrix<type>::PrintRangeIntm(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                    const char * str) const
{
  PrintRangeInt(top-1, bottom-1, left-1, right-1, str);
}

  
// For all those print functions, we also have a version that prints
// the transpose. I find it useful sometimes when debugging.
template<class type>
void vMatrix<type>::PrintTrans(const char * str) const
{
  Trans().Print(str);
}


template<class type>
void vMatrix<type>::PrintIntTrans(const char * str) const
{
  Trans().PrintInt(str);
}

template<class type>
void vMatrix<type>::PrintRangeTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                     const char * str) const
{
  Trans().PrintRange(top, bottom, left, right, str);
}


template<class type>
void vMatrix<type>::PrintRangeIntTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                        const char * str) const
{
  Trans().PrintRangeInt(top, bottom, left, right, str);
}


template<class type>
void vMatrix<type>::PrintRangemTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                      const char * str) const
{
  Trans().PrintRangem(top, bottom, left, right, str);
}


template<class type>
void vMatrix<type>::PrintRangeIntmTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                                         const char * str) const
{
  Trans().PrintRangeIntm(top, bottom, left, right, str);
}


template<class type>
vArray2(type) vMatrix<type>::GetData() const
{
  return data;
}


template<class type>
vArray2(type) vMatrix<type>::Matrix2() const
{
  return this->matrix3[0];
}


// Writing values
// Various functions that make it easier to write values into the matrix.
// All the functions that take variable arguments (i.e. that have ...
// in their list of arguments) should only be used when the type 
// is vint8 or double. I could implement these functions so they can work 
// for all types, it's just a bit more complicated.
template<class type>
vint8 vMatrix<type>::WriteN(const vint8 number, ...)
{
  if ((this->type_name != Vint8Type) && (this->type_name != DoubleType))
  {
    exit_error("\nerror: vMatrix vararg functions can only handle vint8 and double types.\n");
  }

  if (number > this->size) return 0;
  va_list arguments;
  vint8 index = 0;
  va_start(arguments, number);
  while(index < number)
  {
    this->matrix[index++] = va_arg(arguments, type);
  }
  va_end(arguments);
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteRow(const vint8 row, ...)
{
  if ((this->type_name != Vint8Type) && (this->type_name != DoubleType))
  {
    exit_error("\nerror: vMatrix vararg functions can only handle vint8 and double types.\n");
  }

  if ((row < rows_low) || (row > rows_high)) return 0;
  vArray(type) pointer = data[row];
  va_list arguments;
  vint8 index = cols_low;
  va_start(arguments, row);
  while(index <= cols_high)
  {
    pointer[index++] = va_arg(arguments, type);
  }
  va_end(arguments);
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteCol(const vint8 col, ...)
{
  if ((this->type_name != Vint8Type) && (this->type_name != DoubleType))
  {
    exit_error("\nerror: vMatrix vararg functions can only handle vint8 and double types.\n");
  }

  if((col < cols_low) || (col > cols_high)) return 0;

  va_list arguments;
  vint8 index = rows_low;
  va_start(arguments, col);
  while(index <= rows_high)
  {
    data[index++][col] = va_arg(arguments, type);
  }
  va_end(arguments);
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteRowN(const vint8 row, const vint8 start, const vint8 n, ...)
{
  if ((this->type_name != Vint8Type) && (this->type_name != DoubleType))
  {
    exit_error("\nerror: vMatrix vararg functions can only handle vint8 and double types.\n");
  }

  if ((row < rows_low) || (row > rows_high)) return 0;
  if (start < cols_low) return 0;
  if (start + n -1 > cols_high) return 0;

  vArray(type) pointer = data[row];
  va_list arguments;
  vint8 index = start;
  vint8 end = start + n;
  va_start(arguments, n);
  while(index < end)
  {
    pointer[index++] = va_arg(arguments, type);
  }
  va_end(arguments);
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteColN(const vint8 col, const vint8 start, const vint8 n, ...)
{
  if ((this->type_name != Vint8Type) && (this->type_name != DoubleType))
  {
    exit_error("\nerror: vMatrix vararg functions can only handle vint8 and double types.\n");
  }

  if ((col < cols_low) || (col > cols_high)) return 0;
  if (start < rows_low) return 0;
  if (start + n -1 > rows_high) return 0;

  va_list arguments;
  vint8 index = start;
  vint8 end = start + n;
  va_start(arguments, n);
  while(index < end)
  {
    data[index++][col] = va_arg(arguments, type);
  }
  va_end(arguments);
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteRowValue(const vint8 row, const type value)
{
  if ((row < rows_low) || (row > rows_high)) return 0;
  vint8 i;
  vArray2(type) row_data = data[row];
  for (i = 0; i < this->cols; i++)
  {
    row_data[i] = value;
  }
  return 1;
}


template<class type>
vint8 vMatrix<type>::WriteColValue(const vint8 col, const type value)
{
  if ((col < cols_low) || (col > cols_high)) return 0;
  vint8 i;
  for (i = 0; i < this->rows; i++)
  {
    data[i][col] = value;
  }
}

  
template<class type>
general_image * vMatrix<type>::Copy() const
{
  if (this->is_valid <= 0)
  {
    return 0;
  }
  vMatrix * result = new vMatrix(rows_low, rows_high, cols_low, cols_high);
  Copy(result, rows_low, rows_high, cols_low, cols_high,
       rows_low, cols_low);
  return result;
}


template<class type>
void vMatrix<type>::Copy(vMatrix * target) const
{
  if (this->is_valid <= 0)
  {
    return;
  }
  if ((target->Rows() != this->rows) || (target->Cols() != this->cols))
  {
    vPrint("Error in vMatrix::Copy. Target is different size than source.\n");
    assert(0);
  }
  Copy(target, rows_low, rows_high, cols_low, cols_high,
       target->rows_low, target->cols_low);
}


template<class type>
void vMatrix<type>::Copy(vMatrix * target, 
                          const vint8 target_top, const vint8 target_left) const
{
  Copy(target, rows_low, rows_high, cols_low, cols_high,
       target_top, target_left);
}


template<class type>
void vMatrix<type>::Copy(vMatrix * target, const vint8 top, const vint8 bottom, 
                          const vint8 left, const vint8 right,
                          const vint8 target_top, const vint8 target_left) const
{
  if (this->is_valid <= 0)
  {
    return;
  }
  vArray2(type) data2 = target->GetData();
  vint8 i1, j1, i2, j2;
  i2 = target_top;

  for (i1 = top; i1 <= bottom; i1++)
  {
    j2 = target_left;
    for (j1 = left; j1 <= right; j1++)
    {
      data2[i2][j2] = data[i1][j1];
      j2++;
    }
    i2++;
  }
}


template<class type>
vMatrix<type> * vMatrix<type>::Add(const vMatrix<type> * b) const
{
  if ((this->rows != b->Rows()) || (this->cols != b->Cols()))
  {
    vPrint("Error: Can't add matrices of different sizes.\n");
    assert(0);
  }
  
  vMatrix * result = new vMatrix(rows_low, rows_high, cols_low, cols_high);
  Add(b, result);
  return result;
}


template<class type>
void vMatrix<type>::Add(const vMatrix<type> * b, 
                         vMatrix<type> * result) const
{
  if ((this->rows != b->Rows()) || (this->cols != b->Cols()) ||
      (this->rows != result->Rows()) || (this->cols != result->Cols()))
  {
    vPrint("Error: Can't add matrices of different sizes.\n");
    assert(0);
  }

  vArray(type) items = b->Matrix();
  vArray(type) result_items = result->Matrix();
  
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    result_items[i] = this->matrix[i] + items[i];
  } 
}


template<class type>
vMatrix<type> * vMatrix<type>::Sub(const vMatrix<type> * b) const
{
  if ((this->rows != b->Rows()) || (this->cols != b->Cols()))
  {
    vPrint("Error: Can't subtract matrices of different sizes.\n");
    assert(0);
  }
  
  vMatrix<type> * result = new vMatrix(rows_low, rows_high, cols_low, cols_high);
  Sub(b, result);
  return result;
}


template<class type>
void vMatrix<type>::Sub(const vMatrix<type> * b, 
                         vMatrix<type> * result) const
{
  if ((this->rows != b->Rows()) || (this->cols != b->Cols()) ||
      (this->rows != result->Rows()) || (this->cols != result->Cols()))
  {
    vPrint("Error: Can't subtract matrices of different sizes.\n");
    assert(0);
  }

  vArray(type) items = b->Matrix();
  vArray(type) result_items = result->Matrix();
  
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    result_items[i] = this->matrix[i] - items[i];
  } 
}


template<class type>
vMatrix<type> * vMatrix<type>::Mult(const vMatrix<type> * b) const
{
  if (this->cols != b->Rows())
  {
    vPrint("Error: Can't multiply matrices of incompatible sizes.\n");
    assert(0);
  }
  
  vMatrix<type> * result = new vMatrix(1, this->rows, 1, b->Cols());
  Mult(b, result);
  return result;
}


template<class type>
void vMatrix<type>::Mult(const vMatrix<type> * b, 
                          vMatrix<type> * result) const
{
  if ((this->rows != result->Rows()) || (this->cols != b->Rows()) ||
      (b->Cols() != result->Cols()))
  {
    exit_error("Error: Can't multiply matrices of incompatible sizes.\n");
  }

  vint8 b_cols = b->Cols();
  
  vArray2(type) data1 = Matrix2();
  vArray2(type) data2 = b->Matrix2();
  vArray2(type) target = result->Matrix2();

  type temp;
  vArray(type) ptr;


  vint8 i, j, k;
  for (i = 0; i < this->rows; i++)
  {
    ptr = data1[i];
    for (j = 0; j < b_cols; j++)
    {
      temp = 0;
      for (k = 0; k < this->cols; k++)
      {
        temp = temp + ptr[k] * data2[k][j];
      }
      target[i][j] = temp;
    }
  }
}


template<class type>
vMatrix<type> * vMatrix<type>::MultTranspose(const vMatrix<type> * b) const
{
  if (this->cols != b->Cols())
  {
    vPrint("Error: Can't multiply matrices of incompatible sizes.\n");
    assert(0);
  }
  
  vMatrix<type> * result = new vMatrix(1, this->rows, 1, b->Rows());
  MultTranspose(b, result);
  return result;
}


template<class type>
void vMatrix<type>::MultTranspose(const vMatrix<type> * b, 
                                   vMatrix<type> * result) const
{
  if ((this->rows != result->Rows()) || (this->cols != b->Cols()) ||
      (b->Rows() != result->Cols()))
  {
    vPrint("Error: Can't multiply matrices of incompatible sizes.\n");
    assert(0);
  }

  vint8 b_rows = b->Rows();
  
  vArray2(type) data1 = Matrix2();
  vArray2(type) data2 = b->Matrix2();
  vArray2(type) target = result->Matrix2();

  type temp;
  vArray(type) ptr;


  vint8 i, j, k;
  for (i = 0; i < this->rows; i++)
  {
    ptr = data1[i];
    for (j = 0; j < b_rows; j++)
    {
      temp = 0;
      for (k = 0; k < this->cols; k++)
      {
        temp = temp + ptr[k] * data2[j][k];
      }
      target[i][j] = temp;
    }
  }
}


template<class type>
vMatrix<type> * vMatrix<type>::TransposeMult(const vMatrix<type> * b) const
{
  if (this->rows != b->Rows())
  {
    vPrint("Error: Can't multiply matrices of incompatible sizes.\n");
    assert(0);
  }
  
  vMatrix<type> * result = new vMatrix(1, this->cols, 1, b->Cols());
  TransposeMult(b, result);
  return result;
}


template<class type>
void vMatrix<type>::TransposeMult(const vMatrix<type> * b, 
                                   vMatrix<type> * result) const
{
  if ((this->cols != result->Rows()) || (this->rows != b->Rows()) ||
      (b->Cols() != result->Cols()))
  {
    vPrint("Error: Can't multiply matrices of incompatible sizes.\n");
    assert(0);
  }

  vint8 b_cols = b->Cols();
  
  vArray2(type) data1 = Matrix2();
  vArray2(type) data2 = b->Matrix2();
  vArray2(type) target = result->Matrix2();

  type temp;

  vint8 i, j, k;
  for (i = 0; i < this->cols; i++)
  {
    for (j = 0; j < b_cols; j++)
    {
      temp = 0;
      for (k = 0; k < this->rows; k++)
      {
        temp = temp + data1[k][i] * data2[k][j];
      }
      target[i][j] = temp;
    }
  }
}


template<class type>
vMatrix<type> * vMatrix<type>::Inverse() const
{
  if (this->rows != this->cols) 
  {
    vPrint("Error: Can't calculate inverse of non-square matrix\n");
    assert(0);
  }

  // Get the inverse
  vMatrix<type> * temp_result = new vMatrix(1, this->rows, 1, this->cols);
  Inverse(temp_result);

  // The result should have the same range of rows and cols as this object.
  vMatrix<type> * result = new vMatrix(rows_low, rows_high, cols_low, cols_high);
  temp_result->Copy(result);
  vdelete(temp_result);

  return result;
}


template<class type>
ushort vMatrix<type>::Inverse(vMatrix * target) const
{
  if (this->rows != this->cols)
  {
    vPrint("Error: Can't calculate inverse of non-square matrix\n");
    assert(0);
  }
  Copy(target);
  return GaussJordan(target, (vMatrix<type> *) 0);
}

template<class type>
vMatrix<type> * vMatrix<type>::Transpose() const
{
  vMatrix<type> * result = new vMatrix<type>(this->cols, this->rows);
  Transpose(result);
  return result;
}

template<class type>
void vMatrix<type>::Transpose(vMatrix<type> * target) const
{
  assert(this->rows == target->Cols());
  assert(this->cols == target->Rows());
  vArray2(type) data1 = Matrix2();
  vArray2(type) data2 = target->Matrix2();
  vint8 i, j;
  for (i = 0; i < this->rows; i++)
    for(j = 0; j < this->cols; j++)
    {
      data2[j][i] = data1[i][j];
    }
}



// gaussj()
// This function performs gauss-jordan elimination to solve a set
// of linear equations, at the same time generating the inverse of
// the A matrix.

// (C) Copr. 1986-92 Numerical Recipes Software `2$m.1.9-153.
// Arguments: 
//
// a: an nxn matrix of linear coefficients.
// n: The number of rows of the square matrix a.
// b: an nxm matrix of what a * x equals (where x is a matrix of the
//    unknown vectors, each of which is m-dimensional).
// m: The number of columns of the matrix b.
//
// Output:
// 
// a: It receives the inverse of what was passed in as argument a.
// b: It receives the values of the x's that solve the equation.
//
// Returns 0 if a singular matrix was given, 1 if it was successful.

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

template<class type>
ushort gaussj(vArray2(type) a, int n, vArray2(type) b, int m)
{
  vArray(vint8) indxc;
  vArray(vint8) indxr;
  vArray(vint8) ipiv;
  vint8 i,icol,irow,j,k,l,ll;
  type big,dum,pivinv,temp;

  vMatrix<vint8> indxc_matrix(1, 1, 1, n);
  vMatrix<vint8> indxr_matrix(1, 1, 1, n);
  vMatrix<vint8> ipiv_matrix(1, 1, 1, n);
  
  indxc=indxc_matrix.GetData()[1];
  indxr=indxr_matrix.GetData()[1];
  ipiv=ipiv_matrix.GetData()[1];
  for (j=1;j<=n;j++) ipiv[j]=0;
  for (i=1;i<=n;i++) 
  {
    big=0.0;
    for (j=1;j<=n;j++)
      if (ipiv[j] != 1)
        for (k=1;k<=n;k++) 
        {
          if (ipiv[k] == 0) 
          {
            if (fabs(a[j][k]) >= big) 
            {
              big=(type) fabs(a[j][k]);
              irow=j;
              icol=k;
            }
          } 
          else if (ipiv[k] > 1) 
          {
            vPrint("gaussj: Singular Matrix-1\n");
            return 0;
          }
        }
    ++(ipiv[icol]);
    if (irow != icol) 
    {
      for (l=1;l<=n;l++) SWAP(a[irow][l],a[icol][l]);
      for (l=1;l<=m;l++) SWAP(b[irow][l],b[icol][l]);
    }
    indxr[i]=irow;
    indxc[i]=icol;
    if (a[icol][icol] == 0.0)
    {
      vPrint("gaussj: Singular Matrix-2");
      return 0;
    }
    pivinv= (type) (1.0/a[icol][icol]);
    a[icol][icol]=1.0;
    for (l=1;l<=n;l++) a[icol][l] *= pivinv;
    for (l=1;l<=m;l++) b[icol][l] *= pivinv;
    for (ll=1;ll<=n;ll++)
      if (ll != icol) {
        dum=a[ll][icol];
        a[ll][icol]=0.0;
        for (l=1;l<=n;l++) a[ll][l] -= a[icol][l]*dum;
        for (l=1;l<=m;l++) b[ll][l] -= b[icol][l]*dum;
      }
  }
  for (l=n;l>=1;l--) {
    if (indxr[l] != indxc[l])
      for (k=1;k<=n;k++)
        SWAP(a[k][indxr[l]],a[k][indxc[l]]);
  }
  return 1;
}


template<class type>
unsigned short gaussj2(vArray2(type) a, int n, vArray2(type) b, int m)
{
//  vArray(vint8) indxc;
//  vArray(vint8) indxr;
//  vArray(vint8) ipiv;
  vint8 i,icol,irow,j,k,l,ll;
  type big,dum,pivinv,temp;

//  vMatrix<vint8> indxc_matrix(1, 1, 1, n);
//  vMatrix<vint8> indxr_matrix(1, 1, 1, n);
//  vMatrix<vint8> ipiv_matrix(1, 1, 1, n);
//  indxc=indxc_matrix.GetData()[1];
//  indxr=indxr_matrix.GetData()[1];
//  ipiv=ipiv_matrix.GetData()[1];

  vMatrix<vint8> indxc_matrix(1, n);
  vMatrix<vint8> indxr_matrix(1, n);
  vMatrix<vint8> ipiv_matrix(1, n);
  
  vArray(vint8) indxc = indxc_matrix.Matrix();
  vArray(vint8) indxr = indxr_matrix.Matrix();
  vArray(vint8) ipiv = ipiv_matrix.Matrix();
//  vPrint("entering gaussj, n = %li, m = %li\n", n, m);

  for (j = 0; j < n; j++) 
  {
    ipiv[j]=0; 
  }

  for (i = 0; i < n; i++) 
  {
    big=0.0;
    for (j = 0; j < n; j++)
    {
      if (ipiv[j] != 1)
      {
        for (k = 0; k < n; k++) 
        {
          if (ipiv[k] == 0) 
          {
            if (fabs(a[j][k]) >= big) 
            {
              big=(type) fabs(a[j][k]);
              irow=j;
              icol=k;
            }
          } 
          else if (ipiv[k] > 1) 
          {
            vPrint("ipiv[%li] = %li\n", k, ipiv[k]);
            function_warning("gaussj: Singular Matrix-1\n");
            return 0;
          }
        }
      }
    }
    ++(ipiv[icol]);
    if (irow != icol) 
    {
      for (l = 0; l < n; l++)
      {
        SWAP(a[irow][l],a[icol][l]);
//        vPrint("swapping: a[%li][%li] = %f, a[%li][%li] = %f\n",
//                  irow, l, a[irow][l], icol, l, a[icol][l]);
      }
      for (l = 0; l < m; l++) 
      {
        SWAP(b[irow][l],b[icol][l]);
      }
    }
    
    indxr[i]=irow;
    indxc[i]=icol;
    if (a[icol][icol] == 0.0)
    {
//      vPrint("gaussj: Singular Matrix-2");
      return 0;
    }
    pivinv= (type) (1.0/a[icol][icol]);
    a[icol][icol]=1.0;
//    vPrint("a[%li][%li] = %f\n", icol, icol, a[icol][icol]);
    for (l = 0; l < n; l++) 
    {
      a[icol][l] *= pivinv;
//      vPrint("a[%li][%li] = %f\n", icol, l, a[icol][l]);
    }
    for (l = 0; l < m; l++) 
    {
      b[icol][l] *= pivinv;
    }
    for (ll = 0; ll < n; ll++)
    {
      if (ll != icol) 
      {
        dum=a[ll][icol];
        a[ll][icol]=0.0;
//        vPrint("a[%li][%li] = %f\n", ll, icol, a[ll][icol]);
        for (l = 0; l < n; l++) 
        {
          a[ll][l] -= a[icol][l]*dum;
//          vPrint("a[%li][%li] = %f\n", ll, l, a[ll][l]);
        }
        for (l = 0; l < m; l++) 
        {
          b[ll][l] -= b[icol][l]*dum;
        }
      }
    }
  }

  for (l = n-1; l >= 0; l--) 
  {
    if (indxr[l] != indxc[l])
    {
      for (k = 0; k < n; k++)
      {
        SWAP(a[k][indxr[l]],a[k][indxc[l]]);
//        vPrint("a[%li][%li] = %f\n", k, indxr[l], a[k][indxr[l]]);
//        vPrint("a[%li][%li] = %f\n", k, indxc[l], a[k][indxc[l]]);
      }
    }
  }
  return 1;
}


#undef SWAP

template<class type>
ushort vMatrix<type>::GaussJordan(vMatrix<type> * a, 
                                   vMatrix<type> * b)
{
  if (a->rows_low != 1) 
  {
    vPrint("Error: GaussJordan assumes matrix is indexed starting at 1.\n");
    assert(0);
  }
  vint8 b_cols;
  vArray2(type) b_data;
  if (b == 0) 
  {
    b_cols = 0;
    b_data = function_zero(vArray(type));
  }
  else 
  {
    b_cols = b->cols;
    b_data = b->GetData();
  }
  return gaussj(a->GetData(), (vector_size)a->Rows(), b_data, (vector_size)b_cols);
}


template<class type>
vMatrix<type> * vMatrix<type>::Identity(const vint8 in_rows)
{
  vMatrix<type> * result = new vMatrix(in_rows, in_rows);
  result->Identity();
}


template<class type>
vMatrix<type> vMatrix<type>::I(const vint8 in_rows)
{
  vMatrix<type> result(in_rows, in_rows);
  result.Identity();
  return result;
}


template<class type>
void vMatrix<type>::Identity()
{
  assert(this->rows == this->cols);
  vint8 row, col;
  for (row = 0; row < this->rows; row++)
    for (col = 0; col < this->cols; col++)
    {
      if (row == col) this->matrix3[0][row][col] = 1;
      else this->matrix3[0][row][col] = 0;
    }
}



// Returns a row vector consisting of the entries (low, low+1, ..., high).
template<class type>
vMatrix<type> vMatrix<type>::Range(const vint8 low, const vint8 high)
{
  vint8 cols = high-low+1;
  if (cols < 1)
  {
    exit_error ("\nerror in vMatrix<type>::Range(%li,%li)\n", low, high);
  }
  vMatrix<type> result_matrix(1, cols);
  vArray(type) result = result_matrix.Matrix();

  vint8 i;
  for (i = low; i <= high; i++)
  {
    result[i-low] = (type) i;
  }

  return result_matrix;
}


// Returns a row vector consisting of the entries 
// (low, low+step, ..., low+(steps-1)*step).
template<class type>
vMatrix<type> vMatrix<type>::Range(const double low, const double step, const vint8 steps)
{
  if (steps < 1)
  {
    exit_error ("\nerror in vMatrix<type>::Range(%lf,%lf,%li)\n", low, step, steps);
  }
  vMatrix<type> result_matrix(1, steps);
  vArray(type) result = result_matrix.Matrix();

  vint8 i;
  for (i = 0; i < steps; i++)
  {
    result[i] = (type) (low + step * (double) i);
  }

  return result_matrix;
}


template<class type>
vMatrix<type> 
vMatrix<type>::operator - () const
{
  return this->matrix * (-1);
}


template<class type>
vMatrix<type> 
vMatrix<type>::Inv() const
{
//  assert(rows == cols);
//  vMatrix<type> result(rows, cols);
//  Inverse(&result);
//  return result;

  assert(this->rows == this->cols);
  // Get the inverse
  vMatrix<type> temp_result(1, this->rows, 1, this->cols);
  Inverse(&temp_result);

  // The result should have the same range of rows and cols as 
  // this object.
  vMatrix<type> result(rows_low, rows_high, cols_low, cols_high);
  temp_result.Copy(&result);
  return result;
}


template<class type>
vMatrix<type> 
vMatrix<type>::Trans() const
{
//  assert(rows == cols);
  vMatrix<type> result(this->cols, this->rows);
  Transpose(&result);
  return result;
}


template<class type>
vMatrix<type> 
vMatrix<type>::operator + (const vMatrix<type> & matrix) const
{
  vMatrix<type> result(this->rows, this->cols);
  Add(&matrix, &result);
  return result;
}


template<class type>
vMatrix<type> 
vMatrix<type>::operator - (const vMatrix<type> & matrix) const
{
  vMatrix<type> result(this->rows, this->cols);
  Sub(&matrix, &result);
  return result;
}


template<class type>
vMatrix<type> 
vMatrix<type>::operator * (const vMatrix<type> & matrix) const
{
  vMatrix<type> result(this->rows, matrix.Cols());
  Mult(&matrix, &result);
  return result;
}


template<class type>
vMatrix<type> 
vMatrix<type>::operator / (const vMatrix<type> & matrix) const
{
  return *this * matrix.Inv();
}


template<class type>
vMatrix<type> vMatrix<type>::operator + (const type & scalar) const
{
  vMatrix<type> result(this->rows, this->cols);
  vArray(type) data1 = this->Matrix();
  vArray(type) data2 = result.Matrix();
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    data2[i] = data1[i] + scalar;
  }
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::operator - (const type & scalar) const
{
  vMatrix<type> result(this->rows, this->cols);
  vArray(type) data1 = this->Matrix();
  vArray(type) data2 = result.Matrix();
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    data2[i] = data1[i] - scalar;
  }
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::operator * (const type & scalar) const
{
  vMatrix<type> result(this->rows, this->cols);
  vArray(type) data1 = this->Matrix();
  vArray(type) data2 = result.Matrix();
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    data2[i] = data1[i] * scalar;
  }
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::operator / (const type & scalar) const
{
  assert(scalar != 0);
  vMatrix<type> result(this->rows, this->cols);
  vArray(type) data1 = this->Matrix();
  vArray(type) data2 = result.Matrix();
  vint8 i;
  for (i = 0; i < this->size; i++)
  {
    data2[i] = data1[i] / scalar;
  }
  return result;
}


template<class type>
vMatrix<type> operator + (const type & scalar, const vMatrix<type> & matrix)
{
  return matrix + scalar;
}


template<class type>
vMatrix<type> operator - (const type & scalar, const vMatrix<type> & matrix)
{
  return scalar + (matrix * (-1));
}


template<class type>
vMatrix<type> operator * (const type & scalar, const vMatrix<type> & matrix)
{
  return matrix * scalar;
}


template<class type>
vMatrix<type> operator / (const type & scalar, const vMatrix<type> & matrix)
{
  return scalar * matrix.Inv();
}


// Dot product between two vectors. Dimensions are not checked,
// as vint8 as the matrices have the same size.
template<class type>
type vDot(const vMatrix<type> & m1_matrix, const vMatrix<type> & m2_matrix) 
{
  vint8 size1 = m1_matrix.Size();
  vint8 size2 = m2_matrix.Size();
  if (size1 != size2)
  {
    exit_error("Dot product called on incompatible vectors\n");
  }

  vArray(type) m1 = m1_matrix.Matrix();
  vArray(type) m2 = m2_matrix.Matrix();

  type result = 0;
  vint8 i;
  for (i = 0; i < size1; i++)
    result += m1[i] * m2[i];

  return result;
}


// Cross product between two vectors. Dimensions are not checked,
// as vint8 as the size of each vector is 3.
template<class type>
vMatrix<type> vCross(const vMatrix<type> & m1_matrix, const vMatrix<type> & m2_matrix) 
{
  vint8 size1 = m1_matrix.Size();
  vint8 size2 = m2_matrix.Size();
  if ((size1 != 3) || (size2 != 3))
  {
    exit_error("Cross product called on incompatible vectors\n");
  } 

  vArray(type) m1 = m1_matrix.Matrix();
  vArray(type) m2 = m2_matrix.Matrix();
  vMatrix<type> result_matrix(1, 3);
  vArray(type) result = result_matrix.Matrix();
  result[0] = m1[1] * m2[2] - m1[2] * m2[1];
  result[1] = m1[2] * m2[0] - m1[0] * m2[2];
  result[2] = m1[0] * m2[1] - m1[1] * m2[0];
  return result_matrix;
}


template<class type>
vMatrix<type> vMatrix<type>::Read(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) 
  {
    return vMatrix<type>();
  }
  vMatrix<type> result = Read(fp);
  fclose(fp);
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::Read(FILE * fp)
{
  integer rows, cols, bands, type_number;
  vint8 item = read_integer(fp, &type_number);
  if (item != 1)
  {
    return vMatrix<type>();
  }
  item = read_integer(fp, &rows);
  if (item != 1)
  {
    return vMatrix<type>();
  }
  item = read_integer(fp, &cols);
  if (item != 1)
  {
    return vMatrix<type>();
  }
  item = read_integer(fp, &bands);
  if (item != 1)
  {
    return vMatrix<type>();
  }

  if (bands != 1)
  {
    return vMatrix<type>();
  }

  vMatrix<type> result(rows, cols);
  vint8 result_type = vTypeToVint8(result.TypeName());
  if ((type_number != result_type) &&
      (! (type_number == vTypeToVint8(IntType)) &&
         (result_type == vTypeToVint8(Vint8Type))))
  {
    return vMatrix<type>();
  }
  vint8 success = result.ReadBandwise(fp);
  if (success <= 0)
  {
    function_print("failed to read data\n");
    return vMatrix<type>();
  }
  return result;
}


// used for debugging
template<class type>
vMatrix<type> vMatrix<type>::read2(FILE * fp)
{
  integer rows, cols, bands, type_number;
  vint8 item = read_integer(fp, &type_number);
  if (item != 1)
  {
    function_print("failed to read type number, item = %li\n", item);
    return vMatrix<type>();
  }
  function_print("type_number = %li\n", type_number);

  item = read_integer(fp, &rows);
  if (item != 1)
  {
    function_print("failed to read rows, item = %li\n", item);
    return vMatrix<type>();
  }
  function_print("rows = %li\n", rows);

  item = read_integer(fp, &cols);
  if (item != 1)
  {
    function_print("failed to read cols number, item = %li\n", item);
    return vMatrix<type>();
  }
  function_print("cols = %li\n", cols);

  item = read_integer(fp, &bands);
  if (item != 1)
  {
    function_print("failed to read bands, item = %li\n", item);
    return vMatrix<type>();
  }
  function_print("bands = %li\n", bands);

  if (bands != 1)
  {
    return vMatrix<type>();
  }

  vMatrix<type> result(rows, cols);
  long result_type = vTypeToVint8(result.TypeName());

  // make a special case for matrices that were saved in long format
// using 32-bit code, and now we must read as int format
  if (!((type_number == (long) Vint8Type) && (result_type == (long) IntType)))
  { 
  if (type_number != result_type) 
  {
    function_print("type conversion failed, type_number = %li, result_type = %li\n", 
                   type_number, result_type);
    return vMatrix<type>();
  }
  }
  long success = result.ReadBandwise(fp);
  if (success <= 0)
  {
    function_print("failed to read data\n");
    return vMatrix<type>();
  }

  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::ReadText(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) 
  {
    return vMatrix<type>();
  }
  vMatrix<type> result = ReadText(fp);
  fclose(fp);
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::ReadText(FILE * fp)
{
  integer type_number = 0, rows = 0, cols = 0, bands = 0;
  integer items = fscanf(fp, "%li %li %li %li", &type_number, &rows, &cols, &bands);
  if (items != 4) 
  {
    return vMatrix<type>();
  }
  if (bands != 1)
  {
    return vMatrix<type>();
  }

  vMatrix<type> result(rows, cols);
  vint8 result_type = vTypeToVint8(result.TypeName());
  if (type_number != result_type)
  {
    // Some conversions are OK. For a start, reading a double matrix as a float
    // or vice versa is considered OK.
    if ((type_number == (long) DoubleType) && 
        (result.TypeName() == FloatType))
    {
      // Do nothing
    }
    else if ((type_number == (long) FloatType) && 
             (result.TypeName() == DoubleType))
    {
      // Do nothing
    }
    else
    {
      exit_error("type_number = %li instead of %li\n", type_number,
                      (long) result.TypeName());
    }
  }

  vint8 success = result.ReadTextBandwise(fp);
  if (success <= 0)
  {
    return vMatrix<type>();
  }
  return result;
}


template<class type>
vMatrix<type> vMatrix<type>::Zero(const vint8 vertical, const vint8 horizontal)
{
  vMatrix<type> result(vertical, horizontal);
  vint8 length = result.length();
  vint8 counter;
  for (counter = 0; counter < length; counter++)
  {
    result(counter) = 0;
  }

  return result;
}


// copies the specified column into the output matrix
// or output vector.
template<class type>
vMatrix<type> vMatrix<type>::vertical_line(const vint8 index) const
{
  if ((index <0) || (index >= this->cols))
  {
    exit_error("\nerror: index = %li, cols = %li in vertical_line\n",
                        index, this->cols);
  }
  matrix_pointer(type) input = this->matrix3[0];

  vMatrix<type> result(1, this->rows);
  class_pointer(type) pointer = result.flat();
  vint8 counter;
  for (counter = 0; counter < this->rows; counter++)
  {
    pointer[counter] = input[counter][index];
  }

  return result;
}


template<class type>
vint8 vMatrix<type>::vertical_line(const vint8 index, std::vector<type> * output) const
{
  if ((index <0) || (index >= this->cols))
  {
    exit_error("\nerror: index = %li, cols = %li in vertical_line\n",
                        index, this->cols);
  }
  matrix_pointer(type) input = this->matrix3[0];
  output->reserve(this->rows);
  
  vint8 counter;
  for (counter = 0; counter < this->rows; counter++)
  {
    output->push_back(input[counter][index]);
  }

  return 1;
}


// copies the specified row into the output matrix
// or output vector.
template<class type>
vMatrix<type> vMatrix<type>::horizontal_line(const vint8 index) const
{
  if ((index <0) || (index >= this->rows))
  {
    exit_error("\nerror: index = %li, rows = %li in vertical_line\n",
                        index, this->rows);
  }
  class_pointer(type) input = this->matrix3[0][index];

  vMatrix<type> result(1, this->cols);
  class_pointer(type) pointer = result.flat();
  vint8 counter;
  for (counter = 0; counter < this->cols; counter++)
  {
    pointer[counter] = input[counter];
  }

  return result;
}



template<class type>
vint8 vMatrix<type>::horizontal_line(const vint8 index, std::vector<type> * output) const
{
  if ((index <0) || (index >= this->rows))
  {
    exit_error("\nerror: index = %li, rows = %li in vertical_line\n",
                        index, this->rows);
  }
  class_pointer(type) input = this->matrix3[0][index];
  output->reserve(this->cols);
  
  vint8 counter;
  for (counter = 0; counter < this->cols; counter++)
  {
    output->push_back(input[counter]);
  }

  return 1;
}


// copies the specified row into the output matrix
// or output vector.
template<class type>
vMatrix<type> vMatrix<type>::set_range(type lower, type upper) const
{
  vMatrix<type> result(this->vertical(), this->horizontal());
  v3dMatrix<type> * converted_this = (v3dMatrix<type> *) this;
  converted_this->set_range(lower, upper, & result);

  return result;
}







#include "undefine.h"

#endif // VASSILIS_PROCESS_FILE


