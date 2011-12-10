// I am doing something a bit tricky to make sure that I can use 
// the same matrix.cpp both on Windows and the SGI. There
// are two problems: One is
// that on Windows, for template files, I have to include the source file
// in the header file, whereas on the SGI I should not do that.
// Another problem is that I want to include matrix.cpp on the project files,
// but the actual source code should not get compiled, because it gets
// included on the header file. With the introductory compiler directives everything
// works.

#ifdef VASSILIS_PROCESS_FILE
#undef VASSILIS_PROCESS_FILE

#include "vplatform.h"
#include <algorithm>

#include <fstream>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pc_aux.h"

#include "auxiliaries.h"
#include "image.h"

#include "definitions.h"

using std::vector;


template<class type1, class type2>
void S_CopyBands(v3dMatrix<type1> * source, v3dMatrix<type2> * target)
{
  vArray(type1) matrix1 = source->Matrix();
  vArray(type2) matrix2 = target->Matrix();
  vint8 size = source->AllBandSize();
  if (target->AllBandSize() != size)
  {
    exit_error("S_CopyBands: number of bands error\n");
  }
  vint8 i;
  for (i = 0; i < size; i++)
    matrix2[i] = (type2) matrix1[i];
}


template<class type1, class type2>
void static_copy_band(v3dMatrix<type1> * source, vint8 source_channel, 
                      v3dMatrix<type2> * target, vint8 target_channel)
{
  class_pointer(type1) matrix1 = source->Matrix(source_channel);
  class_pointer(type2) matrix2 = target->Matrix(target_channel);
  vint8 size = source->Size();
  if (target->Size() != size)
  {
    exit_error("S_CopyBands: number of bands error\n");
  }
  vint8 i;
  for (i = 0; i < size; i++)
    matrix2[i] = (type2) matrix1[i];
}


template<class type>
void v3dMatrix<type>::initialize()
{
  rows = 0;
  cols = 0;
  bands = 0;
  matrix = vZero(type);
  matrix2 = vZero(vArray(type));
  matrix3 = vZero(vArray2(type));
  size = 0;
  all_band_size = 0;
  is_valid = 0;
}


template<class type>
v3dMatrix<type>::v3dMatrix()
{
  initialize();
}


template<class type>
v3dMatrix<type>::v3dMatrix(const general_image * source) 
{
  initialize();
  if ((source == 0) || (source->valid() <= 0))
  {
    return;
  }
  Initialize(source->Rows(), source->Cols(), source->Bands());
  Type source_type = source->TypeName();
  switch(source_type)
  {
  case ScharType:
    S_CopyBands((v3dMatrix<char> *) source, this);
    break;

  case ShortType:
    S_CopyBands((v3dMatrix<short> *) source, this);
    break;

  case IntType:
    S_CopyBands((v3dMatrix<int> *) source, this);
    break;

  case Vint8Type:
    S_CopyBands((v3dMatrix<vint8> *) source, this);
    break;

  case UcharType:
    S_CopyBands((v3dMatrix<uchar> *) source, this);
    break;

  case UshortType:
    S_CopyBands((v3dMatrix<ushort> *) source, this);
    break;

  case UintType:
    S_CopyBands((v3dMatrix<uint> *) source, this);
    break;

  case UlongType:
    S_CopyBands((v3dMatrix<ulong> *) source, this);
    break;
    
  case FloatType:
    S_CopyBands((v3dMatrix<float> *) source, this);
    break;
    
  case DoubleType:
    S_CopyBands((v3dMatrix<double> *) source, this);
    break;
    
  case OtherType:
    exit_error("OtherType case in v3dMatrix construction.\n");
    break;
  }   

  is_valid = 1;
}


template<class type>
v3dMatrix<type>::v3dMatrix(const general_image * source, vint8 channel)
{
  initialize();
  if ((source == 0) || (source->valid() <= 0))
  {
    return;
  }
  Initialize(source->Rows(), source->Cols(), 1);
  Type source_type = source->TypeName();
  switch(source_type)
  {
  case ScharType:
    static_copy_band((v3dMatrix<char> *) source, channel, this, 0);
    break;

  case ShortType:
    static_copy_band((v3dMatrix<short> *) source, channel, this, 0);
    break;

  case IntType:
    static_copy_band((v3dMatrix<int> *) source, channel, this, 0);
    break;

  case Vint8Type:
    static_copy_band((v3dMatrix<vint8> *) source, channel, this, 0);
    break;

  case UcharType:
    static_copy_band((v3dMatrix<uchar> *) source, channel, this, 0);
    break;

  case UshortType:
    static_copy_band((v3dMatrix<ushort> *) source, channel, this, 0);
    break;

  case UintType:
    static_copy_band((v3dMatrix<uint> *) source, channel, this, 0);
    break;

  case UlongType:
    static_copy_band((v3dMatrix<ulong> *) source, channel, this, 0);
    break;
    
  case FloatType:
    static_copy_band((v3dMatrix<float> *) source, channel, this, 0);
    break;
    
  case DoubleType:
    static_copy_band((v3dMatrix<double> *) source, channel, this, 0);
    break;
    
  case OtherType:
    exit_error("OtherType case in v3dMatrix construction.\n");
    break;
  }   

  is_valid = 1;
}



template<class type>
v3dMatrix<type>::v3dMatrix(const vint8 in_rows, const vint8 in_cols, const vint8 in_bands)
{
  initialize();
  if ((in_rows <= 0) || (in_cols <= 0) || (in_bands <= 0))
  {
    return;
  }

  Initialize(in_rows, in_cols, in_bands);
  is_valid = 1;
}


// creates a general image from raw data.  The raw data itself is copied,
// so it can be deleted after this function has been called. We assume that the data
// format is interleaved, row-by-row, without any padding.
template<class type>
v3dMatrix<type> * v3dMatrix<type>::from_data(void * argument_data, vint8 channels, vint8 vertical_size, vint8 horizontal_size)
{
  type * data = (type *) argument_data;
  v3dMatrix<type> * result = new v3dMatrix<type>(vertical_size, horizontal_size, channels);
  vArray3(type) matrix = result->Matrix3();

  vint8 vertical, horizontal, channel;
  vint8 index = 0;

  for (vertical = 0; vertical < vertical_size; vertical++)
  {
    for (horizontal = 0; horizontal < horizontal_size; horizontal++)
    {
      for (channel = 0; channel < channels; channel++)
      {
        matrix[channel][vertical][horizontal] = data[index];
        index++;
      }
    }
  }

  return result;
}


template<class type>
void v3dMatrix<type>::Initialize(const vint8 in_rows, const vint8 in_cols, const vint8 in_bands)
{
  rows = in_rows;
  cols = in_cols;
  bands = in_bands;
  size = rows * cols;
  all_band_size = rows * cols * bands;
  ProcessType(); // Set all variables that store information about the type.

  vint8 band, row;
  matrix = vnew(type, (vector_size) all_band_size);

  matrix2 = vnew(vArray(type), (vector_size) bands);
  for (band = 0; band < bands; band++)
  {
    matrix2[band] = matrix + band * size;
  }

  matrix3 = vnew(vArray2(type), (vector_size) bands);
  for (band = 0; band < bands; band++)
  {
    matrix3[band] = vnew(vArray(type), (vector_size) rows);
    for (row = 0; row < rows; row++)
    {
      matrix3[band][row] = matrix2[band] + row * cols;
    }
  }
}


template<class type>
v3dMatrix<type>::~v3dMatrix()
{
  //AfxMessageBox("~v3dMatrix called");
  remove_reference();
}


template<class type>
void v3dMatrix<type>::delete_unique()
{
  //AfxMessageBox("V_3D_Matrix::DeleteReference called");
  vdelete2(matrix);
  vdelete2(matrix2);
  vDelete2(matrix3, bands);
}


template<class type>
vint8 v3dMatrix<type>::SizeOfType() const
{
  return sizeof(type);
}


// returns a three-dimensional matrix that provides access to all the data
// in the image and is indexed by channel, row, column
template<class type>
vArray3(type) v3dMatrix<type>::Matrix3() const
{
  return matrix3;
}


// returns a two-dimensional matrix that provides access to the data 
// stored in the i-th band (channel) and is 
// indexed by row, column
template<class type>
vArray2(type) v3dMatrix<type>::Matrix2(vint8 i) const
{
  if ((i >= bands) || (i < 0))
  {
    exit_error("channel = %li, bands = %li\n", i, bands);
  }
  return matrix3[i];
}


// returns a two-dimensional matrix that provides access to the data of all bands (channels),
// and is indexed by channel and index (where index is a single number
// that depends on the row and column.
template<class type>
vArray2(type) v3dMatrix<type>::Matrix2() const
{
  return matrix2;
}


// returns a one-dimensional matrix that stores the data of channel i,
// and is indexed by a single number
template<class type>
vArray(type) v3dMatrix<type>::Matrix(vint8 i) const
{
  if ((i >= bands) || (i < 0))
  {
    exit_error("channel = %li, bands = %li\n", i, bands);
  }
  return matrix2[i];
}


// returns a one-dimensional matrix indexed by a single number.
template<class type>
vArray(type) v3dMatrix<type>::Matrix() const
{
  return matrix;
}


template<class type>
vArray2(type) v3dMatrix<type>::ReorderBands(vector<vint8> * band_order) const
{
  vArray2(type) new_matrix = vnew(vArray(type), band_order->size());
  vint8 i;
  for (i = 0; i < (vint8) band_order->size(); i++)
    new_matrix[i] = matrix2[(*band_order)[(vector_size) i]];
  return new_matrix;
}


template<class type>
void v3dMatrix<type>::DefaultOrder(vector<vint8> & band_order) const
{
  band_order.erase(band_order.begin(), band_order.end());
  vint8 i;
  for (i = 0; i < bands; i++)
    band_order.push_back(i);
}

// This function reads in pixel data from a file. We assume that the pixel
// data starts at "offset" bytes from the beginning of the file (offset is
// not zero, for example, in PPM files), and then, going from top to bottom
// and left to right, we read for each pixel the value for all bands before
// we go to the next pixel. We also assume that the values for the bands appear
// in the same order in which we have ordered the bands in the image.
template<class type>
short v3dMatrix<type>::ReadInterlaced(const char * filename, vint8 offset) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadInterlaced(filename, offset, &band_order);
}


template<class type>
short  v3dMatrix<type>::ReadInterlaced(const char * filename, vint8 offset,
				                                 vector<vint8> * band_order) const
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  fseek(fp, (long) offset, SEEK_SET);
  short result = ReadInterlaced(fp, band_order);
  fclose(fp);
  return result;
}


template<class type>
short v3dMatrix<type>::ReadInterlaced(FILE * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadInterlaced(fp, band_order);
}


template<class type>
short v3dMatrix<type>::ReadInterlaced(class_file * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadInterlaced(fp, band_order);
}


template<class type>
short v3dMatrix<type>::ReadInterlaced(FILE * fp, vector<vint8> * band_order) const
{
  vArray(type) buffer = vnew(type, (vector_size) all_band_size);
  vArray2(type) new_matrix = ReorderBands(band_order);

  short result;
  vint8 bytes_read = fread(vData(buffer), sizeof(type), (long) all_band_size, fp);
  if (bytes_read == all_band_size * sizeof(type))
  {
    result = 1;
  }
  else
  {
    result = 0;
  }
  
  vint8 buffer_index = 0;
  vint8 band_index = 0;
  vint8 i;
  while(buffer_index < all_band_size)
  {
    for (i = 0; i < bands; i++)
      new_matrix[i][band_index] = buffer[buffer_index++];
    band_index++;
  }

  result = 1;

  vdelete2(buffer);
  vdelete2(new_matrix);
  return result;
}


template<class type>
short v3dMatrix<type>::ReadInterlaced(class_file * fp, vector<vint8> * band_order) const
{
  vArray(type) buffer = vnew(type, all_band_size);
  vArray2(type) new_matrix = ReorderBands(band_order);

  short result;
  vint8 bytes_read = fread(vData(buffer), sizeof(type), all_band_size, fp);
  if (bytes_read == all_band_size * sizeof(type))
  {
    result = 1;
  }
  else
  {
    result = 0;
  }
  
  vint8 buffer_index = 0;
  vint8 band_index = 0;
  vint8 i;
  while(buffer_index < all_band_size)
  {
    for (i = 0; i < bands; i++)
      new_matrix[i][band_index] = buffer[buffer_index++];
    band_index++;
  }

  result = 1;

  vdelete2(buffer);
  vdelete2(new_matrix);
  return result;
}


// This function reads in pixel data from a file. We assume that the pixel
// data starts at "offset" bytes from the beginning of the file (offset is
// not zero, for example, in PPM files), and then, going from top to bottom
// and left to right, we read for each band all the pixel values before
// we go to the next band. We also assume that the values for the bands appear
// in the same order in which we have ordered the bands in the image.
template<class type>
short v3dMatrix<type>::ReadBandwise(const char * filename, vint8 offset) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadBandwise(filename, offset, &band_order);
}


template<class type>
short v3dMatrix<type>::ReadBandwise(const char * filename, vint8 offset,
				                              vector<vint8> * band_order) const
{
  if ((vint8) band_order->size() > bands)
  {
    exit_error("too many bands in band_order\n");
  }

  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  fseek(fp, (long) offset, SEEK_SET);
  short result = ReadBandwise(fp, band_order);
  fclose(fp);
  return result;
}


template<class type>
short v3dMatrix<type>::ReadBandwise(FILE * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadBandwise(fp, &band_order);
}


template<class type>
short v3dMatrix<type>::ReadBandwise(class_file * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return ReadBandwise(fp, &band_order);
}


template<class type>
short v3dMatrix<type>::ReadBandwise(FILE * fp, vector<vint8> * band_order) const
{
  if (band_order->size() >  bands) return 0;
  vArray2(type) new_matrix = ReorderBands(band_order);

  short result = 1;
  for (int i = 0; i < (vint8) band_order->size(); i++)
  {
    vint8 bytes_read = fread(vData(new_matrix[i]), sizeof(type), (long) size, fp);
    if (bytes_read != size) 
    {
      result = 0;
    }
  }

  vdelete2(new_matrix);
  return result;
}



template<class type>
short v3dMatrix<type>::ReadBandwise(class_file * fp, vector<vint8> * band_order) const
{
  if (band_order->size() > (ulong) bands) return 0;
  vArray2(type) new_matrix = ReorderBands(band_order);

  short result = 1;
  for (int i = 0; i < (vint8) band_order->size(); i++)
  {
    vint8 bytes_read = fread(vData(new_matrix[i]), sizeof(type), (long) size, fp);
    if (bytes_read != size) 
    {
      result = 0;
    }
  }

  vdelete2(new_matrix);
  return result;
}



template<class type>
ushort v3dMatrix<type>::WriteInterlaced(FILE * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return WriteInterlaced(fp, &band_order);
}


template<class type>
ushort v3dMatrix<type>::WriteInterlaced(class_file * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return WriteInterlaced(fp, &band_order);
}


template<class type>
ushort v3dMatrix<type>::WriteInterlaced(FILE * fp,
			                                		vector<vint8> * band_order) const
{
  vArray2(type) new_matrix = ReorderBands(band_order);
  vArray(type) buffer = vnew(type, (vector_size) all_band_size);
  vint8 buffer_index = 0;
  vint8 band_index;
  vint8 i;
  for (band_index = 0; band_index < size; band_index++)
  {
    for (i = 0; i < bands; i++)
      buffer[buffer_index++] = new_matrix[i][band_index];
  }

  vint8 bytes = fwrite(vData(buffer), sizeof(type), (long) all_band_size, fp);
  ushort result;
  if (bytes != all_band_size * sizeof(type)) result = 0;
  else result = 1;

  vdelete2(buffer);
  vdelete2(new_matrix);
  return result;
}


template<class type>
ushort v3dMatrix<type>::WriteInterlaced(class_file * fp,
			                                		vector<vint8> * band_order) const
{
  vArray2(type) new_matrix = ReorderBands(band_order);
  vArray(type) buffer = vnew(type, all_band_size);
  vint8 buffer_index = 0;
  vint8 band_index;
  vint8 i;
  for (band_index = 0; band_index < size; band_index++)
  {
    for (i = 0; i < bands; i++)
      buffer[buffer_index++] = new_matrix[i][band_index];
  }

  vint8 bytes = fwrite(vData(buffer), sizeof(type), all_band_size, fp);
  ushort result;
  if (bytes != all_band_size * sizeof(type)) result = 0;
  else result = 1;

  vdelete2(buffer);
  vdelete2(new_matrix);
  return result;
}


template<class type>
ushort v3dMatrix<type>::WriteBandwise(FILE * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return WriteBandwise(fp, &band_order);
}


template<class type>
ushort v3dMatrix<type>::WriteBandwise(class_file * fp) const
{
  vector<vint8> band_order;
  DefaultOrder(band_order);
  return WriteBandwise(fp, &band_order);
}


template<class type>
ushort v3dMatrix<type>::WriteBandwise(FILE * fp,
				      vector<vint8> * band_order) const
{
  if (band_order->size() > (ulong) bands)
  {
    exit_error("too many bands in band_order\n");
  }

  vArray2(type) new_matrix = ReorderBands(band_order);

  vint8 i;
  vint8 result = 1;
  for (i = 0; i < (vint8) band_order->size(); i++)
  {
    vint8 items = fwrite(vData(new_matrix[i]), sizeof(type), (vector_size) size, fp);
    if (items != size) result = 0;
  }
  vdelete2(new_matrix);
  return (ushort) result;
}

template<class type>
ushort v3dMatrix<type>::WriteBandwise(class_file * fp,
				                              vector<vint8> * band_order) const
{
  if (band_order->size() > (ulong) bands)
  {
    exit_error("too many bands in band_order\n");
  }

  vArray2(type) new_matrix = ReorderBands(band_order);

  vint8 i;
  vint8 result = 1;
  for (i = 0; i < (vint8) band_order->size(); i++)
  {
    vint8 items = fwrite(vData(new_matrix[i]), sizeof(type), (vector_size) size, fp);
    if (items != size) result = 0;
  }
  vdelete2(new_matrix);
  return (ushort) result;
}

template<class type>
v3dMatrix<type> * v3dMatrix<type>::Read(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  v3dMatrix<type> * result = Read(fp);
  fclose(fp);
  return result;
}


template<class type>
v3dMatrix<type> * v3dMatrix<type>::Read(FILE * fp)
{
  integer rows, cols, bands, type_number;
  read_integer(fp, &type_number);
  read_integer(fp, &rows);
  read_integer(fp, &cols);
  read_integer(fp, &bands);

  v3dMatrix<type> * result = new v3dMatrix(rows, cols, bands);
  vint8 result_type = vTypeToVint8(result->TypeName());
  if (type_number != result_type) 
  {
    vdelete(result);
    return 0;
  }

  result->ReadBandwise(fp);
  return result;
}


template<class type>
v3dMatrix<type> * v3dMatrix<type>::Read(class_file * fp)
{
  integer rows, cols, bands, type_number;
  read_integer(fp, &type_number);
  read_integer(fp, &rows);
  read_integer(fp, &cols);
  read_integer(fp, &bands);

  v3dMatrix<type> * result = new v3dMatrix(rows, cols, bands);
  vint8 result_type = vTypeToVint8(result->TypeName());
  if (type_number != result_type) 
  {
    vdelete(result);
    return 0;
  }

  result->ReadBandwise(fp);
  return result;
}


template<class type>
ushort v3dMatrix<type>::Write(const char * filename) const
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0) return 0;
  ushort result = Write(fp);
  fclose(fp);
  return result;
}


template<class type>
ushort v3dMatrix<type>::Write(FILE * fp) const
{
  vint8 type_number = vTypeToVint8(type_name);

  store_integer(fp, (integer) type_number);
  store_integer(fp, (integer) rows);
  store_integer(fp, (integer) cols);
  store_integer(fp, (integer) bands);
  return WriteBandwise(fp);
}


template<class type>
ushort v3dMatrix<type>::Write(class_file * fp) const
{
  vint8 type_number = vTypeToVint8(type_name);

  store_integer(fp, (integer) type_number);
  store_integer(fp, (integer) rows);
  store_integer(fp, (integer) cols);
  store_integer(fp, (integer) bands);
  return WriteBandwise(fp);
}


template<class type>
vint8 v3dMatrix<type>::WriteDebug(const char * filename) const
{
  vint8 success;
  
  if (type_name == FloatType)
  {
    success = Write(filename);
  }
  else
  {
    v3dMatrix<float> temp(this);
    success = temp.Write(filename);
  }
  return success;
}

template<class type>
v3dMatrix<type> * v3dMatrix<type>::ReadText(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  v3dMatrix<type> * result = ReadText(fp);
  fclose(fp);
  return result;
}


template<class type>
v3dMatrix<type> * v3dMatrix<type>::ReadText(FILE * fp)
{
  integer type_number = 0, rows = 0, cols = 0, bands = 0;
  vint8 items = fscanf(fp, "%li %li %li %li", &type_number, &rows, &cols, &bands);
  if (items != 4) return 0;
  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  if (type_number != (vint8) result->TypeName())
  {
    // Some conversions are OK. For a start, reading a double matrix as a float
    // or vice versa is considered OK.
    if ((type_number == (vint8) DoubleType) && 
        (result->TypeName() == FloatType))
    {
      // Do nothing
    }
    else if ((type_number == (vint8) FloatType) && 
             (result->TypeName() == DoubleType))
    {
      // Do nothing
    }
    else
    {
      exit_error("type_number = %li instead of %li\n", type_number,
                      (vint8) result->TypeName());
    }
  }
  vint8 success = result->ReadTextBandwise(fp);
  if (success <= 0)
  {
    vdelete(result);
    return 0;
  }
  return result;
}


template<class type>
v3dMatrix<type> * v3dMatrix<type>::ReadText(class_file * fp)
{
  exit_error("error: unimplemented ReadText(class_file * fp)\n");
  integer type_number = 0, rows = 0, cols = 0, bands = 0;
  vint8 items = fscanf(fp->file_pointer, "%li %li %li %li", &type_number, &rows, &cols, &bands);
  if (items != 4) return 0;
  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, bands);
  if (type_number != (vint8) result->TypeName())
  {
    // Some conversions are OK. For a start, reading a double matrix as a float
    // or vice versa is considered OK.
    if ((type_number == (vint8) DoubleType) && 
        (result->TypeName() == FloatType))
    {
      // Do nothing
    }
    else if ((type_number == (vint8) FloatType) && 
             (result->TypeName() == DoubleType))
    {
      // Do nothing
    }
    else
    {
      exit_error("type_number = %li instead of %li\n", type_number,
                      (vint8) result->TypeName());
    }
  }
  vint8 success = result->ReadTextBandwise(fp);
  if (success <= 0)
  {
    vdelete(result);
    return 0;
  }
  return result;
}


template<class type>
vint8 v3dMatrix<type>::ReadTextBandwise(const char * filename, vint8 offset) const
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  fseek(fp, (long) offset, SEEK_SET);
  vint8 result = ReadTextBandwise(fp);
  fclose(fp);
  return result;
}


template<class type>
vint8 v3dMatrix<type>::ReadTextBandwise(FILE * fp) const
{
  vint8 i;
  double temp = 0;
  vint8 success = 1;
  for (i = 0; i < all_band_size; i++)
  {
    vint8 items_matched = fscanf(fp, "%lf", &temp);
    if (items_matched != 1) 
    {
      success = 0;
    }
    matrix[i] = (type) temp;
  }
  return success;
}


template<class type>
vint8 v3dMatrix<type>::ReadTextBandwise(class_file * fp) const
{
  exit_error("error: unimplemented ReadText(class_file * fp)\n");
  vint8 i;
  double temp = 0;
  vint8 success = 1;
  for (i = 0; i < all_band_size; i++)
  {
    vint8 items_matched = fscanf(fp->file_pointer, "%lf", &temp);
    if (items_matched != 1) 
    {
      success = 0;
    }
    matrix[i] = (type) temp;
  }
  return success;
}


template<class type>
vint8 v3dMatrix<type>::ReadHeader(FILE * fp, vector<vint8> * header)
{
  const vint8 header_size = 4;
  integer buffer[header_size];
  vint8 items = read_integers(fp, buffer, header_size);
  if (items != header_size) 
  {
    return 0;
  }

  vint8 i;
  for (i = 0; i < header_size; i++)
  {
    header->push_back(buffer[i]);
  }

  return 1;
}


template<class type>
vint8 v3dMatrix<type>::ReadHeader(class_file * fp, vector<vint8> * header)
{
  const vint8 header_size = 4;
  integer buffer[header_size];
  vint8 items = read_integers(fp, buffer, header_size);
  if (items != header_size) 
  {
    return 0;
  }

  vint8 i;
  for (i = 0; i < header_size; i++)
  {
    header->push_back(buffer[i]);
  }

  return 1;
}


template<class type>
vint8 v3dMatrix<type>::ReadHeaderText(FILE * fp, vector<vint8> * header)
{
  vint8 type_number = 0, rows = 0, cols = 0, bands = 0;
  vint8 items = fscanf(fp, "%li %li %li %li", &type_number, &rows, &cols, &bands);
  if (items != 4) 
  {
    return 0;
  }

  header->push_back(type_number);
  header->push_back(rows);
  header->push_back(cols);
  header->push_back(bands);
  return 1;
}


template<class type>
vint8 v3dMatrix<type>::ReadHeaderText(class_file * fp, vector<vint8> * header)
{
  exit_error("error: unimplemented ReadText(class_file * fp)\n");
  vint8 type_number = 0, rows = 0, cols = 0, bands = 0;
  vint8 items = fscanf(fp->file_pointer, "%li %li %li %li", &type_number, &rows, &cols, &bands);
  if (items != 4) 
  {
    return 0;
  }

  header->push_back(type_number);
  header->push_back(rows);
  header->push_back(cols);
  header->push_back(bands);
  return 1;
}


template<class type>
ushort v3dMatrix<type>::WriteText(const char * filename) const
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0) return 0;
  ushort result = WriteText(fp);
  fclose(fp);
  return result;
}


template<class type>
ushort v3dMatrix<type>::WriteText(FILE * fp) const
{
  //long r_aux,c_aux,b_aux;
  //r_aux = rows;
  //c_aux = cols;
  //b_aux = bands;

  fprintf(fp, "%li\n%li %li\n%li\n", (long)(vTypeToVint8(type_name)), (long)rows, (long)cols, (long)bands);
  //fprintf(fp, "%li\n%li %li\n%li\n", (vTypeToVint8(type_name)), rows, cols, bands);
   //fprintf(fp, "%li\n%li %li\n%li\n", (long)(vTypeToVint8(type_name)), r_aux, c_aux, b_aux);
  //fprintf(fp, "%li\n%li %li\n%li\n", (long)(vTypeToVint8(type_name)), 1, 3, 1);
  vint8 i;
  vint8 result = 1;
  for (i = 0; i < all_band_size; i++)
  {
    if (i % cols == 0) fprintf(fp, "\n");
    double temp = (double) matrix[i];
    vint8 fprintf_result = fprintf(fp, "%lf\n", temp);
    if (fprintf_result < 0) result = 0;
  }
  return (ushort) result;
}


template<class type>
ushort v3dMatrix<type>::WriteText(class_file * fp) const
{
  exit_error("error: unimplemented ReadText(class_file * fp)\n");
  fprintf(fp->file_pointer, "%li\n%li %li\n%li\n", vTypeToVint8(type_name), rows, cols, bands);
  vint8 i;
  vint8 result = 1;
  for (i = 0; i < all_band_size; i++)
  {
    if (i % cols == 0) fprintf(fp->file_pointer, "\n");
    double temp = matrix[i];
    vint8 fprintf_result = fprintf(fp->file_pointer, "%lf\n", temp);
    if (fprintf_result < 0) result = 0;
  }
  return (ushort) result;
}


// This works assuming that type is one of the predefined integer or
// real types in C++.
// It is a bit trickier to distinguish between an int and a vint8. In
// most architectures I know they are the same, anyway. 
template<class type>
void v3dMatrix<type>::ProcessType()
{
  // Check if integer or real type
  if (0.5 != (type) 0.5)
  {
    // It is integer, check if signed or unsigned
    if (0 > (type) -1)
    {
      // It is signed integer
      if (sizeof(type) == sizeof(signed char)) 
      {
      	type_name = ScharType;
      }
      else if (sizeof(type) == sizeof(short)) 
      {
	      type_name = ShortType;
      }
      else if (sizeof(type) == sizeof(vint8)) 
      {
	      type_name = Vint8Type;
      }
      else if (sizeof(type) == sizeof(int)) 
      {
	      type_name = IntType;
      }
      else 
      {
	      type_name = OtherType;
      }
    }
    else
    {
      // it is unsigned integer
      if (sizeof(type) == sizeof(uchar)) 
      {
	      type_name = UcharType;
      }
      else if (sizeof(type) == sizeof(ushort)) 
      {
	      type_name = UshortType;
      }
      else if (sizeof(type) == sizeof(unsigned long)) 
      {
	      type_name = UlongType;
      }
      else if (sizeof(type) == sizeof(unsigned int)) 
      {
	      type_name = UintType;
      }
      else 
      {
	      type_name = OtherType;
      }
    }
  }
  else
  {
    // it is real
    if (sizeof(type) == sizeof(float)) 
    {
      type_name = FloatType;
    }
    else if (sizeof(type) == sizeof(double)) 
    {
      type_name = DoubleType;
    }
    else 
    {
      type_name = OtherType;
    }
  }

  // Here we pass max_possible_value as an argument just to tell the
  // compiler which template to use.
  max_possible_value = vTypeMax(max_possible_value);
  min_possible_value = vTypeMin(min_possible_value);
}


template<class type>
general_image * v3dMatrix<type>::SumBands() const
{
  Type super_type = vSuperType(type_name);
  if ((super_type == Vint8Type) || (super_type == UlongType))
    return SumBandsVint8();
  else 
  {
    if (super_type != DoubleType)
    {
      exit_error("super_type is not DoubleType in SumBands\n");
    }
    return SumBandsDouble();
  }
}

template<class type1, class type2>
static void S_SumBands(const v3dMatrix<type1> * source,
		       v3dMatrix<type2> * result)
{
  vArray(type2) result_matrix = result->Matrix(0);
  vint8 size = source->Size();
  vint8 i;
  for (i = 0; i < size; i++)
    result_matrix[i] = 0;
  vint8 band;
  vint8 bands = source->Bands();
  for (band = 0; band < bands; band++)
  {
    vArray(type1) source_matrix = source->Matrix(band);
    for (i = 0; i < size; i++)
    {
      result_matrix[i] += (type2) source_matrix[i];
    }
  }
}


template<class type>
v3dMatrix<vint8> * v3dMatrix<type>::SumBandsVint8() const
{
  v3dMatrix<vint8> * result = new v3dMatrix<vint8>(rows, cols, 1);
  S_SumBands(this, result);
  return result;
}


template<class type>
v3dMatrix<double> * v3dMatrix<type>::SumBandsDouble() const
{
  v3dMatrix<double> * result = new v3dMatrix<double>(rows, cols, 1);
  S_SumBands(this, result);
  return result;
}


template<class type1, class type2>
void S_DivideImage(v3dMatrix<type1> * source, v3dMatrix<type2> * result,
		   double divisor)
{
  vArray(type1) matrix1 = source->Matrix();
  vArray(type2) matrix2 = result->Matrix();
  vint8 all_band_size = source->AllBandSize();
  if (result->AllBandSize() != all_band_size)
  {
    exit_error("source and result size disagree in S_DivideImage\n");
  }
  vint8 i;
  for (i = 0; i < all_band_size; i++)
    matrix2[i] = (type2) (((double) matrix1[i]) / divisor);
}


template<class type>
general_image * v3dMatrix<type>::AverageBands() const
{
  Type super_type = vSuperType(type_name);
  v3dMatrix<type> * result = new v3dMatrix<type>(rows, cols, 1);

  if ((super_type == Vint8Type) || (super_type == UlongType))
  {
    v3dMatrix<vint8> * sum = SumBandsVint8();
    S_DivideImage(sum, result, (double) bands);
    vdelete(sum);
  }
  else
  {
    if (super_type != DoubleType)
    {
      exit_error("super_type is not DoubleType in SumBands\n");
    }
    v3dMatrix<double> * sum = SumBandsDouble();
    S_DivideImage(sum, result, (double) bands);
    vdelete(sum);
  }
  return result;
}


template<class type>
type v3dMatrix<type>::MaxValue() const
{
  vint8 size = AllBandSize();
  vArray(type) values = Matrix();
  type max = values[0];

  for (vint8 i = 1; i < size; i++)
    if (values[i] > max) max = values[i];
  return max;
}
 
 
template<class type>
vint8 v3dMatrix<type>::WriteHeader(FILE * fp) const
{
  const vint8 header_size = 4;
  vint8 buffer[header_size];
  vint8 type_number = vTypeToVint8(type_name);

  vint8 index = 0;
  buffer[index++] = type_number;
  buffer[index++] = rows;
  buffer[index++] = cols;
  buffer[index++] = bands;

  vint8 items = store_vint8s(fp, buffer, header_size);
  if (items != header_size) 
  {
    return 0;
  }

  return 1;
}


template<class type>
vint8 v3dMatrix<type>::WriteHeaderText(FILE * fp) const
{
  vint8 type_number = vTypeToVint8(type_name);
  vint8 items = fprintf(fp, "%li %li %li %li", type_number, rows, cols, bands);
  if (items != 4) 
  {
    return 0;
  }

  return 1;
}


template<class type>
vint8 v3dMatrix<type>::WriteHeader(FILE * fp, vector<vint8> * header)
{
  vint8 header_size = header->size();
  vint8 i;
  vArray(integer) buffer = vnew(integer, (vector_size) header_size);
  for (i = 0; i < header_size; i++)
  {
    buffer[i] = (integer) (*header)[(vector_size) i];
  }

  vint8 items = store_integers(fp, actual_pointer(buffer), header_size);
  vdelete2(buffer);
  if (items != header_size) 
  {
    return 0;
  }

  return 1;
}


template<class type>
vint8 v3dMatrix<type>::WriteHeaderText(FILE * fp, vector<vint8> * header)
{
  vint8 items = fprintf(fp, "%li %li %li %li", 
                       (*header)[0], (*header)[1], (*header)[2], (*header)[3]);
  if (items != 4) 
  {
    return 0;
  }

  return 1;
}


template<class type>
type v3dMatrix<type>::MinValue() const
{
  vint8 size = AllBandSize();
  vArray(type) values = Matrix();
  type min = values[0];

  for (vint8 i = 1; i < size; i++)
    if (values[i] < min) min = values[i];
  return min;
}
  

template<class type1>
v3dMatrix<type1> * S_BoundBands(const v3dMatrix<type1> * source,
				  const double lower, const double upper)
{
  vArray(type1) matrix1 = source->Matrix();
  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  vint8 bands = source->Bands();
  v3dMatrix<type1> * result = new v3dMatrix<type1>(rows, cols, bands);
  vArray(type1) matrix2 = result->Matrix();
  vint8 size = source->AllBandSize();

  type1 min_value = source->MinValue();
  type1 max_value = source->MaxValue();
  double range = (max_value - min_value) * 1.000001;
  double offset = lower;
  double factor;
  if (range == 0.0)
  {
    factor = 1.0;
  }
  else
  {
    factor = (upper - lower) / range;
  }

  vint8 i;
  for (i = 0; i < size; i++)
  {
    double entry = (double) matrix1[i];
    matrix2[i] = (type1) ((entry - min_value) * factor + offset);
  }

  min_value = result->MinValue();
	max_value = result->MaxValue();
	return result;
}


template<class type1>
v3dMatrix<type1> * S_BoundBands(const v3dMatrix<type1> * source,
				                        const float lower, const float upper)
{
  vArray(type1) matrix1 = source->Matrix();
  vint8 rows = source->Rows();
  vint8 cols = source->Cols();
  vint8 bands = source->Bands();
  v3dMatrix<type1> * result = new v3dMatrix<type1>(rows, cols, bands);
  vArray(type1) matrix2 = result->Matrix();
  vint8 size = source->AllBandSize();

  type1 min_value = source->MinValue();
  type1 max_value = source->MaxValue();
  double range = (max_value - min_value) * 1.000001;
  double offset = lower;
  double factor;
  if (range == 0.0)
  {
    factor = 1.0;
  }
  else
  {
    factor = (upper - lower) / range;
  }

  vint8 i;
  for (i = 0; i < size; i++)
  {
    double entry = (double) matrix1[i];
    matrix2[i] = (type1) ((entry - min_value) * factor + offset);
  }

  min_value = result->MinValue();
	max_value = result->MaxValue();
	return result;
}


template<class type>
general_image * v3dMatrix<type>::BoundBands(vint8 lower, vint8 upper) const
{
  return S_BoundBands(this, (double) lower, (double) upper);
}


template<class type>
general_image * v3dMatrix<type>::BoundBands(float lower, float upper) const
{
  return S_BoundBands(this, lower, upper);
}


template<class type>
general_image * v3dMatrix<type>::BoundBands(double lower, double upper) const
{
  return S_BoundBands(this, lower, upper);
}


template<class type>
v3dMatrix<type> * v3dMatrix<type>::set_range(type lower, type upper)
{
  v3dMatrix<type> * result = new v3dMatrix(rows, cols, bands);
  set_range(lower, upper, result);
  return result;
}


template<class type>
vint8 v3dMatrix<type>::set_range(type lower, type upper, v3dMatrix<type> * result)
{
  if ((result->vertical() != vertical()) || 
      (result->horizontal() != horizontal()) || 
      (result->channels() != channels()))
  {
    exit_error("\nerror: incompatible sizes in set_range\n");
    return 0;
  }

  vArray(type) result_matrix = result->Matrix();

  double min_value = (double) MinValue();
  double max_value = (double) MaxValue();
  double range = (max_value - min_value) * 1.000001;
  type offset = lower;
  type factor;
  if (range == 0.0)
  {
    factor = (type) 1.0;
  }
  else
  {
    factor = (type) ((upper - lower) / range);
  }

  vint8 i;
  for (i = 0; i < size; i++)
  {
    type entry = matrix[i];
    result_matrix[i] = (type) ((entry - min_value) * factor + offset);
  }

	return 1;
}



template<class type>
color_image * v3dMatrix<type>::ToRGB() const
{
  if ((TypeName() == UcharType) && (Bands() == 3)) return (color_image *) this;
  else return (ToRGBCopy());
}


template<class type>
color_image * v3dMatrix<type>::ToRGBCopy() const
{
  if ((TypeName() == UcharType) && (Bands() == 3)) 
  {
    return (color_image *) Copy();
  }

  color_image * result;
  vint8 i;

  if (bands == 3)
  {
    result = new color_image(rows, cols);
    vArray(uchar) R = result->GetR();
    vArray(uchar) G = result->GetG();
    vArray(uchar) B = result->GetB();
    vArray(type) source_r = matrix2[0];
    vArray(type) source_g = matrix2[1];
    vArray(type) source_b = matrix2[2];

    for (i = 0; i < size; i++)
    {
      R[i] = (uchar) source_r[i];
      G[i] = (uchar) source_g[i];
      B[i] = (uchar) source_b[i];
    }
  }
  else if (bands == 1)
  {
    result = new color_image(rows, cols);
    vArray(uchar) R = result->GetR();
    vArray(uchar) G = result->GetG();
    vArray(uchar) B = result->GetB();
    vArray(type) source_i = matrix2[0];
    for (i = 0; i < size; i++)
    {
      uchar value = (uchar) source_i[i];
      R[i] = value;
      G[i] = value;
      B[i] = value;
    }
  }
  else 
  {
    general_image * average = AverageBands();
    result = average->ToRGBBoundCopy();
    vdelete(average);
  }
  return result;
}


template<class type>
color_image * v3dMatrix<type>::ToRGBBound() const
{
  if ((TypeName() == UcharType) && (Bands() == 3)) return (color_image *) this;
  return ToRGBBoundCopy();
}


template<class type>
color_image * v3dMatrix<type>::ToRGBBoundCopy() const
{
  if ((TypeName() == UcharType) && (Bands() == 3))
  {
    return (color_image *) Copy();
  }
	
  color_image * result;
  if (TypeName() == UcharType) 
  {
    result = this->ToRGB();
  }
  else
  {
    const v3dMatrix<type> * bound = (v3dMatrix<type> *) BoundBands((vint8)0, (vint8)255);
    result = bound->ToRGB();
    vdelete(bound);
  }

  return result;
}


template<class type>
grayscale_image * v3dMatrix<type>::ToGray() const
{
  if ((TypeName() == UcharType) && (Bands() == 1)) return (grayscale_image *) this;
  return ToGrayCopy();
}


template<class type>
grayscale_image * v3dMatrix<type>::ToGrayCopy() const
{
  if ((TypeName() == UcharType) && (Bands() == 1))
  {
    return (grayscale_image *) Copy();
  }

  grayscale_image * result;
  vint8 i;

  if (bands == 3)
  {
    result = new grayscale_image(rows, cols);
    vArray(uchar) I = result->GetI();
    vArray(type) source_r = matrix2[0];
    vArray(type) source_g = matrix2[1];
    vArray(type) source_b = matrix2[2];

    for (i = 0; i < size; i++)
    {
      double r = (double) source_r[i];
      double g = (double) source_g[i];
      double b = (double) source_b[i];
      double double_gray = 0.3 * r + 0.59 * g + 0.11 * b;
      
      // We want to convert the double_gray value to the
      // nearest integral value, basically we want to round it.
      uchar rounded = (uchar) double_gray;
      double diff = double_gray - rounded;
      if (diff >= 0.5) rounded++;
      
      I[i] = rounded;
    }
  }
  else if (bands == 1)
  {
    result = new grayscale_image(rows, cols);
    vArray(uchar) I = result->GetI();
    vArray(type) source_i = matrix2[0];
    for (i = 0; i < size; i++)
    {
      I[i] = (uchar) source_i[i];
    }
  }
  else 
  {
    general_image * average = AverageBands();
    result = average->ToGray();
    vdelete(average);
  }
  return result;
}


template<class type>
general_image * v3dMatrix<type>::Copy() const
{
  v3dMatrix<type> * result = new v3dMatrix<type>(this);
  return (general_image *) result;
}


template<class type>
general_image * v3dMatrix<type>::Copy(vint8 left, vint8 right, vint8 top, vint8 bottom) const
{
  if (left < 0)
  {
    left = 0;
  }
  if (right >= Cols()) 
  {
    right = Cols() - 1;
  }
  if (top < 0)
  {
    top = 0;
  }
  if (bottom >= Rows()) 
  {
    bottom = Rows() - 1;
  }
  
  if (right < left)
  {
    return 0;
  }

  if (bottom < top)
  {
    return 0;
  }
  
  vint8 result_rows = bottom - top + 1;
  vint8 result_cols = right - left + 1;
  v3dMatrix<type> * target = new v3dMatrix<type>(result_rows, result_cols, bands);

  // Get the size of the source and target
  vint8 source_rows = rows;
  vint8 source_cols = cols;
  vint8 target_rows = target->Rows();
  vint8 target_cols = target->Cols();
  vint8 min_rows = Min(source_rows, target_rows);
  vint8 min_cols = Min(source_cols, target_cols);

  // Get the boundaries of the source and target regions
  vint8 target_top = 0;
  vint8 target_left = 0;
  vint8 target_bottom = target_top + target_rows - 1;
  vint8 target_right = target_left + target_cols - 1;

  // Return false if one of the specified regions exceeds the boundaries
  // of its corresponding image
  if (top < 0) return 0;
  if (bottom >= source_rows) return 0;
  if (target_bottom >= target_rows) return 0;
  if (left < 0) return 0;
  if (right >= source_cols) return 0;;
  if (target_right >= target_cols) return 0;

  vint8 i, j, row, col, band;
  
  for (band = 0; band < bands; band++)
  {
    vArray2(type) matrix1 = Matrix2(band);
    vArray2(type) matrix2 = target->Matrix2(band);
    for (row = top; row <= bottom; row++)
    {
      i = row - top + target_top;
      for (col = left; col <= right; col++)
      {
	      j = col - left + target_left;
	      matrix2[i][j] = matrix1[row][col];
      }
    }
  }

  return target;
}



template<class type>
grayscale_image * v3dMatrix<type>::ToGrayBound() const
{
  if ((TypeName() == UcharType) && (Bands() == 1)) 
  {
    return (grayscale_image *) this;
  }
  else 
  {
    return ToGrayBoundCopy();
  }
}


template<class type>
grayscale_image * v3dMatrix<type>::ToGrayBoundCopy() const
{
  if ((TypeName() == UcharType) && (Bands() == 1)) 
  {
    return (grayscale_image *) Copy();
  }

  grayscale_image * result;
	
  if (TypeName() == UcharType)
  {
    result = this->ToGray();
  }

  else
  {
    v3dMatrix<type> * bound = (v3dMatrix<type> *) BoundBands((vint8)0, (vint8)255);
    result = bound->ToGray();
    vdelete(bound);
  }

  return result;
}


template<class type>
grayscale_image * v3dMatrix<type>::grayscale_bound_copy() const
{
  return ToGrayBoundCopy();
}


template<class type>
grayscale_image * v3dMatrix<type>::fast_grayscale_bound_copy() const
{
  if ((TypeName() == UcharType) && (Bands() == 1)) 
  {
    return (grayscale_image *) Copy();
  }

  if (TypeName() == UcharType)
  {
    return this->ToGray();
  }

  // so far (03/20/2006) my implementation only handles images with one channel.
  else if (bands != 1)
  {
    return this->grayscale_bound_copy();
  }
    
  grayscale_image * result_image = new grayscale_image(rows, cols);
	class_pointer(uchar) result = result_image->flat();

  type minimum_value = matrix[0];
  type maximum_value = minimum_value;

  // find minimum and maximum values
  vint8 counter;
  for (counter = 1; counter < size; counter++)
  {
    const type current = matrix[counter];
    if (current < minimum_value)
    {
      minimum_value = current;
    }
    else if (current > maximum_value)
    {
      maximum_value = current;
    }
  }

  // compute offset and scaling factor
  const float range = maximum_value - minimum_value;
  float factor;
  if (range == 0.0f)
  {
    factor = 1.0f;
  }
  else
  {
    factor = 255.0f / range;
  }

  // compute the pixel values for the grayscale image
  for (counter = 0; counter < size; counter++)
  {
    type current = matrix[counter];
    result[counter] = (uchar) (((float) (current - minimum_value)) * factor);
  }

  return result_image;
}


template<class type>
color_image * v3dMatrix<type>::color_bound_copy() const
{
  return ToRGBBoundCopy();
}


template<class type>
grayscale_image * v3dMatrix<type>::grayscale_copy() const
{
  return ToGrayCopy();
}


template<class type>
color_image * v3dMatrix<type>::color_copy() const
{
  return ToRGBCopy();
}


template<class type>
void v3dMatrix<type>::DoubleValues(vector<double> & values, 
				                             vint8 row, vint8 col) const
{
  vint8 band;
  for (band = 0; band < bands; band++)
    values.push_back((double) matrix3[band][row][col]);
}

template<class type>
void v3dMatrix<type>::DoubleValues(vector<double> & values, vint8 i) const
{
  vint8 band;
  for (band = 0; band < bands; band++)
    values.push_back((double) matrix2[band][i]);
}

template<class type>
void v3dMatrix<type>::Vint8Values(vector<vint8> & values, vint8 row, vint8 col) const
{
  vint8 band;
  for (band = 0; band < bands; band++)
    values.push_back((vint8) matrix3[band][row][col]);
}

template<class type>
void v3dMatrix<type>::Vint8Values(vector<vint8> & values, vint8 i) const
{
  vint8 band;
  for (band = 0; band < bands; band++)
    values.push_back((vint8) matrix2[band][i]);
}

template<class type>
double v3dMatrix<type>::double_value(vint8 channel, vint8 vertical, vint8 horizontal) const
{
  return (double) matrix3 [channel] [vertical] [horizontal];
}


template<class type>
vint8 v3dMatrix<type>::vint8_value (vint8 channel, vint8 vertical, vint8 horizontal) const
{
  return round_number (double_value(channel, vertical, horizontal));
}


template<class type>
double v3dMatrix<type>::double_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const
{
  if ((channel < 0) || (channel >= bands) || (vertical < 0) || (vertical >= rows) || 
      (horizontal <0) || (horizontal >= cols))
  {
    return (double) -12345678;
  }

  return (double) matrix3 [channel] [vertical] [horizontal];
}


template<class type>
vint8 v3dMatrix<type>::vint8_value_safe (vint8 channel, vint8 vertical, vint8 horizontal) const
{
  if ((channel < 0) || (channel >= bands) || (vertical < 0) || (vertical >= rows) || 
      (horizontal <0) || (horizontal >= cols))
  {
    return (vint8) -12345678;
  }

  return round_number (double_value(channel, vertical, horizontal));
}


template<class type>
double v3dMatrix<type>::double_value_strict (vint8 channel, vint8 vertical, vint8 horizontal) const
{
  if ((channel < 0) || (channel >= bands) || (vertical < 0) || (vertical >= rows) || 
      (horizontal <0) || (horizontal >= cols))
  {
    exit_error ("\ndouble_value_strict (%li, %li, %li) for image(%li, %li, %li)\n",
                         channel, vertical, horizontal, bands, rows, cols);
  }

  return (double) matrix3 [channel] [vertical] [horizontal];
}


template<class type>
vint8 v3dMatrix<type>::vint8_value_strict(vint8 channel, vint8 vertical, vint8 horizontal) const
{
  if ((channel < 0) || (channel >= bands) || (vertical < 0) || (vertical >= rows) || 
      (horizontal <0) || (horizontal >= cols))
  {
    exit_error ("\ndouble_value_strict (%li, %li, %li) for image(%li, %li, %li)\n",
                         channel, vertical, horizontal, bands, rows, cols);
  }

  return round_number (double_value(channel, vertical, horizontal));
}


template<class type>
v3dMatrix<uchar> * v3dMatrix<type>::threshold(double threshold) const
{
  v3dMatrix<uchar> * result = new v3dMatrix<uchar>(rows, cols, 1);
  vint8 i, band;
  double sum;
  vArray(uchar) I = result->Matrix(0);

  for (i = 0; i < size; i++)
    {
      sum = 0;
      for (band = 0; band < bands; band++)
      	sum += matrix2[band][i];

      if (sum >= threshold) 
		  I[i] = 255;
      else I[i] = 0;
    }

  return result;
}

template<class type>
general_image * v3dMatrix<type>::AverageRegions(vint8 win_rows, 
						vint8 win_cols) const
{
  vint8 result_rows = rows / win_rows;
  vint8 result_cols = cols / win_cols;

  v3dMatrix<type> * result = new v3dMatrix<type>(result_rows, result_cols,
						     1);
  vArray3(type) I1 = matrix3;
  vArray2(type) I2 = result->Matrix2(0);

  vint8 row, col, i, j, band;
  double temp;

  vint8 result_row = 0;
  vint8 factor = bands * win_rows * win_cols;
  vint8 limit_rows = result_rows * win_rows;
  vint8 limit_cols = result_cols * win_cols;

  for (row = 0; row < limit_rows; row += win_rows)
  {
    vint8 result_col = 0;
    for (col = 0; col < limit_cols; col += win_cols)
    {
      temp = 0;
      for (i = 0; i < win_rows; i++)
	for (j = 0; j < win_cols; j++)
	{
	  vint8 temp_i = row + i;
	  vint8 temp_j = col + j;
	  for (band = 0; band < bands; band++)
	    temp += I1[band][temp_i][temp_j];
	}
      temp = temp / factor;
      I2[result_row][result_col] = (type) temp;
      result_col++;
    }
    result_row++;
  }
  return result;
}

template<class type>
static void S_Resample(vArray2(type) source, vArray2(type) target, 
		       vint8 top, vint8 left, vint8 rows1, vint8 cols1,
		       vint8 rows2, vint8 cols2)
{
  double row, col;
  vint8 i1, j1, i2, j2;
  
  double rowf = rows2 / (double) rows1;
  double colf = cols2 / (double) cols1;
  
  for (row = 0; row < rows2; row++)
    for (col = 0; col < cols2; col++)
    {
      i1 = (vint8) (top + (row / rowf));
      j1 = (vint8) (left + (col / colf));
      i2 = (vint8) row;
      j2 = (vint8) col;
      target[i2][j2] = source[i1][j1];
    }
}

template<class type>
general_image * v3dMatrix<type>::Resample(vint8 top, vint8 left, 
					  vint8 rows1, vint8 cols1,
					  vint8 rows2, vint8 cols2) const
{
  if ((top < 0) || (top >= rows) || (left < 0) || (left >= cols) ||
      (rows1 < 0) || (rows1 + top > rows) ||
      (cols1 < 0) || (cols1 + left > cols) ||
      (rows2 < 0) || (cols2 < 0))
  {
    return 0;
  }

  v3dMatrix<type> * result = new v3dMatrix<type>(rows2, cols2, bands);
  vint8 band;
  for (band = 0; band < bands; band++)
  {
    S_Resample(Matrix2(band), result->Matrix2(band), 
	       top, left, rows1, cols1, rows2, cols2);
  }
  return result;
}


// Resample2 takes as input the subimage defined by (top, left) as its top
// left corner that has full_rows1 rows and full_cols1 columns. It creates
// a result image in which the input subimage is subsampled (1 out of
// every "scale" rows and columns survive). If scale doesn't divide 
// exactly rows (or cols) the remainder of rows (or cols) of the input
// image is ignored.
//
// Note that the input image is not blurred before resampling. It is the 
// responsibility of the caller to appropriately blur the input image before
// calling Resample2
template<class type>
general_image * v3dMatrix<type>::Resample2(vint8 top, vint8 left, 
					                                 vint8 full_rows1, vint8 full_cols1,
                                					 vint8 scale) const
{
  // Check for some error conditions
  if ((top < 0) || (top >= rows) || (left < 0) || (left >= cols) ||
      (full_rows1 < 0) || (full_rows1 + top > rows) ||
      (full_cols1 < 0) || (full_cols1 + left > cols))
  {
    return 0;
  }

  if (scale == 0) 
  {
    return 0;
  }

  // figure out the size of the result image. We will ignore parts at
  // the bottom or right of the input image that are not enough to make
  // a row or column in the result image
  vint8 rows2 = full_rows1 / scale;
  vint8 cols2 = full_cols1 / scale;

  // Now figure out the actual rows and cols of the input image that will
  // be used (remainders of division by scale will be discarded).
  vint8 rows1 = scale * rows2;
  vint8 cols1 = scale * cols2;
  
  v3dMatrix<type> * result = new v3dMatrix<type>(rows2, cols2, bands);
  vint8 band;
  for (band = 0; band < bands; band++)
  {
    S_Resample(Matrix2(band), result->Matrix2(band), 
	       top, left, rows1, cols1, rows2, cols2);
  }
  return result;
}

template<class type>
general_image * v3dMatrix<type>::Resample(vint8 rows2, vint8 cols2) const
{
  return Resample(0, 0, rows, cols, rows2, cols2);
}


// Find the 4-connected components in the image.
template<class type>
v3dMatrix<vint8> * v3dMatrix<type>::ConnectedComponents(vector<Label> * label_objects) const
{
  // We assume that the image is binarized, and gray-scale. This way,
  // it is enough to look at the R band.

  vArray2(type) R2 = Matrix2(0);
  vint8 number_of_labels = 0;
  vint8 row, col;

  v3dMatrix<vint8> * labels_image = new v3dMatrix<vint8>(rows, cols, 1);
  vArray2(vint8) labels = labels_image->Matrix3()[0];

  uchar previous_value = 0;

  // First pass: assign some initial labels to each pixel. We assign the 
  // same label to consecutive non-zero
  // pixels in the same row.
  for (row = 0; row < rows; row++)
  {
    previous_value = (uchar) 0;
    for (col = 0; col < cols; col++)
    {
      type current = R2[row][col];
      if (current != 0)
      {
        if (previous_value == 0) number_of_labels++;
        labels[row][col] = number_of_labels;
      }
      else labels[row][col] = 0;
      previous_value = (uchar) current;
    }
  }


 	  
// Second pass: For each connected region, find the smallest label in
//  that region.
  vint8 * label_refs = new vint8[(vector_size) (number_of_labels+1)];
  vint8 i;
  for (i = 0; i <= number_of_labels; i++)
    label_refs[i] = i;

  for (row = 1; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 label1 = labels[row][col];
      if (label1 == 0) continue;
      vint8 label2 = labels[row-1][col];
      if (label2 == 0) continue;
      vint8 ref1 = label_refs[label1];
      vint8 ref2 = label_refs[label2];
      vint8 temp_ref;
      while (ref1 != label_refs[ref1])  
      {
	      temp_ref = ref1;
	      ref1 = label_refs[ref1];
	      label_refs[temp_ref] = label_refs[ref1];
      }

      while (ref2 != label_refs[ref2])  
      {
	      temp_ref = ref2;
	      ref2 = label_refs[ref2];
	      label_refs[temp_ref] = label_refs[ref2];
      }

      if (ref1 < ref2) temp_ref = ref1;
      else temp_ref = ref2;

      label_refs[label2] = temp_ref;
      label_refs[label1] = temp_ref;
      label_refs[ref1] = temp_ref;
      label_refs[ref2] = temp_ref;
      labels[row][col] = temp_ref;
    }
  }

  // Now we can count the number of different connected components in
  // the image. We renumber the labels, so that they have consecutive
  // numbers.
  vint8 * new_refs = new vint8[(vector_size) (number_of_labels+1)];
  for (i = 0; i <= number_of_labels; i++)
    new_refs[i] = 0;
  vint8 new_numbers = 1;

  // here we update temp_label, so that for each equivalence class
  // of labels we identify the smallest label id.
  for (i = 1; i <= number_of_labels; i++)
  {
    vint8 temp_label = label_refs[i];
    while(temp_label != label_refs[temp_label])
    {
      if (temp_label <= label_refs[temp_label])
      {
        exit_error("Bug in ConnectedComponents\n");
      }
      vint8 temp_ref = temp_label;

      // temp_label now is the label pointed to by temp_ref.
      temp_label = label_refs[temp_label];
      label_refs[temp_ref] = label_refs[temp_label];
    }
    label_refs[i] = temp_label;
    if (temp_label == i)
    {
      new_refs[temp_label] = new_numbers;
      new_numbers++;
    }
    else 
    {
      new_refs[i] = new_refs[temp_label];
    }
  }

  // Third pass: For each pixel, replace the temporary label with the
  // renumbered label;

  // number_of_components is a pass-by-reference argument to the function.

  Label temp_label;
  for (i = 0; i < new_numbers; i++)
  {
    temp_label.id = i;
    label_objects->push_back(temp_label);
  }

  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < cols; col++)
    {
      vint8 label = labels[row][col];
      vint8 label_ref = label_refs[label];
      vint8 new_label = new_refs[label_ref];
      labels[row][col] = new_label;
      Label & label_object = (*label_objects)[(vector_size) new_label];
      label_object.count++;
      if (row < label_object.top)
	      label_object.top = row;
      if (row > label_object.bottom)
	      label_object.bottom = row;
      if (col < label_object.left)
	      label_object.left = col;
      if (col > label_object.right)
	      label_object.right = col;

      // Store the value corresponding to the label, if this
      // is the first pixel we found on that label
      if (label_object.count == 1)
      {
        label_object.value = (uchar) (R2[row][col]);
      }
    }
  }

  vdelete2(label_refs);
  vdelete2(new_refs);
  return labels_image;
}


// Prints interlaced.
template<class type>
vint8 v3dMatrix<type>::Print(const char * text) const
{
  vint8 row, col, band;
  vPrint("(%li x %li x %li). %s = [\n", band, rows, cols, text);
  for (row = 0; row < rows; row++)
  { 
    for (col = 0; col < cols; col++)
    {
      vPrint("(%li, %li): ", row, col);
      for (band = 0; band < bands; band++)
      {
        vPrint("%lf ", (double) (matrix3[band][row][col]));
      }
      vPrint("\n];\n");
    }
  }

  vPrint("\n");
  return 1;
}

  
// Prints bandwise.
template<class type>
vint8 v3dMatrix<type>::Print2(const char * text) const
{
  vint8 row, col, band;
  vPrint("\n%s: %li x %li x %li\n", text, (long)bands, (long)rows, (long)cols);
  for (band = 0; band < bands; band++)
  {
    vPrint("band %li\n", (long)band);
    for (row = 0; row < rows; row++)
    { 
      for (col = 0; col < cols; col++)
      {
        vPrint("%lf ", (double)matrix3[band][row][col]);				  
      }
      vPrint("\n");
    }
    vPrint("\n");
  }
  return 1;
}


// file position where information for the specific entry is stored
template<class type>
vint8 v3dMatrix<type>::file_position(vint8 channel, vint8 vertical, vint8 horizontal) const
{
  vint8 result = header_bytes() + (channel * size + vertical * cols + horizontal) * sizeof(type);
  return result;
}


// file position where information for the specific entry (channel, row, column) is stored
// rows and cols are the size of the matrix stored in the file.
template<class type>
vint8 v3dMatrix<type>::file_position(vint8 rows, vint8 cols, vint8 channel, vint8 row, vint8 column)
{
  vint8 size = rows * cols;
  vint8 result = header_bytes() + (channel * size + vertical * cols + horizontal) * sizeof(type);
  return result;
}


// file position where information for the specific entry is stored
// this function should only be called for 2D matrices (i.e., matrices
// with one channel), otherwise the program will exit.
template<class type>
vint8 v3dMatrix<type>::file_position(vint8 vertical, vint8 horizontal) const
{
  if (bands != (vint8) 1)
  {
    exit_error("error: channels = %li in file position\n", (long) bands);
  }
  return file_position(0, vertical, horizontal);
}


template<class type>
v4dArray<type>::v4dArray(vint8 in_dim1, vint8 in_dim2, vint8 in_dim3, vint8 in_dim4)
{
  dim1 = in_dim1;
  dim2 = in_dim2;
  dim3 = in_dim3;
  dim4 = in_dim4;
  dim1_low = 0;
  dim2_low = 0;
  dim3_low = 0;
  dim4_low = 0;
  dim1_high = dim1 - 1;
  dim2_high = dim2 - 1;
  dim3_high = dim3 - 1;
  dim4_high = dim4 - 1;

  ZeroArrays();
  Initialize();
}


template<class type>
v4dArray<type>::v4dArray(vint8 in_dim1_low, vint8 in_dim1_high,
                       vint8 in_dim2_low, vint8 in_dim2_high,
                       vint8 in_dim3_low, vint8 in_dim3_high,
                       vint8 in_dim4_low, vint8 in_dim4_high)
{
  dim1_low = in_dim1_low;
  dim2_low = in_dim2_low;
  dim3_low = in_dim3_low;
  dim4_low = in_dim4_low;

  dim1_high = in_dim1_high;
  dim2_high = in_dim2_high;
  dim3_high = in_dim3_high;
  dim4_high = in_dim4_high;

  dim1 = dim1_high - dim1_low + 1;
  dim2 = dim2_high - dim2_low + 1;
  dim3 = dim3_high - dim3_low + 1;
  dim4 = dim4_high - dim4_low + 1;

  ZeroArrays();
  Initialize();
}


template<class type>
v4dArray<type>::~v4dArray()
{
  DeleteArrays();
}


template<class type>
vint8 v4dArray<type>::ZeroArrays()
{
  hidden_data = vZero(type);
  hidden_data2 = vZero(vArray(type));
  hidden_data3 = vZero(vArray2(type));
  hidden_data4 = vZero(vArray3(type));
  return 1;
}


template<class type>
vint8 v4dArray<type>::Initialize()
{
  size = dim1 * dim2 * dim3 * dim4;
  vint8 size2 = dim2 * dim3 * dim4;
  vint8 size3 = dim3 * dim4;

  hidden_data = vnew(type, (vector_size) size);

  vint8 i1, i2, i3;
  hidden_data2 = vnew(vArray(type), (vector_size) dim1);
  for (i1 = 0; i1 < dim1; i1++)
  {
    hidden_data2[i1] = hidden_data + i1 * size2;
  }

  hidden_data3 = vnew(vArray2(type), (vector_size) dim1);
  for (i1 = 0; i1 < dim1; i1++)
  {
    hidden_data3[i1] = vnew(vArray(type), (vector_size) dim2);
    for (i2 = 0; i2 < dim2; i2++)
    {
      hidden_data3[i1][i2] = hidden_data2[i1] + i2 * size3;
    }
  }

  hidden_data4 = vnew(vArray3(type), (vector_size) dim1);
  data4 = vnew(vArray3(type), (vector_size) dim1);
  data4 = data4 - dim1_low;
  for (i1 = 0; i1 < dim1; i1++)
  {
    hidden_data4[i1] = vnew(vArray2(type), (vector_size) dim2);
    vArray3(type) array3 = vnew(vArray2(type), (vector_size) dim2);
    data4[i1+dim1_low] = array3 - dim2_low;
    for (i2 = 0; i2 < dim2; i2++)
    {
      hidden_data4[i1][i2] = vnew(vArray(type), (vector_size) dim3);
      vArray2(type) array2 = vnew(vArray(type), (vector_size) dim3);
      data4[i1+dim1_low][i2+dim2_low] = array2 - dim3_low; 
      for (i3 = 0; i3 < dim3; i3++)
      {
        hidden_data4[i1][i2][i3] = hidden_data3[i1][i2] + i3 * dim4;
        data4[i1+dim1_low][i2+dim2_low][i3+dim3_low] = hidden_data4[i1][i2][i3] - dim4_low;
      }
    }
  }
  return 1;
}


template<class type>
vint8 v4dArray<type>::DeleteArrays()
{
  vint8 i1, i2;
  for (i1 = 0; i1 < dim1; i1++)
  {
    for (i2 = 0; i2 < dim2; i2++)
    {
      vdelete2(hidden_data4[i1][i2]);
      vdelete2(data4[i1+dim1_low][i2+dim2_low] + dim3_low);
    }
    vdelete2(hidden_data4[i1]);
    vdelete2(data4[i1+dim1_low] + dim2_low);
    vdelete2(hidden_data3[i1]);
  }
  vdelete2(hidden_data);
  vdelete2(hidden_data2);
  vdelete2(hidden_data3);
  vdelete2(hidden_data4);
  vdelete2(data4+dim1_low);

  return 1;
}


template<class type>
vint8 v4dArray<type>::Write(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0)
  {
    return 0;
  }
  vint8 success = Write(fp);
  fclose(fp);
  return success;
}


template<class type>
vint8 v4dArray<type>::Write(FILE * fp)
{
  vint8 zero = 0;
  vint8 items_written = 0;
  items_written += store_vint8s(fp, &zero, 1);
  items_written += store_vint8s(fp, &dim1, 1);
  items_written += store_vint8s(fp, &dim2, 1);
  items_written += store_vint8s(fp, &dim3, 1);
  items_written += store_vint8s(fp, &dim4, 1);

  items_written += store_vint8s(fp, &dim1_low, 1);
  items_written += store_vint8s(fp, &dim2_low, 1);
  items_written += store_vint8s(fp, &dim3_low, 1);
  items_written += store_vint8s(fp, &dim4_low, 1);

  items_written += store_vint8s(fp, &dim1_high, 1);
  items_written += store_vint8s(fp, &dim2_high, 1);
  items_written += store_vint8s(fp, &dim3_high, 1);
  items_written += store_vint8s(fp, &dim4_high, 1);

  if (items_written != 13) return 0;
  items_written = fwrite(vData(hidden_data), sizeof(type), (long) size, fp);
  if (items_written == size) return 1;
  else return 0;
}


template<class type>
v4dArray<type> * v4dArray<type>::Read(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0)
  {
    return 0;
  }
  v4dArray<type> * result = Read(fp);
  fclose(fp);
  return result;
}


template<class type>
v4dArray<type> * v4dArray<type>::Read(FILE * fp)
{
  vint8 zero = 0;
  vint8 items_read = 0;
  vint8 dim1 = 0, dim2 = 0, dim3 = 0, dim4 = 0;
  vint8 dim1_low = 0, dim2_low = 0, dim3_low = 0, dim4_low = 0;
  vint8 dim1_high = 0, dim2_high = 0, dim3_high = 0, dim4_high = 0;

  items_read += read_vint8s(fp, &zero, 1);
  items_read += read_vint8s(fp, &dim1, 1);
  items_read += read_vint8s(fp, &dim2, 1);
  items_read += read_vint8s(fp, &dim3, 1);
  items_read += read_vint8s(fp, &dim4, 1);

  items_read += read_vint8s(fp, &dim1_low, 1);
  items_read += read_vint8s(fp, &dim2_low, 1);
  items_read += read_vint8s(fp, &dim3_low, 1);
  items_read += read_vint8s(fp, &dim4_low, 1);

  items_read += read_vint8s(fp, &dim1_high, 1);
  items_read += read_vint8s(fp, &dim2_high, 1);
  items_read += read_vint8s(fp, &dim3_high, 1);
  items_read += read_vint8s(fp, &dim4_high, 1);

  if (zero != 0) return 0;
  if (items_read != 13) return 0;
  v4dArray<type> * result = 
    new v4dArray(dim1_low, dim1_high, dim2_low, dim2_high,
                   dim3_low, dim3_high, dim4_low, dim4_high);

  vint8 success = result->ReadData(fp);
  if (success <= 0)
  {
    vdelete(result);
    return 0;
  }
  else
  {
    return result;
  }
}


// This function is useful if we are at the position where the array
// is saved and we want to skip the header and get directly to 
// the data.
template<class type>
vint8 v4dArray<type>::SkipHeader(FILE * fp)
{
  const vint8 header_items = 13;
  vint8 buffer[header_items];
  vint8 items = read_vint8s(fp, buffer, header_items);
  if (items != header_items) return 0;
  else return 1;
}


template<class type>
vint8 v4dArray<type>::ReadData(FILE * fp)
{
  vint8 items_read = fread(vData(hidden_data), sizeof(type), (long) size, fp);
  if (items_read == size) return 1;
  else return 0;
}


template<class type>
vint8 v4dArray<type>::Print()
{
  vint8 i1, i2, i3, i4;
  for (i1 = 0; i1 < dim1; i1++)
  {
    vPrint("Dimension 1 position %li, %li\n", i1, i1+dim1_low);
    for (i2 = 0; i2 < dim2; i2++)
    {
      vPrint("  Dimension 2 position %li, %li\n", i2, i2+dim2_low);
      for (i3 = 0; i3 < dim3; i3++)
      {
        vPrint("    Dimension 3 position %li, %li\n", i3, i3+dim3_low);
        for (i4 = 0; i4 < dim4; i4++)
        {
          vPrint("      hidden_data4[%li][%li][%li][%li] = %lf\n",
                  i1, i2, i3, i4, hidden_data4[i1][i2][i3][i4]);
          vPrint("      data4[%li][%li][%li][%li] = %lf\n",
                  i1+dim1_low, i2+dim2_low, i3+dim3_low, i4+dim4_low, 
                  data4[i1+dim1_low][i2+dim2_low][i3+dim3_low][i4+dim4_low]);
        }
      }
    }
  }
  return 1;
}


template<class type>
vint8 v4dArray<type>::Zero()
{
  bzero(vData(hidden_data), MemorySize());
  return 1;
}


template<class type>
vint8 v4dArray<type>::WriteValue(type value)
{
  vint8 i;
  for (i = 0; i < size; i++)
  {
    hidden_data[i] = value;
  }
  return 1;
}


// If the cpp template file is included as part of the header file,
// we must undefine certain things.
#include "undefine.h"


#endif // VASSILIS_INCLUDING_MATRIX_CPP

