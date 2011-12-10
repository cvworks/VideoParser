
#include "vplatform.h"

#ifdef VASSILIS_SGI_PLATFORM
#include <fstream>
#else 
#include <fstream>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "auxiliaries.h"
#include "matrix.h"
#include "image.h"
#include "drawing.h"
#include "wrapper.h"

#include "basics/definitions.h"

/////////////////////////////////////////////////////
//
// Beginning of implementation of class vPPM_Header
//
/////////////////////////////////////////////////////

class vPPM_Header
{
private:
  long rows;
  long cols;
  long max_value;
  long offset; // Offset in the file where the actual pixel data starts.
  char header[4];
  short is_valid;

public:
  vPPM_Header();
  vPPM_Header(const char * filename);
  vPPM_Header(long in_rows, long in_cols, char * in_header);
  long Rows();
  long Cols();
  long MaxValue();
  long Offset();
  char * Header();
  ushort Write(FILE * fp);
  short IsValid();
};


vPPM_Header::vPPM_Header()
{
}


vPPM_Header::vPPM_Header(const char * filename)
{
  const int BUFFER_SIZE = 5000;

  // BUFFER_SIZE is the number of bytes we read first. It is assumed to
  // be enough to read the whole header.

  static char buffer[BUFFER_SIZE];
  is_valid = 0;

  FILE *fp = fopen(filename, vFOPEN_READ);
  if (fp == 0)
  {
    return;
  }
  int data_read = fread( (void*) buffer, sizeof(char), BUFFER_SIZE, fp);
  fclose(fp);
  if (data_read <= 3)
  {
    printf("Error: Output of conversion from %s contains no data.\n", 
	   filename);
    return;
  }

  memcpy(header, buffer, 3);
  header[3] = 0;


  int counter = 3;  // Current position in buffer.
  char current = buffer[counter];

  // We go past comment lines, one by one.
  while(current == '#')
  {
    // Read next comment line.
    while (buffer[++counter] != '\n')
      if (counter == BUFFER_SIZE)
      {
	      printf("Error: Unterminated comment in output of conversion \n");
	      printf("       from %s\n", filename);
	      return;
      }
    current = buffer[++counter];
  }

  int len;
  rows = -1;
  cols = -1;
  max_value = -1;
  sscanf(&buffer[counter], "%i%i%i%n", &cols, &rows, &max_value, &len);

  if ( (rows < 0) || (cols < 0) )
  {
    printf("Error: Could not read image size in output of conversion \n");
    printf("       from %s \n", filename);
  }

  if (max_value != 255)
  {
    printf("Error: Max gray scale value in output of conversion from ");
    printf("%s\n       is not the expected 255 but %i\n", 
	   filename, max_value);
  }

  counter = counter + len;
  current = buffer[counter++];
  if (current != '\n')
  {
    printf("Error: Unexpected character after image size in output of\n"); 
    printf("       conversion from %s: %1s\n", filename, &current);
    return;
  }
  offset = counter;
  is_valid = 1;
}


vPPM_Header::vPPM_Header(long in_rows, long in_cols, 
			   char * in_header)
{
  offset = 0;
  rows = in_rows;
  cols = in_cols;
  max_value = 255;
  memcpy(header, in_header, 3);
  header[3] = 0;
}


long vPPM_Header::Rows()
{
  return rows;
}


long vPPM_Header::Cols()
{
  return cols;
}


long vPPM_Header::MaxValue()
{
  return max_value;
}


long vPPM_Header::Offset()
{
  return offset;
}


char * vPPM_Header::Header()
{
  return header;
}


ushort vPPM_Header::Write(FILE * fp)
{
  int result;
  result = fprintf(fp, "%s", header);
  if (result < 0) return 0;
  result = fprintf(fp, 
	                "# Made by function S_Write_PPM (written by Vassilis Athitsos)\n");
  if (result < 0) return 0;
  result = fprintf(fp, "%i %i\n%i\n", cols, rows, max_value);
  if (result < 0) return 0;
  return 1;
}


short vPPM_Header::IsValid()
{
  return is_valid;
}


/////////////////////////////////////////////////////
//
// End of implementation of class vPPM_Header
//
/////////////////////////////////////////////////////


general_image::~general_image()
{
//  AfxMessageBox("ImageBase destructor called\n");
}


general_image * general_image::Read(const char * filename)
{
  if (vIsPPM(filename)) return color_image::load(filename);
  else if (vIsPGM(filename)) return grayscale_image::load(filename);
  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    return color_image::load(filename);
  }
  else
  {
    FILE * fp = fopen(filename, vFOPEN_READ);
    if (fp == 0) return 0;
    return Read(fp);
    fclose(fp);
  }
}


// print some general information about the image: type name,
// dimensions.
vint8 general_image::print_header() const
{
  print_type_name(type_name);
  function_print(", %li channels, %li rows, %li cols\n", bands, rows, cols);
  return 1;
}


// creates a general image from raw data.  The raw data itself is copied,
// so it can be deleted after this function has been called.
general_image * general_image::from_data(void * data, vint8 channels, vint8 vertical, vint8 horizontal,
                                         vint8 bits_per_pixel)
{
  switch(bits_per_pixel)
  {
  case 8:
    return uchar_image::from_data(data, channels, vertical, horizontal);

  case 16:
    return ushort_image::from_data(data, channels, vertical, horizontal);

  case 32:
    return uvint8_image::from_data(data, channels, vertical, horizontal);

  default:
    exit_error("\nerror: in general_image::from_data, bits_per_pixel = %li\n", bits_per_pixel);
  }

  return 0;
}


general_image * general_image::Read(FILE * fp)
{
  long type_number;
  long items_read;

  vint8 current_position = function_tell(fp);
  items_read = fread(&type_number, sizeof(long), 1, fp);
  if (items_read != 1) 
  {
    return 0;
  }
  // We should put the type back, so that the Read function
  // of the subclass can also read it.
  ushort success = function_seek(fp, current_position, SEEK_SET);
  if (success == 0) return 0;


  switch(type_number)
  {
  case ScharType:
    return v3dMatrix<char>::Read(fp);

  case ShortType:
    return v3dMatrix<short>::Read(fp);

  case IntType:
    return v3dMatrix<int>::Read(fp);

  case Vint8Type:
    return v3dMatrix<long>::Read(fp);

  case FloatType:
    return v3dMatrix<float>::Read(fp);

  case DoubleType:
    return v3dMatrix<double>::Read(fp);

  case UcharType:
    return v3dMatrix<uchar>::Read(fp);

  case UshortType:
    return v3dMatrix<ushort>::Read(fp);

  case UintType:
    return v3dMatrix<uint>::Read(fp);

  case UlongType:
    return v3dMatrix<ulong>::Read(fp);

  case OtherType:
    return 0;

  default:
    assert(0);
    return 0;
  }
}


general_image * general_image::ReadText(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_READ);
  if (fp == 0) return 0;
  long type_number;
  long items_read;
  items_read = fscanf(fp, "%li", &type_number);
  fclose(fp);
  if (items_read != 1) 
  {
    return 0;
  }

  switch(type_number)
  {
  case ScharType:
    return v3dMatrix<char>::ReadText(filename);

  case ShortType:
    return v3dMatrix<short>::ReadText(filename);

  case IntType:
    return v3dMatrix<int>::ReadText(filename);

  case Vint8Type:
    return v3dMatrix<long>::ReadText(filename);

  case FloatType:
    return v3dMatrix<float>::ReadText(filename);

  case DoubleType:
    return v3dMatrix<double>::ReadText(filename);

  case UcharType:
    return v3dMatrix<uchar>::ReadText(filename);

  case UshortType:
    return v3dMatrix<ushort>::ReadText(filename);

  case UintType:
    return v3dMatrix<uint>::ReadText(filename);

  case UlongType:
    return v3dMatrix<ulong>::ReadText(filename);

  case OtherType:
    return 0;

  default:
    assert(0);
    return 0;
  }
}


color_image::color_image(vint8 in_rows, vint8 in_cols) 
: v3dMatrix<uchar>(in_rows, in_cols, 3)
{
}


color_image::~color_image()
{
}


vArray(uchar) color_image::GetR()
{
  return matrix2[0];
}


vArray(uchar) color_image::GetG()
{
  return matrix2[1];
}


vArray(uchar) color_image::GetB()
{
  return matrix2[2];
}


vArray(uchar) color_image::red_pointer()
{
  return matrix2[0];
}


vArray(uchar) color_image::green_pointer()
{
  return matrix2[1];
}


vArray(uchar) color_image::blue_pointer()
{
  return matrix2[2];
}


vArray2(uchar) color_image::GetR2()
{
  return matrix3[0];
}


vArray2(uchar) color_image::GetG2()
{
  return matrix3[1];
}


vArray2(uchar) color_image::GetB2()
{
  return matrix3[2];
}


vArray2(uchar) color_image::red_matrix()
{
  return matrix3[0];
}


vArray2(uchar) color_image::green_matrix()
{
  return matrix3[1];
}


vArray2(uchar) color_image::blue_matrix()
{
  return matrix3[2];
}


color_image * color_image::load(const char * filename)
{
  color_image * result = 0;

  if (vStringCaseEndsIn(filename, ".ppm"))
  {
    vPPM_Header header(filename);
    if (header.IsValid() <= 0) return 0;
    if(strcmp(header.Header(), "P6\n") != 0) return 0;
    result = new color_image(header.Rows(), header.Cols());
    result->ReadInterlaced(filename, header.Offset());
  }
  
  else if (vStringCaseEndsIn(filename, ".pgm"))
  {
    vector<vint8> band_order;
    band_order.push_back(0);
    vPPM_Header header(filename);
    if (header.IsValid() <= 0) return 0;
    if ((strcmp(header.Header(), "P5\n") != 0) &&
        (strcmp(header.Header(), "P2\n") != 0))
    {
      return 0;
    }

    result = new color_image(header.Rows(), header.Cols());

    if (strcmp(header.Header(), "P5\n") == 0)
    {
      result->ReadBandwise(filename, header.Offset(), &band_order);
    }
    else if (strcmp(header.Header(), "P2\n") == 0)
    {
      result->ReadTextBandwise(filename, header.Offset());
    }

    vArray2(uchar) bands = result->Matrix2();
    long i, j;
    for (j = 1; j < result->Bands(); j++)
      for (i = 0;  i < result->Size(); i++)
      	bands[j][i] = bands[0][i];
  }
  
  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    result = vReadBMP(filename);
  }

  else
  {
    return 0;
  }
  return result;
}


ushort color_image::save(const char * filename)
{
  ushort result = 0;
  if (vStringCaseEndsIn(filename, ".ppm"))
  {
    vPPM_Header header((long) rows, (long) cols, "P6\n");
    FILE * fp = fopen(filename, vFOPEN_WRITE);
    if (fp == 0) return 0;
    result = 1;
    ushort success;
    success = header.Write(fp);
    if (success == 0) result = 0;
    success = WriteInterlaced(fp);
    if (success == 0) result = 0;
    fclose(fp);
  }
  else if (vStringCaseEndsIn(filename, ".pgm"))
  {
    grayscale_image * the_image = this->grayscale_copy();
    result = the_image->save(filename);
    vdelete(the_image);
  }
  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    result = (ushort) function_save_bitmap(this, filename);
  }
  

  return result;
}


grayscale_image::grayscale_image(vint8 in_rows, vint8 in_cols) :
v3dMatrix<uchar>(in_rows, in_cols, 1)
{
}


grayscale_image::~grayscale_image()
{
}


vArray(uchar) grayscale_image::GetI()
{
  return matrix2[0];
}


vArray2(uchar) grayscale_image::GetI2()
{
  return matrix3[0];
}


grayscale_image * grayscale_image::load(const char * filename)
{
  grayscale_image * result = 0;

  if (vStringCaseEndsIn(filename, ".ppm"))
  {
    color_image * the_image = color_image::load(filename);
    if (the_image == 0)
    {
      return 0;
    }
    result = the_image->ToGray();
    vdelete(the_image);
  }
  else if (vStringCaseEndsIn(filename, ".pgm"))
  {
    vPPM_Header header(filename);
    if ((strcmp(header.Header(), "P5\n") != 0) &&
        (strcmp(header.Header(), "P2\n") != 0))
    {
      return 0;
    }

    result = new grayscale_image(header.Rows(), header.Cols());
    if (strcmp(header.Header(), "P5\n") == 0)
    {
      result->ReadBandwise(filename, header.Offset());
    }
    else if (strcmp(header.Header(), "P2\n") == 0)
    {
      result->ReadTextBandwise(filename, header.Offset());
    }
  }

  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    color_image * the_image = color_image::load(filename);
    if (the_image == 0)
    {
      return 0;
    }
    result = the_image->ToGray();
    vdelete(the_image);
  }

  else
  {
    return 0;
  }

  return result;
}


ushort grayscale_image::save(const char * filename)
{
  ushort result = 1;
  if (vStringCaseEndsIn(filename, ".ppm"))
  {
    color_image * the_image = this->color_copy();
    result = the_image->save(filename);
    vdelete(the_image);
  }
  else if (vStringCaseEndsIn(filename, ".pgm"))
  {
    vPPM_Header header((vint4) rows, (vint4) cols, "P5\n");
    FILE * fp = fopen(filename, vFOPEN_WRITE);
    if (fp == 0) return 0;
    ushort success;
    success = header.Write(fp);
    if (success == 0) result = 0;
    success = WriteBandwise(fp);
    if (success == 0) result = 0;
    fclose(fp);
  }
  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    result = (ushort) function_save_grayscale_bitmap(this, filename);
  }
  else if (vStringCaseEndsIn(filename, ".bmp"))
  {
    result = store(filename);
  }
  return result;
}


Type vSuperType(Type type)
{
  switch(type)
  {
  case ScharType:
  case ShortType:
  case IntType:
  case Vint8Type:
    return Vint8Type;

  case UcharType:
  case UshortType:
  case UintType:
  case UlongType:
    return UlongType;

  case FloatType:
  case DoubleType:
    return DoubleType;
    
  default:
    exit_error("\nerror: unexpected argument in vSuperType\n");
    return OtherType;
  }
}


vArray(uchar) vImageToRGBA(color_image * the_image)
{
  vint8 buffer_size = the_image->Size() * 4;
  vArray(uchar) buffer = vnew(uchar, (vector_size) buffer_size);
  vImageToRGBA(the_image, buffer);
  return buffer;
}


void vImageToRGBA(color_image * the_image, vArray(uchar) buffer)
{
  vArray(uchar) R = the_image->GetR();
  vArray(uchar) G = the_image->GetG();
  vArray(uchar) B = the_image->GetB();
  vint8 size = the_image->Size();
  vint8 i;
  vint8 index = 0;

  for (i = 0; i < size; i++)
  {
    buffer[index++] = R[i];
    buffer[index++] = G[i];
    buffer[index++] = B[i];
    buffer[index++] = 0;
  }
}

vArray(uchar) vImageToRGB(color_image * the_image)
{
  vint8 buffer_size = the_image->Size() * 3;
  vArray(uchar) buffer = vnew(uchar, (vector_size) buffer_size);
  vImageToRGB(the_image, buffer);
  return buffer;
}


void vImageToRGB(color_image * the_image, vArray(uchar) buffer)
{
  vArray(uchar) R = the_image->GetR();
  vArray(uchar) G = the_image->GetG();
  vArray(uchar) B = the_image->GetB();
  vint8 size = the_image->Size();
  vint8 i;
  vint8 index = 0;

  for (i = 0; i < size; i++)
  {
    buffer[index++] = R[i];
    buffer[index++] = G[i];
    buffer[index++] = B[i];
  }
}

 
color_image * vRGBToImage(vArray(uchar) buffer, long rows, long cols)
{
  color_image * the_image = new color_image(rows, cols);
  vRGBToImage2(buffer, the_image);
  return the_image;
}


void vRGBToImage2(vArray(uchar) buffer, color_image  * the_image)
{
  vArray(uchar) R = the_image->GetR();
  vArray(uchar) G = the_image->GetG();
  vArray(uchar) B = the_image->GetB();
  
  vint8 size = the_image->Size();

  vint8 band_index;
  vint8 buffer_index = 0;
  for (band_index = 0; band_index < size; band_index++)
  {
    R[band_index] = buffer[buffer_index++];
    G[band_index] = buffer[buffer_index++];
    B[band_index] = buffer[buffer_index++];
  }  
}


// Same as vRGBToImage2, but here the buffer is not covering
// necessarily the whole target image. The arguments rows and cols
// specify the size of the image described by the buffer. The
// buffer data will be put to a target region of the same size, whose
// top-left corner is the origin. Returns 0 (false) if something
// goes wrong, 1 (true) otherwise.
ushort vRGBToImage3(vArray(uchar) buffer, color_image  * the_image,
                     long rows, long cols)
{
  vint8 target_rows = the_image->Rows();
  vint8 target_cols = the_image->Cols();
  if (rows >= target_rows) return 0;
  if (cols >= target_cols) return 0;

  vArray2(uchar) R2 = the_image->GetR2();
  vArray2(uchar) G2 = the_image->GetG2();
  vArray2(uchar) B2 = the_image->GetB2();
  
  vint8 buffer_index = 0;
  vint8 row, col;
  for (row = 0; row < rows; row++)
  {
    vArray(uchar) R = R2[row];
    vArray(uchar) G = G2[row];
    vArray(uchar) B = B2[row];
    for (col = 0; col < cols; col++)
    {
      R[col] = buffer[buffer_index++];
      G[col] = buffer[buffer_index++];
      B[col] = buffer[buffer_index++];
    }
  }
  return 1;
}


// Returns an image whose pixel values are uniformly distributed
// between 0 and 255.
color_image * vRandomImage(long rows, long cols)
{
  srand((unsigned)time(NULL));
  color_image * the_image = new color_image(rows, cols);
  vArray(uchar) matrix = the_image->Matrix();
  long size = rows * cols * 3;
  long i;
  for (i = 0; i < size; i++)
  {
    matrix[i] = (uchar) rand() % 256;
  }
  return the_image;
}


ushort predicate_color_image(general_image * base)
{
  if ((base->TypeName() == UcharType) && 
      (base->Bands()) == 3)
    return 1;
  else return 0;
}


ushort predicate_gray_image(general_image * base)
{
  if ((base->TypeName() == UcharType) && 
      (base->Bands()) == 1)
    return 1;
  else return 0;
}


long vDebug_4D_Array()
{
  long success = 1;
  long dim1 = 0, dim2 = 0, dim3 = 0, dim4 = 0, i1 = 0, i2 = 0, i3 = 0, i4 = 0;
  long dim1_low = 0, dim2_low = 0, dim3_low = 0, dim4_low = 0;
  long dim1_high = 0, dim2_high = 0, dim3_high = 0, dim4_high = 0;
  double value = 0.0;

  v4dArray<double> * array = new v4dArray<double>(1,2,10,12,100,104,1000,1002);
  array->Zero();
  vArray4(double) data = array->Data4();
  char command[10];
  long break_flag = 0;
  while(break_flag == 0)
  {
    vScan("%s", command);
    switch(command[0])
    {
    case 'n':
      vScan("%li %li %li %li", &dim1, &dim2, &dim3, &dim4);
      vdelete(array);
      array = new v4dArray<double>(dim1, dim2, dim3, dim4);
      array->Zero();
      data = array->Data4();
      break;

    case 'N':
      vScan("%li %li %li %li %li %li %li %li", &dim1_low, &dim1_high,
             &dim2_low, &dim2_high, &dim3_low, &dim3_high, &dim4_low, &dim4_high);
      vdelete(array);
      array = new v4dArray<double>(dim1_low, dim1_high, dim2_low, dim2_high, 
                                     dim3_low, dim3_high, dim4_low, dim4_high);
      array->Print();
      break;

    case 'w':
      vScan("%li %li %li %li %lf", &i1, &i2, &i3, &i4, &value);
      data[i1][i2][i3][i4] = value;
      array->Print();
      break;

    case 's':
      success = (long) array->Write("D:\\users\\athitsos\\trash.bin");
      vPrint("success = %li\n", success);
      break;

    case 'l':
      vdelete(array);
      array = v4dArray<double>::Read("D:\\users\\athitsos\\trash.bin");
      if (array == 0)
      {
        vPrint("array is zero\n");
      }
      else
      {
        array->Print();
      }
      break;

    case 'q':
      break_flag = 1;
      break;
    }
  }
  vdelete(array);
  return 1;
}


// This function writes a header into a binary output file
// accessible via fp. This is useful when the matrix that
// we want to write is too large to fit into memory, so 
// we cannot just create it and save it, instead we have
// to create it piece-by-piece.
vint8 vWriteHeader(FILE * fp, Type type_name, vint8 rows, 
                   vint8 cols, vint8 bands)
{
  vint8 type_number = vTypeToVint8(type_name);

  // write header
  vector<vint8> header(4);
  header[0] = type_number;
  header[1] = rows;
  header[2] = cols;
  header[3] = bands;

  vint8 success = 1;
  vint8 temp;
  
  // the <float> here doesn't matter, the function should not 
  // be a template function, it doesn't use the type anywhere.
  temp = v3dMatrix<float>::WriteHeader(fp, &header);
  if (temp <= 0)
  {
    function_warning("Warning: failed to write header\n");
    success = 0;
  }

  return success;
}


long print_type_name(Type type_name)
{
  switch(type_name)
  {
  case ScharType:
    function_print("signed char type");
    break;

  case ShortType:
    function_print("signed short type");
    break;

  case IntType:
    function_print("signed int type");
    break;

  case Vint8Type:
    function_print("signed long type");
    break;

  case FloatType:
    function_print("float type");
    break;

  case DoubleType:
    function_print("double type");
    break;

  case UcharType:
    function_print("unsigned char");
    break;

  case UshortType:
    function_print("unsigned short type");
    break;

  case UintType:
    function_print("unsigned int type");
    break;

  case UlongType:
    function_print("unsigned long type");
    break;

  case OtherType:
    function_print("other type");
    break;

  default:
    exit_error("error: unexpected type in print_type_name\n");
    return 0;
  }

  return 1;
}
