#ifndef VASSILIS_SIMPLE_IMAGE_H
#define VASSILIS_SIMPLE_IMAGE_H

#include "vplatform.h"

class simple_float_matrix
{
public:
  long vertical_size; // rows
  long horizontal_size; // cols
  float * data; // contains the image data
  
public:
  simple_float_matrix();
  simple_float_matrix(const long argument_vertical, const long argument_horizontal);
  
  // status is an output argument that can be used to store
  // value 1, if loading from filename was successful,
  // or a value <= 0 if something went wrong.
  simple_float_matrix(const char * filename, vint8 * status = 0);

  ~simple_float_matrix();

  // the return value follows the same conventions as the value of the
  // status argument in the constructor that reads from a filename:
  // 1 for success, and an error code <= 0 in case of failure.
  long save(const char * filename) const;
  
  // returns the index in the data array that corresponds to
  // pixel location (vertical, horizontal). For the top left corner of 
  // the image, vertical = 0, horizontal = 0, and get_index returns 0.
  long get_index(long vertical, long horizontal) const;
};











#endif // VASSILIS_SIMPLE_IMAGE_H
