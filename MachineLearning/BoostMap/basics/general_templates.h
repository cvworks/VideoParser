#ifndef VASSILIS_GENERAL_TEMPLATES_H
#define VASSILIS_GENERAL_TEMPLATES_H

#include "vplatform.h"
vint8 exit_error(char * format, ...);

//*********************************************************************
//
// This file declares classes that make it easier to detect memory
// leaks and other memory management problems.
//
// vMemoryCheck should be used as a base class for all complex classes.
// It gives error messages if it never gets deleted or it gets deleted 
// twice.
//
// class_unique should also be used as a base class for all complex classes
// (and inherits from vMemoryCheck). A class_unique object knows how many 
// other objects have been set equal to it, and (assuming the derived class
// implements vdelete_unique the right way) its destructor deletes
// data that it shares
// with other objects only when all those other objects have been deleted.
// Inherited objects should simply call delete_unique in their destructor
// or whenever they become useless in the program and can be safely 
// discarded.
//
// Arrays:
//
// Depending on the compile flag VASSILIS_USE_vArray, if the flag is defined
// all vArray types become equivalent to vArrayClass, with initialization 
// checks, bound checks and all that stuff. If the flag is not defined,
// they become regular pointers, which, of course, is faster.
//
//*********************************************************************

#include <list>

#include "auxiliaries.h"

enum Type {ScharType, ShortType, IntType, Vint8Type, FloatType, DoubleType,
	   UcharType, UshortType, UintType, UlongType, OtherType};


class class_unique
{
protected:
  long * count;

protected:
  virtual void delete_unique() = 0;

public:
  class_unique();
  class_unique(class_unique const & unique);
  ~class_unique();

  void remove_reference();
  void operator = (class_unique const & unique);
};


class vArrayClassBase : public class_unique
{
public:
  virtual void Initialize(long dimensions, long * lengths) = 0;
};


template<class type>
class vArrayClass : public vArrayClassBase
{
private:
  type * data;
  uchar * init_flags;

  long length;
  long low, high;

  void delete_unique();
  void InitializeLength();
  
public:
  vArrayClass();
  explicit vArrayClass(long in_length);
  vArrayClass(long in_length, type * in_data);
  vArrayClass(long in_low, long in_high);
  ~vArrayClass();
  void Zero();
  
  void Initialize(long dimensions, ...);
  void Initialize(long dimensions, long * lengths);
  void SetLength(long in_length);
  void SetRange(long in_low, long in_high);

  inline type & operator [] (long index)
  {
    if ((index < low) || (index > high))
    {
      exit_error("Error, bad index passed to array. (%li, %li) %li\n", 
                      low, high, index);
    }
    return data[index-low];
  }
//  type & operator [](long index);
  
  long Length();
  type * Data();

  vArrayClass<type> operator - (long items);
  vArrayClass<type> operator + (long items);
};





#ifdef VASSILIS_USE_vArray

//*************************************************************
// 
// The following definitions are used when we want vArray to mean
// vArrayClass.
//
//*************************************************************

#define vArray(type) vArrayClass< type >
#define vArray2(type) vArrayClass<vArrayClass< type > >
#define vArray3(type) vArrayClass<vArray2(type) >
#define vArray4(type) vArrayClass<vArray3(type) >
#define vArray5(type) vArrayClass<vArray4(type) >

#define class_pointer vArray
#define matrix_pointer vArray2
#define three_pointer vArray3

#define vnew(type, size) vArrayClass<type >(size)
#define vData(array) array.Data()
#define actual_pointer(array) array.Data()

template<class type>
vArray(type) vToArray(long length, type * data)
{
  return vArray(type)(length, data);
}

template<class type>
inline void vdelete2(type *& pointer)
{
  delete [] pointer;
  pointer = 0;
}

template<class type>
inline void delete_pointer(vArrayClass<type> & array)
{
  vSetZero(array);
}

template<class type>
inline void delete_pointer(type *& pointer)
{
  delete [] pointer;
  pointer = 0;
}

template<class type>
inline void vdelete2(vArrayClass<type> & array)
{
  vSetZero(array);
}

#define function_zero(type) (vZeroArray<type >())
#define vZero(type) (vZeroArray<type >())

#else

//*************************************************************
// 
// The following definitions are used when we want vArray to mean
// simply pointer.
//
//*************************************************************

#define vArray(type) type *
#define vArray2(type) type **
#define vArray3(type) type ***
#define vArray4(type) type ****
#define vArray5(type) type *****

#define class_pointer(type) type *
#define matrix_pointer(type) type **
#define three_pointer(type) type ***

#define vnew(type, size) (new type[size])
#define vData(array) (array)
#define actual_pointer(array) (array)
#define vToArray(size, data) (data)

#define vdelete2(array) delete [] (array);
#define delete_pointer(array) delete [] (array);
#define vZero(type) ((type *) 0)
#define vZero(type) ((type *) 0)
#define function_zero(type) ((type *) 0)

#endif // VASSILIS_USE_vArray


template<class type>
void vHyperArray2(vArrayClass<type> * array, long dim1, long dim2);

template<class type>
void vHyperArray2(type *** array, long dim1, long dim2);

template<class type>
void vHyperArray3(vArrayClass<type> * array, 
                   long dim1, long dim2, long dim3);

template<class type>
void vHyperArray3(type **** array,
                   long dim1, long dim2, long dim3);

template<class type>
void vHyperArray4(type ***** array,
                   long dim1, long dim2, long dim3, long dim4);

template<class type>
void vHyperArray5(type ****** array,
                   long dim1, long dim2, long dim3, long dim4, long dim5);

template<class type>
ushort vIsZero(vArrayClass<type> & array);

template<class type>
ushort vIsZero(type * pointer);

template<class type>
ushort predicate_zero(vArrayClass<type> & array)
{
  return vIsZero(array);
}

template<class type>
ushort predicate_zero(type * pointer)
{
  return vIsZero(pointer);
}

template<class type>
ushort vIsZero(type * pointer);

template<class type>
vArrayClass<type> vZeroArray()
{
  return vArrayClass<type>();
}

template<class type>
void vDelete2(vArrayClass<vArrayClass<type> > & array, vint8 dim1);

template<class type>
void vDelete2(type ** array, vint8 dim1);

template<class type>
void vDelete3(vArrayClass<vArrayClass<vArrayClass<type> > > & array, 
               vint8 dim1, vint8 dim2);

template<class type>
void vDelete3(type *** array, vint8 dim1, vint8 dim2);


template<class type>
void vPrintVector(std::vector<type> * items, long items_per_line = 1);


// Start of mathematical template functions
// Returns the maximum of arg1 and arg2.
template <class number>
inline number Max(number arg1, number arg2)
{
  if (arg1 > arg2) return arg1;
  return arg2;
}

// Returns the minimum of arg1 and arg2.
template <class number>
inline number Min(number arg1, number arg2)
{
  if (arg1 < arg2) return arg1;
  return arg2;
}

template <class number>
inline number vAbs(number arg1)
{
  if (arg1 >= 0) return arg1;
  return -arg1;
}

// Start of mathematical template functions
// Returns the maximum of arg1 and arg2.
template <class number>
inline number function_maximum(number arg1, number arg2)
{
  if (arg1 > arg2) return arg1;
  return arg2;
}

// Returns the minimum of arg1 and arg2.
template <class number>
inline number function_minimum(number arg1, number arg2)
{
  if (arg1 < arg2) return arg1;
  return arg2;
}

template <class number>
inline number function_absolute(number arg1)
{
  if (arg1 >= 0) return arg1;
  return -arg1;
}

template <class type>
Type vGetType();

// Returns the maximum allowable value of a given type. We can
// pass as argument any expression of that type.
template<class type>
type vTypeMax(type a);

// Returns the minimum allowable value of a given type. We can
// pass as argument any expression of that type.
template<class type>
type vTypeMin(type a);


// Circular distance is useful for quantities where any value x
// is considered identical to x plus an integral multiple of
// some period. An example is angles measured in degrees, where
// angles 10 and angles 350 have a distance of 20.
template<class type>
inline type vCircularDistance(type x, type y, type period)
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
    return -distance2 * factor;
  }
}


template<class type>
inline type circular_distance(type x, type y, type period)
{
  return vCircularDistance(x, y, period);
}

// Circular distance is useful for quantities where any value x
// is considered identical to x plus an integral multiple of
// some period. An example is angles measured in degrees, where
// angles 10 and angles 350 have a distance of 20.
// Directed circular distance means that we must reach y from x
// going in the specified direction (which is >= 0 for positive,
// < 0 for negative). So, for arguments 350, 10, 360, 1, we
// return 20. For arguments 10, 350, 360, -1, we return -20.
template<class type>
inline type vDirectedCircularDistance(type x, type y, type period, 
                                       long direction)
{
  // Make the arguments positive.
  type difference = y - x;
  long periods = (long) (difference / period);
  if ((difference >= 0) && (direction >= 0))
  {
    x = x + periods * period;
  }
  else if ((difference >= 0) && (direction < 0))
  {
    x = x + (periods + 1) * period;
  }
  else if ((difference < 0) && (direction >= 0))
  {
    x = x + (periods - 1) * period;
  }
  else 
  {
    x = x + (periods * period);
  }

  type result = y - x;
  return result;
}


static inline double vSquareDistance(double x1, double y1, double x2, double y2)
{
  double diff1 = x1 - x2;
  double diff2 = y1 - y2;
  return (diff1 * diff1 + diff2 * diff2);
}

static inline double vDistance(double x1, double y1, double x2, double y2)
{
  return sqrt(vSquareDistance(x1, y1, x2, y2));
}


// End of mathematical template functions


template<class type>
inline void vFourNeighbors(long row, long col, 
                            type * rows, type * cols)
{
  rows[0] = row;
  rows[1] = row;
  rows[2] = row-1;
  rows[3] = row+1;

  cols[0] = col-1;
  cols[1] = col+1;
  cols[2] = col;
  cols[3] = col;
}


template <class type>
long vector_delete_data (std::vector <type> * data_vector, long index);



#ifndef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#include "general_templates.cpp"
#endif // VASSILIS_SGI_PLATFORM

#endif // VASSILIS_GENERAL_TEMPLATES_H
