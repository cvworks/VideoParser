#ifndef VASSILIS_ANGLES_H
#define VASSILIS_ANGLES_H


#ifndef M_PI 
#define M_PI   ((double)3.1415926535897932384626433832795028841972)
#endif

#ifndef M_PIf
#define M_PIf  ((float)3.1415926535897932384626433832795028841972)
#endif

#ifndef M_2PI 
#define M_2PI   ((double)6.28318530717959)
#endif

#ifndef M_2PIf
#define M_2PIf ((float) 6.28318530717959)
#endif

#include "basics/auxiliaries.h"

// converts orientation to [0,range) range
float canonical_range (float orientation, float range);

// converts orientation to [0,180) range
float canonical_orientation_half (float orientation);

// converts orientation to [0,360) range
float canonical_orientation (float orientation);

float make_degrees_canonical (float radiant_argument);

float make_radians_canonical (float degree_argument);

// signed_circular_distance is useful for quantities where any value x
// is considered identical to x plus an integral multiple of
// some period. An example is angles measured in degrees, where
// angles 10 and angles 350 have a distance of 20.
template<class type>
static type signed_circular_distance(type x, type y, type period)
{
  type little, big, factor;

  if (x < y)
  {
    little = x;
    big = y;
    factor = (type) 1;
  }
  else
  {
    little = y;
    big = x;
    factor = (type) -1;
  }
  type difference = big - little;
  long periods = (long) (difference / period);
  type little1 = little + periods * period;
  type little2 = little1 + period;

  type distance1 = big - little1;
  type distance2 = little2 - big;
  if (distance1 < distance2)
  {
    return distance1 * factor;
  }
  else
  {
    return distance2 * factor;
  }
}


float signed_degrees_distance(float first, float second);

float signed_radians_distance(float first, float second);

// Circular distance is useful for quantities where any value x
// is considered identical to x plus an integral multiple of
// some period. An example is angles measured in degrees, where
// angles 10 and angles 350 have a distance of 20.
template<class type>
static type absolute_circular_distance(type x, type y, type period)
{
  type little, big, factor;

  if (x < y)
  {
    little = x;
    big = y;
    factor = (type) 1;
  }
  else
  {
    little = y;
    big = x;
    factor = (type) -1;
  }
  type difference = big - little;
  long periods = (long) (difference / period);
  type little1 = little + periods * period;
  type little2 = little1 + period;

  type distance1 = big - little1;
  type distance2 = little2 - big;
  if (distance1 < distance2)
  {
    return function_absolute(distance1 * factor);
  }
  else
  {
    return function_absolute(distance2 * factor);
  }
}


// converts orientation to [0,360) range
float canonical_degrees (float degrees);

// converts orientation to [0, 2*pi) range
float canonical_radians (float radians);

float degrees_distance(float first_degrees, float second_degrees);

float radians_distance(float first_radians, float second_radians);

float degrees_distance2(float first_degrees, float second_degrees);

float radians_distance2(float first_radians, float second_radians);

inline float radians_to_degrees(float radians)
{
  return radians * 180.0f / M_PIf;
}

inline float degrees_to_radians(float degrees)
{
  return degrees / 180.0f * M_PIf;
}

inline float make_degrees(float radians)
{
  return radians * 180.0f / M_PIf;
}

inline float make_radians(float degrees)
{
  return degrees / 180.0f * M_PIf;
}


float orientation_average(float first_radians, float second_radians);
float orientation_average(float first_radians, float second_radians,float wght);























#endif // VASSILIS_ANGLES_H
