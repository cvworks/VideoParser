#ifndef VASSILIS_IMAGE_H
#define VASSILIS_IMAGE_H

#include "auxiliaries.h"
#include "matrix.h"


class color_image : public v3dMatrix<uchar>
{
public:
  color_image() : v3dMatrix<uchar>()
  {
  }
  color_image(color_image * source) : v3dMatrix<uchar>(source)
  {
  }
  color_image(vint8 in_rows, vint8 in_cols);
  ~color_image();

  vArray(uchar) GetR();
  vArray(uchar) GetG();
  vArray(uchar) GetB();
  vArray2(uchar) GetR2();
  vArray2(uchar) GetG2();
  vArray2(uchar) GetB2();

  vArray(uchar) red_pointer();
  vArray(uchar) green_pointer();
  vArray(uchar) blue_pointer();
  vArray2(uchar) red_matrix();
  vArray2(uchar) green_matrix();
  vArray2(uchar) blue_matrix();

  static color_image * load(const char * filename);
  ushort save(const char * filename);
};


class grayscale_image : public v3dMatrix<uchar>
{
public:
  grayscale_image() : v3dMatrix<uchar>()
  {
  }

  grayscale_image(color_image * source) : v3dMatrix<uchar>(source)
  {
  }

  grayscale_image(vint8 in_rows, vint8 in_cols);
  ~grayscale_image();

  vArray(uchar) GetI();
  vArray2(uchar) GetI2();
  static grayscale_image * load(const char * filename);
  ushort save(const char * filename);
};


vArray(uchar) vImageToRGBA(color_image * the_image);

void vImageToRGBA(color_image * the_image, vArray(uchar) buffer);

vArray(uchar) vImageToRGB(color_image * the_image);

void vImageToRGB(color_image * the_image, vArray(uchar) buffer);

// Takes in as argument a image buffer that is of the form
// RGBRGBRGB... and whose first pixel is the top left corner
// of the image (and it proceeds left to right, top to bottom)
// It returns the same image in the color_image format.
color_image * vRGBToImage(vArray(uchar) buffer, vint8 rows, vint8 cols);

// Same as vRGBToImage, but here the target is passed in as an 
// argument
void vRGBToImage2(vArray(uchar) buffer, color_image * the_image);

// Same as vRGBToImage2, but here the buffer is not covering
// necessarily the whole target image. The arguments rows and cols
// specify the size of the image described by the buffer. The
// buffer data will be put to a target region of the same size, whose
// top-left corner is the origin. Returns 0 (false) if something
// goes wrong, 1 (true) otherwise.
ushort vRGBToImage3(vArray(uchar) buffer, color_image * target, 
                   vint8 rows, vint8 cols);

// Returns an image whose pixel values are uniformly distributed
// between 0 and 255.
color_image * vRandomImage(long rows, long cols);

// A small user interface that helps verify that the 
// v4dArray class works OK.
long vDebug_4D_Array();

// This function writes a header into a binary output file
// accessible via fp. This is useful when the matrix that
// we want to write is too large to fit into memory, so 
// we cannot just create it and save it, instead we have
// to create it piece-by-piece.
vint8 vWriteHeader(FILE * fp, Type type_name, vint8 rows, 
                   vint8 cols, vint8 bands);

long print_type_name(Type type_name);



#endif   // VASSILIS_IMAGE_H



