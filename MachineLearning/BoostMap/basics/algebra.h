#ifndef VASSILIS_ALGEBRA_H
#define VASSILIS_ALGEBRA_H

#include "vplatform.h"
#include "matrix.h"

/*!
	This class implements 2D matrices. It inherits from v3dMatrix.
	It adds several nice features to that class: We can
	access the element i,j directly using the () operator. We can
	choose to index elements starting with any integer, as opposed to 
	having to start at 0. Several operators have been overloaded
	so that we can add, multiply, subtract and divide matrices.
	
	The data is stored in the matrix3 array of the v3dMatrix
	superclass. The "data" member variable of vMatrix is set up
	so that using it we can access the i,j element as data[i][j]
	(which is 
	not necessarily the same as matrix3[0][i][j], they only 
	coincide when indexing for rows and columns starts at 0).
*/
template<class type>
class vMatrix : public v3dMatrix<type>
{
protected:
  vArray2(type) data;
  vint8 rows_low, rows_high, cols_low, cols_high;
  void SetRange();

  virtual void delete_unique();
  void initialize();
public:
  vMatrix();
  vMatrix(const vint8 in_rows, const vint8 in_cols);
  vMatrix(const vint8 in_rows_low, const vint8 in_rows_high, 
          const vint8 in_cols_low, const vint8 in_cols_high);
  explicit vMatrix(const general_image * source);
  explicit vMatrix(const general_image & source);
  vMatrix(const general_image * source, vint8 band);
  vMatrix(const std::vector<double> * numbers, const vint8 in_cols);

  // Copies the specified band of the input
  vMatrix(const v3dMatrix<type> * source, const vint8 band);
  ~vMatrix();

  void Print(const char * str = 0, const char * format = 0) const;
  void PrintInt(const char * str = 0, const char * format = 0) const;

  inline void print (const char* argument = 0, const char * format = 0) const
  {
    Print (argument, format);
  }

  inline void print_integer (const char* argument = 0, const char * format = 0) const
  {
    PrintInt (argument, format);
  }

  void PrintRange(vint8 top, vint8 bottom, vint8 left, vint8 right, 
                  const char * str = 0) const;
  void PrintRangeInt(vint8 top, vint8 bottom, vint8 left, vint8 right, 
                     const char * str = 0)  const;

  // These functions are useful in debugging, when we want to compare
  // a matlab matrix to a C++ matrix. C++ indices start at zero,
  // matlab indices start at one. These functions take in the 
  // "matlab" version of the indices, adjust them, and call PrintRange
  // and PrintRangeInt.
  void PrintRangem(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                  const char * str = 0) const;
  void PrintRangeIntm(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                     const char * str = 0) const;

  // For all those print functions, we also have a version that prints
  // the transpose. I find it useful sometimes when debugging.
  void PrintTrans(const char * str = 0) const;
  void PrintIntTrans(const char * str = 0) const;

  void PrintRangeTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                       const char * str = 0) const;
  void PrintRangeIntTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                          const char * str = 0) const;

  void PrintRangemTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                        const char * str = 0) const;
  void PrintRangeIntmTrans(const vint8 top, const vint8 bottom, const vint8 left, const vint8 right, 
                           const char * str = 0) const;

  vArray2(type) GetData() const;

  // Returns matrix3[0]
  vArray2(type) Matrix2() const;
  inline vArray2(type) planar()
  {
    return Matrix2();
  }

  inline vArray2(type) planar_safe()
  {
    if (this->is_valid <= 0)
    {
      return function_zero(class_pointer(type));
    }
    return Matrix2();
  }

  // Various functions that make it easier to write values into the matrix.
  // All the functions that take variable arguments (i.e. that have ...
  // in their list of arguments) should only be used when the type 
  // is vint8 or double. I could implement these functions so they can work 
  // for all types, it's just a bit more complicated.
  vint8 WriteN(const vint8 number, ...);
  vint8 WriteRow(const vint8 row, ...);
  vint8 WriteCol(const vint8 col, ...);
  vint8 WriteRowN(const vint8 row, const vint8 start, const vint8 n, ...);
  vint8 WriteColN(const vint8 col, const vint8 start, const vint8 n, ...);
  vint8 WriteRowValue(const vint8 row, const type value);
  vint8 WriteColValue(const vint8 col, const type value);

  // Referencing items
  inline type & operator () (const vint8 row, const vint8 col) const
  {
    if ((row >= rows_low) && (row <= rows_high) &&
           (col >= cols_low) && (col <= cols_high))
    {
      return data[row][col];         
    }
    else
    {
      exit_error("\nerror: bad indices in matrix operator (): %li %li\n", row, col);
      return data[0][0];
    }
  }
  
  inline type & operator () (const vint8 row, const vint8 col)
  {
    if ((row >= rows_low) && (row <= rows_high) &&
           (col >= cols_low) && (col <= cols_high))
    {
      return data[row][col];         
    }
    else
    {
      exit_error("\nerror: bad indices in matrix operator (): %li %li\n", row, col);
      return data[0][0];
    }
  }
  
  inline type & operator () (const vint8 index) const
  {
    if ((index < this->size) && (index >= 0))
    {
      return this->matrix[index];
    }
    else
    {
      exit_error("\nerror: bad index in matrix operator (): %li\n", index);
      return this->matrix[0];
    }
  }

  inline type & operator () (const vint8 index)
  {
    if ((index < this->size) && (index >= 0))
    {
      return this->matrix[index];
    }
    else
    {
      exit_error("\nerror: bad index in matrix operator (): %li\n", index);
      return this->matrix[0];
    }
  }

  // Matrix operations
  vMatrix<type> operator - () const;
  vMatrix<type> Inv() const;
  vMatrix<type> Trans() const;
  vMatrix<type> operator + (const vMatrix<type> & matrix) const;
  vMatrix<type> operator - (const vMatrix<type> & matrix) const;
  vMatrix<type> operator * (const vMatrix<type> & matrix) const;
  vMatrix<type> operator / (const vMatrix<type> & matrix) const;

  // Scalar operations
  vMatrix<type> operator + (const type & scalar) const;
  vMatrix<type> operator - (const type & scalar) const;
  vMatrix<type> operator * (const type & scalar) const;
  vMatrix<type> operator / (const type & scalar) const;

//  vMatrix<type> friend operator +<type> (const type & scalar, 
//                                    vMatrix<type> & matrix);
//  vMatrix<type> friend operator -<type> (const type & scalar, 
//                                    vMatrix<type> & matrix);
//  vMatrix<type> friend operator *<type> (const type & scalar, 
//                                    vMatrix<type> & matrix);
//  vMatrix<type> friend operator /<type> (const type & scalar, 
//                                    vMatrix<type> & matrix);
  
  general_image * Copy() const;
  void Copy(vMatrix * target) const;
  void Copy(vMatrix * target, const vint8 target_top, const vint8 target_left) const;
  void Copy(vMatrix * target, const vint8 top, const vint8 bottom, const vint8 left, const vint8 right,
            const vint8 target_top, const vint8 target_left) const;
  vMatrix * Add(const vMatrix<type> * b) const;
  void Add(const vMatrix<type> * b, vMatrix<type> * result) const;
  vMatrix * Sub(const vMatrix<type> * b) const;
  void Sub(const vMatrix<type> * b, vMatrix<type> * result) const;
  vMatrix * Mult(const vMatrix<type> * b) const;
  void Mult(const vMatrix<type> * b, vMatrix<type> * result) const;
  vMatrix * MultTranspose(const vMatrix<type> * b) const;
  void MultTranspose(const vMatrix<type> * b, vMatrix<type> * result) const;
  vMatrix * TransposeMult(const vMatrix<type> * b) const;
  void TransposeMult(const vMatrix<type> * b, vMatrix<type> * result) const;

  vMatrix<type> * Transpose() const;
  void Transpose(vMatrix<type> * target) const;
  vMatrix * Inverse() const;
  ushort Inverse(vMatrix * target) const;
    
  static ushort GaussJordan(vMatrix<type> * a, vMatrix<type> * b);

  static vMatrix * Identity(const vint8 in_rows);
  void Identity();
  static vMatrix I(const vint8 in_rows);

  static vMatrix Zero(const vint8 in_rows, const vint8 in_cols);

  // Returns a row vector consisting of the entries (low, low+1, ..., high).
  static vMatrix Range(const vint8 low, const vint8 high);

  // Returns a row vector consisting of the entries (low, low+1, ..., high).
  static vMatrix range(const vint8 low, const vint8 high)
  {
    return Range(low, high);
  }

  // Returns a row vector consisting of the entries 
  // (low, low+step, ..., low+(steps-1)*step).
  static vMatrix Range(const double low, const double step, const vint8 steps);

  static vMatrix range(const double low, const double step, const vint8 steps)
  {
    return Range(low, step, steps);
  }

  static vMatrix Read(const char * filename);

  static vMatrix read(const char * filename)
  {
    return Read(filename);
  }

  static vMatrix Read(FILE * fp);

  static vMatrix read(FILE * fp)
  {
    return Read(fp);
  }

  // lots of printing, for debugging
  static vMatrix read2(FILE * fp);

  static vMatrix<type> ReadText(const char * filename);

  static vMatrix<type> ReadText(FILE * fp);

  // copies the specified column into the output matrix
  // or output vector.
  vMatrix<type> vertical_line(const vint8 index) const;
  vint8 vertical_line(const vint8 index, std::vector<type> * output) const;

  // copies the specified row into the output matrix
  // or output vector.
  vMatrix<type> horizontal_line(const vint8 index) const;
  vint8 horizontal_line(const vint8 index, std::vector<type> * output) const;

  vMatrix<type> set_range(type lower, type upper) const;
};


//template<class type>
//class vMatrix : public vMatrix<type>
//{
//};




// Dot product between two vectors. Dimensions are not checked,
// as long as the matrices have the same size.
template<class type>
type vDot(const vMatrix<type> & m1_matrix, const vMatrix<type> & m2_matrix);

// Cross product between two vectors. Dimensions are not checked,
// as long as the size of each vector is 3.
template<class type>
vMatrix<type> vCross(const vMatrix<type> & m1_matrix, const vMatrix<type> & m2_matrix);

template<class type>
vMatrix<type> operator + (const type & scalar, const vMatrix<type> & matrix);

template<class type>
vMatrix<type> operator - (const type & scalar, const vMatrix<type> & matrix);

template<class type>
vMatrix<type> operator * (const type & scalar, const vMatrix<type> & matrix);

template<class type>
vMatrix<type> operator / (const type & scalar, const vMatrix<type> & matrix);


typedef vMatrix<uchar> standard_matrix;
typedef vMatrix<char> char_matrix;
typedef vMatrix<uchar> uchar_matrix;
typedef vMatrix<ushort> ushort_matrix;
typedef vMatrix<short> short_matrix;
typedef vMatrix<vint4> integer_matrix;
typedef vMatrix<vint8> vint8_matrix;
typedef vMatrix<float> float_matrix;
typedef vMatrix<double> double_matrix;



//#define VASSILIS_PROCESS_FILE
//#include "algebra.cpp"


#define VASSILIS_PROCESS_FILE
#include "algebra.cpp"

#ifdef VASSILIS_LINUX_PLATFORM
#define VASSILIS_PROCESS_FILE
#include "algebra.cpp"
#endif // VASSILIS_LINUX_PLATFORM

#endif // VASSILIS_ALGEBRA_H

