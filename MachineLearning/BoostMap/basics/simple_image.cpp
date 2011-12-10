// just comment out the next line if it gives errors
// it is needed for Visual Studio projects using precompiled headers.

#include <stdio.h>

#include "simple_image.h"


simple_float_matrix::simple_float_matrix()
{
  vertical_size = 0;
  horizontal_size = 0;
  data = 0;
}


simple_float_matrix::simple_float_matrix(const long argument_vertical, 
                                         const long argument_horizontal)
{
  vertical_size = argument_vertical;
  horizontal_size = argument_horizontal;
  data = new float[vertical_size * horizontal_size];
}


// status is an output argument that can be used to store
// value 1, if loading from filename was successful,
// or a value <= 0 if something went wrong.
simple_float_matrix::simple_float_matrix(const char * filename, vint8 * status)
{
  vertical_size = 0;
  horizontal_size = 0;
  data = 0;

  // FILE * file_pointer = fopen(filename, "r"); // UNIX/Linux version
  FILE * file_pointer = fopen(filename, "rb"); // Visual Studio version
  if (file_pointer == 0)
  {
    if (status != 0)
    {
      *status = 0; // error code for failing to open filename
    }
    return;
  }

  long header[4];

  long items;
  items = fread(header, sizeof(long), 4, file_pointer);
  if (items != 4)
  {
    if (status != 0)
    {
      *status = -1; // error code for failing to read the header
    }
    fclose(file_pointer);
    return;
  }

  if ((header[0] != 4) || (header[1] <= 0) || (header[2] <= 0) || (header[3] != 1))
  {
    if (status != 0)
    {
      *status = -2; // error code for invalid header
    }
    fclose(file_pointer);
    return;
  }

  vertical_size = header[1];
  horizontal_size = header[2];
  long total_size = vertical_size * horizontal_size;
  data = new float[total_size];

  items = fread(data, sizeof(float), total_size, file_pointer);
  if (items != total_size)
  {
    if (status != 0)
    {
      *status = -3; // error code for failing to read the image data
    }
    fclose(file_pointer);
    return;
  }

  fclose(file_pointer);
  if (status != 0)
  {
    *status = 1;
  }
}


simple_float_matrix::~simple_float_matrix()
{
  delete [] data;
  vertical_size = 0;
  horizontal_size = 0;
  data = 0;
}

  
// the return value follows the same conventions as the value of the
// status argument in the constructor that reads from a filename:
// 1 for success, and an error code <= 0 in case of failure.
long simple_float_matrix::save(const char * filename) const
{
  // FILE * file_pointer = fopen(filename, "w"); // UNIX/Linux version
  FILE * file_pointer = fopen(filename, "wb"); // Visual Studio version
  if (file_pointer == 0)
  {
    return 0; // error code for failing to open filename
  }

  long header[4];
  header[0] = 4;
  header[1] = vertical_size;
  header[2] = horizontal_size;
  header[3] = 1;

  long items;
  items = fwrite(header, sizeof(long), 4, file_pointer);
  if (items != 4)
  {
    fclose(file_pointer);
    return -1; // error code for failing to read the header
  }

  long total_size = vertical_size * horizontal_size;
  items = fwrite(data, sizeof(float), total_size, file_pointer);
  if (items != total_size)
  {
    fclose(file_pointer);
    return -3; // error code for failing to read the image data
  }

  fclose(file_pointer);
  return 1;
}



// returns the index in the data array that corresponds to
// pixel location (vertical, horizontal). For the top left corner of 
// the image, vertical = 0, horizontal = 0, and get_index returns 0.
long simple_float_matrix::get_index(long vertical, long horizontal) const
{
  long result = vertical * horizontal_size + horizontal;
  return result;
}
