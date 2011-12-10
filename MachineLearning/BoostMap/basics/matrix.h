#ifndef VASSILIS_MATRIX_H
#define VASSILIS_MATRIX_H

#include <vector>

#include "auxiliaries.h"



// v3dMatrix is meant to be a generic 3D_array class, that can be used
// among other things to represent images, with arbitrary numbers of bands, 
// and arbitrary types for the representation of the color of each
// pixel in each  band.

// Note that whether char is the same as signed char or same as
// unsigned char is not specified in C++.
// enum Type {ScharType, ShortType, IntType, Vint8Type, FloatType, DoubleType,
//	   UcharType, UshortType, UintType, UlongType, OtherType};
// In the previous line I keep the original order. If I add more types later, I
// should make sure that the Type values when converted to vint8 give the same
// numbers as before. My code that reads and writes images to and from files
// depends on the assumption that the vint8 number equivalent to a given type
// never changes.

template<class type>
class v3dMatrix;

class color_image;
class grayscale_image;

class general_image : public class_unique
{
protected:
  Type type_name;
  vint8 rows, cols, bands, size, all_band_size;
  vint8 is_valid;

public:
  virtual ~general_image();
  inline vint8 valid() const
  {
    return is_valid;
  }

  inline vint8 Bands() const
  {
    return bands;
  }

  inline vint8 channels () const
  {
    return bands;
  }

  inline Type TypeName() const
  {
    return type_name;
  }

  inline vint8 Rows() const
  {
    return rows;
  }

  inline vint8 vertical () const
  {
    return rows;
  }

  inline vint8 Cols() const
  {
    return cols;
  }

  inline vint8 Columns() const
  {
    return cols;
  }

  inline vint8 horizontal () const
  {
    return cols;
  }

  inline vint8 Size() const // Size of one band, in pixels;
  {
    return size;
  }

  inline vint8 length() const // Size of one band, in pixels;
  {
    return size;
  }

  inline vint8 AllBandSize() const
  {
    return all_band_size;
  }

  inline vint8 all_channel_size() const
  {
    return all_band_size;
  }

  static inline vint8 HeaderSize()
  {
    return 4;
  }

  // number of bytes needed to store the image header to a file
  static inline vint8 header_bytes()
  {
    return HeaderSize() * sizeof(integer);
  }

  // True if row and col are coordinates of a valid pixel
  // in the image, false if they are out of bounds.
  inline vint8 check_bounds(vint8 row, vint8 col) const
  {
    if (row < 0) return 0;
    if (col < 0) return 0;
    if (row >= rows) return 0;
    if (col >= cols) return 0;
    return 1;
  }

  inline vint8 Index2(vint8 row, vint8 col) const
  {
    return (row * cols + col);
  }

  inline vint8 Index3(vint8 band, vint8 row, vint8 col) const
  {
    return (band * size + Index2(row, col));
  }

  // print some general information about the image: type name,
  // dimensions.
  vint8 print_header() const;

  virtual vint8 SizeOfType() const = 0;
  
  // creates a general image from raw data.  The raw data itself is copied,
  // so it can be deleted after this function has been called. We assume that the data
  // format is interleaved, row-by-row, without any padding.
  static general_image * from_data(void * data, vint8 channels, vint8 vertical, vint8 horizontal,
                                   vint8 bits_per_pixel);
  
  virtual ushort Write(const char * filename) const = 0;
  ushort store(const char * filename) const {  return Write(filename);  }
  virtual ushort Write(FILE * fp) const = 0;
  ushort store(FILE * fp) const {  return Write(fp);  }
  virtual ushort Write(class_file * fp) const = 0;
  ushort store(class_file * fp) const {  return Write(fp);  }
  virtual ushort WriteText(const char * filename) const = 0;
  ushort store_text(const char * filename) const {  return WriteText(filename);  }

  static general_image * Read(const char * filename);
  static general_image * read(const char * filename) {  return Read(filename);  }
  static general_image * Read(FILE * fp);
  static general_image * read(FILE * fp) {  return Read(fp);  }
  static general_image * Read(class_file * fp);
  static general_image * read(class_file * fp) {  return Read(fp);  }
  static general_image * ReadText(const char * filename);
  static general_image * read_text(const char * filename) {  return ReadText(filename);  }

  virtual general_image * SumBands() const = 0;
  general_image * sum_bands() const {  return SumBands();  }
  virtual v3dMatrix<vint8> * SumBandsVint8() const = 0;
  v3dMatrix<vint8> * sum_bands_vint8() const {  return SumBandsVint8();  }
  virtual v3dMatrix<double> * SumBandsDouble() const = 0;
  v3dMatrix<double> * sum_bands_double() {  SumBandsDouble();  }
  virtual general_image * AverageBands() const = 0;
  general_image * average_bands() {  return AverageBands();  }

  virtual general_image * BoundBands(vint8 lower, vint8 upper) const = 0;
  general_image * bound_bands(vint8 lower, vint8 upper) {  return BoundBands(lower, upper);  }
  virtual general_image * BoundBands(float lower, float upper) const = 0;
  general_image * bound_bands(float lower, float upper) {  return BoundBands(lower, upper);  }
  virtual general_image * BoundBands(double lower, double upper) const = 0;
  general_image * bound_bands(double lower, double upper) {  return BoundBands(lower, upper);  }

  // ToRGB and ToGray should be used when we know that 
  // the values in the image that we 
  // want to convert are already between 0 and 255
  // The functions ToGrayCopy and ToRGBCopy return a new image.
  // The functions ToRGB and ToGray return the object that 
  // they are called from, if it is indeed RGB or gray.
  virtual color_image * ToRGB() const = 0;
  virtual grayscale_image * ToGray() const = 0;
  virtual color_image * ToRGBCopy() const = 0;
  virtual color_image * color_copy() const = 0;
  virtual grayscale_image * ToGrayCopy() const = 0;
  virtual grayscale_image * grayscale_copy() const = 0;

  // ToRGBBound and ToGrayBound shift the values of the 
  // source image so that they are between
  // 0 and 255 before it converts to RGB
  virtual color_image * ToRGBBound() const = 0;
  virtual grayscale_image * ToGrayBound() const = 0;
  virtual grayscale_image * ToGrayBoundCopy() const = 0;
  virtual color_image * ToRGBBoundCopy() const = 0;
  virtual grayscale_image * grayscale_bound_copy() const = 0;
  virtual color_image * color_bound_copy() const = 0;

  // Copy returns a copy of this image
  virtual general_image * Copy() const = 0;
  virtual general_image * Copy(vint8 left, vint8 right, vint8 top, vint8 bottom) const = 0;
  inline general_image * copy() const
  {
    return Copy();
  }

  inline general_image * copy(vint8 left, vint8 right, vint8 top, vint8 bottom) const
  {
    return Copy (left, right, top, bottom);
  }

  virtual void DoubleValues(std::vector<double> & values, vint8 row, vint8 col) const = 0;
  virtual void DoubleValues(std::vector<double> & values, vint8 i) const = 0;
  virtual void Vint8Values(std::vector<vint8> & values, vint8 row, vint8 col) const = 0;
  virtual void Vint8Values(std::vector<vint8> & values, vint8 i) const = 0;
  virtual double double_value (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;
  virtual vint8 vint8_value (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;
  virtual double double_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;
  virtual vint8 vint8_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;
  virtual double double_value_strict (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;
  virtual vint8 vint8_value_strict (vint8 channel, vint8 vertical, vint8 horizontal) const = 0;

  // The following functions perform various image operations.
  virtual v3dMatrix<uchar> * threshold(double threshold) const = 0;
  virtual general_image * AverageRegions(vint8 win_rows, vint8 win_cols) const = 0;
  virtual general_image * Resample(vint8 rows2, vint8 cols2) const = 0;
  virtual general_image * Resample(vint8 top, vint8 left, vint8 rows1, vint8 cols1,
				 vint8 rows2, vint8 cols2) const = 0;

  // Resample2 takes as input the subimage defined by (top, left) as its top
  // left corner that has full_rows1 rows and full_cols1 columns. It creates
  // a result image in which the input subimage is subsampled (1 out of
  // every "scale" rows and columns survive). If scale doesn't divide 
  // exactly rows (or cols) the remainder of rows (or cols) of the input
  // image is ignored.
  virtual general_image * Resample2(vint8 top, vint8 left, 
                          vint8 full_rows1, vint8 full_cols1, vint8 scale) const = 0;

  virtual v3dMatrix<vint8> * ConnectedComponents(std::vector<Label> * label_objects) const = 0;

  // file position where information for the specific entry is stored
  virtual vint8 file_position(vint8 channel, vint8 vertical, vint8 horizontal) const = 0;

  // file position where information for the specific entry is stored
  // this function should only be called for 2D matrices (i.e., matrices
  // with one channel), otherwise the program will exit.
  virtual vint8 file_position(vint8 vertical, vint8 horizontal) const = 0;
};


template<class type>
class v3dMatrix : public general_image
{
protected:
  vArray3(type) matrix3; // Indexed by band, row, col.
  vArray2(type) matrix2; // Indexed by band, index 
                          // (index depends on row, col).
  vArray(type) matrix;   // Index depends on band, row, col.
  
  // The next two variables hold the maximum and minimum allowable
  // values for the type. This is useful in template functions where
  // we need to know these values (for example when we amplify an
  // image, and we want to use the whole range of allowable values).
  type max_possible_value;
  type min_possible_value;

  virtual void delete_unique();

  vArray2(type) ReorderBands(std::vector<vint8> * band_order) const;

  // The next function stores in band_order the sequence 0, 1, ..., bands-1
  void DefaultOrder(std::vector<vint8> & band_order) const;
  void ProcessType();
  void Initialize(vint8 in_rows, vint8 in_cols, vint8 in_bands);
  void initialize();

public:
  v3dMatrix();
  v3dMatrix(const vint8 in_rows, const vint8 in_cols, const vint8 bands);
  explicit v3dMatrix(const general_image * source);
  v3dMatrix(const general_image * source, vint8 channel);
  ~v3dMatrix();

  static v3dMatrix * from_data(void * data, vint8 channels, vint8 vertical, vint8 horizontal);

  // returns a three-dimensional matrix that provides access to all the data
  // in the image and is indexed by channel, row, column
  vArray3(type) Matrix3() const;

  // returns a two-dimensional matrix that provides access to the data 
  // stored in the i-th band (channel) and is 
  // indexed by row, column
  vArray2(type) Matrix2(vint8 i) const;

  // returns a two-dimensional matrix that provides access to the data of all bands (channels),
  // and is indexed by channel and index (where index is a single number
  // that depends on the row and column.
  vArray2(type) Matrix2() const;

  // returns a one-dimensional matrix that stores the data of channel i,
  // and is indexed by a single number
  vArray(type) Matrix(vint8 i) const;

  // returns a one-dimensional matrix indexed by a single number.
  vArray(type) Matrix() const;

  // these inline commands are simply renamed versions
  // (with names that are easy to dictate using speech recognition)
  // of the previous commands that provide access to the matrix data
  inline three_pointer(type) three_matrix() const
  {
    return Matrix3();
  }

  inline matrix_pointer(type) planar(const vint8 channel) const
  {
    return Matrix2(channel);
  }

  inline matrix_pointer(type) planar_safe(const vint8 channel) const
  {
    if ((channel >= 0) && (channel < bands) &&
        (predicate_zero(matrix3) == 0))
    {
      return Matrix2(channel);
    }
    else
    {
      return function_zero(class_pointer(type));
    }
  }

  inline matrix_pointer(type) planar() const
  {
    return Matrix2();
  }

  inline class_pointer(type) flat(const vint8 channel) const
  {
    return Matrix(channel);
  }

  inline class_pointer(type) flat() const
  {
    return Matrix();
  }

  
  short ReadInterlaced(const char * filename, vint8 offset) const;
  short ReadInterlaced(const char * filename, vint8 offset,
		                   std::vector<vint8> * band_order) const;

  short ReadInterlaced(FILE * fp) const;
  short ReadInterlaced(class_file * fp) const;
  short ReadInterlaced(FILE * fp, std::vector<vint8> * band_order) const;

  short ReadInterlaced(class_file * fp, std::vector<vint8> * band_order) const;

  short ReadBandwise(const char * filename, vint8 offset) const;
  short read_bandwise(const char * filename, vint8 offset) const 
  {  return ReadBandwise(filename, offset);  }
  short ReadBandwise(const char * filename, vint8 offset, 
	                  std::vector<vint8> * band_order) const;
  short read_bandwise(const char * filename, vint8 offset, 
	                    std::vector<vint8> * band_order) const
  {  return ReadBandwise(filename, offset, band_order);  }
  short ReadBandwise(FILE * fp) const;
  short read_bandwise(FILE * fp) const {  return ReadBandwise(fp);  }
  short ReadBandwise(class_file * fp) const;
  short read_bandwise(class_file * fp) const {  return ReadBandwise(fp);  }
  short ReadBandwise(FILE * fp, std::vector<vint8> * band_order) const;
  short ReadBandwise(class_file * fp, std::vector<vint8> * band_order) const;

  vint8 ReadTextBandwise(const char * filename, vint8 offset) const;
  vint8 ReadTextBandwise(FILE * fp) const;
  vint8 ReadTextBandwise(class_file * fp) const;

  static vint8 ReadHeader(FILE * fp, std::vector<vint8> * header);
  static vint8 ReadHeader(class_file * fp, std::vector<vint8> * header);
  static vint8 ReadHeaderText(FILE * fp, std::vector<vint8> * header);
  static vint8 ReadHeaderText(class_file * fp, std::vector<vint8> * header);

  ushort WriteInterlaced(FILE * fp) const;
  ushort WriteInterlaced(class_file * fp) const;
  ushort WriteInterlaced(FILE * fp, std::vector<vint8> * band_order) const;
  ushort WriteInterlaced(class_file * fp, std::vector<vint8> * band_order) const;
  ushort WriteBandwise(FILE * fp) const;
  ushort store_bandwise(FILE * fp) const {  return WriteBandwise(fp);  }
  ushort WriteBandwise(class_file * fp) const;
  ushort store_bandwise(class_file * fp) const {  return WriteBandwise(fp);  }
  ushort WriteBandwise(FILE * fp, std::vector<vint8> * band_order) const;
  ushort WriteBandwise(class_file * fp, std::vector<vint8> * band_order) const;
  vint8 WriteHeader(FILE * fp) const;
  vint8 store_header(FILE * fp) const {  return WriteHeader(fp);  }
  vint8 WriteHeader(class_file * fp) const;
  vint8 store_header(class_file * fp) const {  return WriteHeader(fp);  }
  vint8 WriteHeaderText(FILE * fp) const;
  vint8 WriteHeaderText(class_file * fp) const;
  
  static vint8 WriteHeader(FILE * fp, std::vector<vint8> * header);
  static vint8 store_header(FILE * file_pointer, std::vector<vint8> * header)
  {
    return WriteHeader(file_pointer, header);
  }

  static vint8 WriteHeader(class_file * fp, std::vector<vint8> * header);
  static vint8 store_header(class_file * file_pointer, std::vector<vint8> * header)
  {
    return WriteHeader(file_pointer, header);
  }

  static vint8 WriteHeaderText(FILE * fp, std::vector<vint8> * header);
  static vint8 store_header_text(FILE * file_pointer, std::vector<vint8> * header)
  {
    return WriteHeaderText(file_pointer, header);
  }

  static vint8 WriteHeaderText(class_file * fp, std::vector<vint8> * header);
  static vint8 store_header_text(class_file * file_pointer, std::vector<vint8> * header)
  {
    return WriteHeaderText(file_pointer, header);
  }

  type MinValue() const;
  type MaxValue() const;

  virtual vint8 SizeOfType() const;
  general_image * SumBands() const;
  v3dMatrix<vint8> * SumBandsVint8() const;
  v3dMatrix<double> * SumBandsDouble() const;
  general_image * AverageBands() const;

  general_image * BoundBands(vint8 lower, vint8 upper) const;
  general_image * BoundBands(float lower, float upper) const;
  general_image * BoundBands(double lower, double upper) const;
  v3dMatrix * set_range(type lower, type upper);
  vint8 set_range(type lower, type upper, v3dMatrix * target);

  color_image * ToRGB() const;
  grayscale_image * ToGray() const;
  color_image * ToRGBCopy() const;
  grayscale_image * ToGrayCopy() const;
  
  // Returns a copy of this image.
  general_image * Copy() const;
  general_image * Copy(vint8 left, vint8 right, vint8 top, vint8 bottom) const;
  color_image * ToRGBBound() const;
  grayscale_image * ToGrayBound() const;
  color_image * ToRGBBoundCopy() const;
  grayscale_image * ToGrayBoundCopy() const;
  grayscale_image * grayscale_copy() const;
  color_image * color_copy() const;
  grayscale_image * grayscale_bound_copy() const;
  grayscale_image * fast_grayscale_bound_copy() const;
  color_image * color_bound_copy() const;

  void DoubleValues(std::vector<double> & values, vint8 row, vint8 col) const;
  void DoubleValues(std::vector<double> & values, vint8 i) const;
  void Vint8Values(std::vector<vint8> & values, vint8 row, vint8 col) const;
  void Vint8Values(std::vector<vint8> & values, vint8 i) const;
  virtual double double_value (vint8 channel, vint8 vertical, vint8 horizontal) const;
  virtual vint8 vint8_value (vint8 channel, vint8 vertical, vint8 horizontal) const;
  virtual double double_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const;
  virtual vint8 vint8_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const;
  virtual double double_value_strict (vint8 channel, vint8 vertical, vint8 horizontal) const;
  virtual vint8 vint8_value_strict (vint8 channel, vint8 vertical, vint8 horizontal) const;

  // The following functions perform various image operations.
  v3dMatrix<uchar> * threshold(double threshold) const;
  general_image * AverageRegions(vint8 win_rows, vint8 win_cols) const;
  general_image * Resample(vint8 top, vint8 left, vint8 rows1, vint8 cols1,
			 vint8 rows2, vint8 cols2) const;
  general_image * Resample(vint8 rows2, vint8 cols2) const;

  // Resample2 takes as input the subimage defined by (top, left) as its top
  // left corner that has full_rows1 rows and full_cols1 columns. It creates
  // a result image in which the input subimage is subsampled (1 out of
  // every "scale" rows and columns survive). If scale doesn't divide 
  // exactly rows (or cols) the remainder of rows (or cols) of the input
  // image is ignored.
  virtual general_image * Resample2(vint8 top, vint8 left, 
                          vint8 full_rows1, vint8 full_cols1, vint8 scale) const;

  v3dMatrix<vint8> * ConnectedComponents(std::vector<Label> * label_objects) const;
  
  static v3dMatrix * Read(const char * filename);
  static v3dMatrix * read(const char * filename)
  {
    return Read(filename);
  }

  static v3dMatrix * Read(FILE * fp);
  static v3dMatrix * read(FILE * fp)
  {
    return Read(fp);
  }

  static v3dMatrix * Read(class_file * fp);
  static v3dMatrix * read(class_file * file_pointer)
  {
    return Read(file_pointer);
  }

  static v3dMatrix * ReadText(const char * filename);
  static v3dMatrix * read_text(const char * filename)
  {
    return ReadText(filename);
  }

  static v3dMatrix * ReadText(FILE * fp);
  static v3dMatrix * read_text(FILE * fp)
  {
    return ReadText(fp);
  }

  static v3dMatrix * ReadText(class_file * fp);
  static v3dMatrix * read_text(class_file file_pointer)
  {
    return ReadText(file_pointer);
  }

  ushort Write(const char * filename) const;
  ushort Write(FILE * fp) const;
  ushort Write(class_file * fp) const;
  ushort WriteText(const char * filename) const;
  ushort WriteText(FILE * fp) const;
  ushort WriteText(class_file * fp) const;
  inline ushort store_text(FILE * fp) const
  {
    return WriteText(fp);
  }
  inline ushort store_text(class_file * fp) const
  {
    return WriteText(fp);
  }

  // WriteDebug is useful for saving matrices while debugging, at
  // a pre-specified directory. The first application of this
  // function was in debugging the shape context code: I saved
  // (while debugging) matrices to a file, and then I read them
  // from matlab and compared them to the matlab equivalents.
  vint8 WriteDebug(const char * filename) const;

  // Prints interlaced.
  vint8 Print(const char * text) const;

  // Prints bandwise.
  vint8 Print2(const char * text) const;

  // Referencing items
  inline type & operator () (vint8 band, vint8 row, vint8 col) const
  {
    assert((band >= 0) && (band < bands) &&
           (row >= 0) && (row < rows) &&
           (col >= 0) && (col < cols));
    return matrix3[band][row][col];         
  }

  inline type & operator () (vint8 band, vint8 row, vint8 col)
  {
    assert((band >= 0) && (band < bands) &&
           (row >= 0) && (row < rows) &&
           (col >= 0) && (col < cols));
    return matrix3[band][row][col];         
  }

  inline type & operator () (vint8 band, vint8 index) const
  {
    assert((band >= 0) && (band < bands) &&
           (index >= 0) && (index < size));
    return matrix2[band][index];         
  }
  
  inline type & operator () (vint8 band, vint8 index)
  {
    assert((band >= 0) && (band < bands) &&
           (index >= 0) && (index < size));
    return matrix2[band][index];         
  }
  
  inline type & operator () (vint8 index) const
  {
    assert((index < this->all_band_size) && (index >= 0));
    return this->matrix[index];
  }
  inline type & operator () (vint8 index)
  {
    assert((index < this->all_band_size) && (index >= 0));
    return this->matrix[index];
  }

  // file position where information for the specific entry is stored
  virtual vint8 file_position(vint8 channel, vint8 vertical, vint8 horizontal) const;

  // file position where information for the specific entry is stored
  // this function should only be called for 2D matrices (i.e., matrices
  // with one channel), otherwise the program will exit.
  virtual vint8 file_position(vint8 vertical, vint8 horizontal) const;

  // file position where information for the specific entry (channel, row, column) is stored
  // rows and cols are the size of the matrix stored in the file.
  static vint8 file_position(vint8 rows, vint8 cols, vint8 channel, vint8 row, vint8 column);
};
  

ushort predicate_color_image(general_image * base);

ushort predicate_gray_image(general_image * base);


// This class implements a four-dimensional array, with some basic
// functionality: We can create it, delete it, write it to file,
// read it from file. For each dimension, the index range doesn't
// have to start at 0, it can start below zero or above zero.
//
// Note: The function vDebug_4D_Array contains a small interface
// for verifying that the class implementation works correctly.
template<class type>
class v4dArray
{
private:
  // hidden_data4 is an array where every dimension's
  // index range starts at zero.
  vArray4(type) hidden_data4;
  vArray3(type) hidden_data3;
  vArray2(type) hidden_data2;
  vArray(type) hidden_data;

  // data4 is the array from which the data will be read.
  // It points to data in hidden_data, but it adjusts
  // the indices to make up for the index ranges of 
  // all dimensions of the array.
  vArray4(type) data4;
//  vArray3(type) data3;
//  vArray2(type) data2;
//  vArray1(type) data1;

  vint8 dim1, dim2, dim3, dim4;
  vint8 dim1_low, dim1_high, dim2_low, dim2_high;
  vint8 dim3_low, dim3_high, dim4_low, dim4_high;
  vint8 size;

  vint8 ZeroArrays();
  vint8 Initialize();
  vint8 DeleteArrays();

public:
  v4dArray(vint8 in_dim1, vint8 in_dim2, vint8 in_dim3, vint8 in_dim4);
  v4dArray(vint8 in_dim1_low, vint8 in_dim1_high,
             vint8 in_dim2_low, vint8 in_dim2_high,
             vint8 in_dim3_low, vint8 in_dim3_high,
             vint8 in_dim4_low, vint8 in_dim4_high);

  ~v4dArray();

  inline vint8 Dim1()
  {
    return dim1;
  }

  inline vint8 Dim2()
  {
    return dim2;
  }

  inline vint8 Dim3()
  {
    return dim3;
  }

  inline vint8 Dim4()
  {
    return dim4;
  }

  inline vint8 Dim1Low()
  {
    return dim1_low;
  }

  inline vint8 Dim2Low()
  {
    return dim2_low;
  }

  inline vint8 Dim3Low()
  {
    return dim3_low;
  }

  inline vint8 Dim4Low()
  {
    return dim4_low;
  }

  inline vint8 Dim1High()
  {
    return dim1_high;
  }

  inline vint8 Dim2High()
  {
    return dim2_high;
  }

  inline vint8 Dim3High()
  {
    return dim3_high;
  }

  inline vint8 Dim4High()
  {
    return dim4_high;
  }

  inline vArray4(type) Data4()
  {
    return data4;
  }

  inline vArray(type) AllData()
  {
    return hidden_data;
  }

  inline vint8 Size()
  {
    return dim1*dim2*dim3*dim4;
  }

  inline vint8 MemorySize()
  {
    return Size() * sizeof(type);
  }

  vint8 Write(const char * filename);

  vint8 Write(FILE * fp);

  static v4dArray * Read(const char * filename);

  static v4dArray * Read(FILE * fp);

  vint8 SkipHeader(FILE * fp);
  
  vint8 ReadData(FILE * fp);

  // Print is used for debugging
  vint8 Print();

  // Write 0 on all array positions.
  vint8 Zero();

  // Write value on all array positions.
  vint8 WriteValue(type value);
};


Type vSuperType(Type type);


// speech-recognition-friendly names for various classes


typedef v3dMatrix<uchar> standard_image;
typedef v3dMatrix<uchar> uchar_image;
typedef v3dMatrix<ushort> ushort_image;
typedef v3dMatrix<short> short_image;
typedef v3dMatrix<vint8> vint8_image;
typedef v3dMatrix<vint4> integer_image;
typedef v3dMatrix<ulong> uvint8_image;
typedef v3dMatrix<float> float_image;
typedef v3dMatrix<double> double_image;





#ifndef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#include "matrix.cpp"
#endif // VASSILIS_SGI_PLATFORM


#endif   // VASSILIS_MATRIX_H
