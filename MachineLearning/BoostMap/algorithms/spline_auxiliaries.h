#ifndef VASSILIS_SPLINE_AUXILIARIES_H
#define VASSILIS_SPLINE_AUXILIARIES_H

#include "basics/angles.h"

class spline_point
{
public:
  float vertical;
  float horizontal;
  float radians;
  long width;
  long category;
  float parameter; // the spline parameter that corresponds to this point

  spline_point (float in_vertical, float in_horizontal, long in_width, 
                float in_radians, float in_parameter)
  {
    vertical = in_vertical;
    horizontal = in_horizontal;
    width = in_width;
    radians = in_radians;
    parameter = in_parameter;
  }

  long location_equal (spline_point & argument)
  {
    if ((vertical == argument.vertical) && (horizontal == argument.horizontal))
    {
      return 1;
    }
    return 0;
  }

  long print ()
  {
    printf ("vertical = %f, horizontal = %f, width = %li, angle = %f (%f degrees) \n",
                    vertical, horizontal, width, radians, radians / M_PIf * 180.0f);
    return 1;
  }

};


// similar to class spline_point, except that spline_pixel
// uses integers for the coordinates, and it is not designed for
// subpixel accuracy.
class spline_pixel
{
public:
  long vertical;
  long horizontal;
  long width;
  long category;
  float radians;
  float parameter;

  float vertical_float;
  float horizontal_float;
  
  // to keep track of the spline the pixel came from when
  // we have multiple splines as, for example, in a frame_annotation object.
  long spline_index; 


  long initialize();
  spline_pixel();
  spline_pixel(long in_vertical, long in_horizontal, long in_width, 
               float in_radians, float in_parameter);
  spline_pixel(long in_vertical, long in_horizontal, float in_radians);
  long location_equal(spline_pixel & argument);
  long print();
  long save(file_handle * file_pointer);
  long load(file_handle * file_pointer);
};






#endif  //  VASSILIS_SPLINE_AUXILIARIES_H
