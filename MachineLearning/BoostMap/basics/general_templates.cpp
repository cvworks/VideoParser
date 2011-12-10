

#ifdef VASSILIS_SGI_PLATFORM
#define VASSILIS_PROCESS_FILE
#endif // VASSILIS_SGI_PLATFORM

#ifdef VASSILIS_PROCESS_FILE
#undef VASSILIS_PROCESS_FILE

#ifndef VASSILIS_WINDOWS_MFC
#include <iostream>
#endif // VASSILIS_WINDOWS_MFC

#include <assert.h>
#include <limits.h>
#include <float.h>

#include "auxiliaries.h"
#include "general_templates.h"

#include "definitions.h"

using namespace std;

// The arg is only used to instantiate the template. I was getting
// wrong behavior (probably a Visual Studio bug) if I didn't use
// an argument.
template<class type>
Type vGetType(type arg)
{
  Type type_name;

  // Next two lines are for debugging.
  type a = (type) 0.5;
  long size = sizeof(type);
  // Check if integer or real type
  if (0 == (type) 0.5)
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
      else if (sizeof(type) == sizeof(long)) 
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
  return type_name;
}


template<class type>
void vArrayClass<type>::InitializeLength()
{
  assert(length >= 0);
  vdelete2(data);
  vdelete2(init_flags);
  if (length > 0)
  {
    data = new type[length];
    init_flags = new uchar[length];
    long i;
    for (i = 0; i < length; i++)
    {
      init_flags[i] = 0;
    }
  }
  else 
  {
    data = 0;
    init_flags = 0;
  }
}


template<class type>
vArrayClass<type>::vArrayClass()
{
  data = 0;
  init_flags = 0;
  length = 0;
}


template<class type>
vArrayClass<type>::vArrayClass(long in_length)
{
  length = in_length;
  data = 0;
  init_flags = 0;
  InitializeLength();
  low = 0;
  high = length - 1;
}


template<class type>
vArrayClass<type>::vArrayClass(long in_length, type * in_data)
{
  length = in_length;
  assert(length > 0);
  data = 0;
  init_flags = 0;
  
  InitializeLength();  
  low = 0;
  high = length - 1;
}


template<class type>
vArrayClass<type>::vArrayClass(long in_low, long in_high)
{
  low = in_low;
  high = in_high;
  length = high - low + 1;
  data = 0;
  init_flags = 0;
  InitializeLength();
}


template<class type>
vArrayClass<type>::~vArrayClass()
{
  remove_reference();
}


template<class type>
void vArrayClass<type>::Zero()
{
  length = 0;
}


template<class type>
void vArrayClass<type>::delete_unique()
{
  vdelete2(data);
  vdelete2(init_flags);
}


//template<class type>
//type & vArrayClass<type>::operator [](long index)
//{
//  assert((index >= low) && (index <= high));
//  return data[index-low];
//}


template<class type>
void vArrayClass<type>::Initialize(long dimensions, ...)
{
  va_list arguments;
  long dimension = 0;
  long * lengths = new long[dimensions];
  va_start(arguments, dimensions);
  while(dimension < dimensions)
  {
    lengths[dimension] = va_arg(arguments, long);
    dimension++;
  }
  va_end(arguments);
  Initialize(dimensions, lengths);
  vdelete(lengths);
}


static void S_Initialize(void * array_pointer, long dimensions, long * lengths)
{
  vArrayClassBase * array = (vArrayClassBase *) array_pointer;
  array->Initialize(dimensions, lengths);
}



template<class type>
void vArrayClass<type>::Initialize(long dimensions, long * lengths)
{
  assert(data == 0);
  assert(length == 0);

  length = lengths[0];
  InitializeLength();
  low = 0;
  high = length-1;

  if (dimensions == 1) return;

  long * new_lengths = &(lengths[1]);
  long i;
  for (i = 0; i < length; i++)
  {
    S_Initialize(&(data[i]), dimensions-1, new_lengths);
    init_flags[i] = 1;
  }
}


template<class type>
void vArrayClass<type>:: SetLength(long in_length)
{
  assert(in_length <= length);
  assert(in_length >= 0);
  length = in_length;
  low = 0;
  high = length - 1;
}


template<class type>
void vArrayClass<type>:: SetRange(long in_low, long in_high)
{
  assert(in_low >= low);
  assert(in_high <= high);
  low = in_low;
  high = in_high;
  length = high - low + 1;
}



template<class type>
long vArrayClass<type>::Length()
{
  return length;
}


template<class type>
type * vArrayClass<type>::Data()
{
  return data - low;
}


template<class type>
vArrayClass<type> vArrayClass<type>::operator - (long items)
{
  vArrayClass<type> result = *this;
  result.low = low + items;
  result.high = high + items; 
  return result;
}


template<class type>
vArrayClass<type> vArrayClass<type>::operator + (long items)
{
  vArrayClass<type> result = *this;
  result.low = low - items;
  result.high = high - items; 
  return result;
}


template<class type>
void vHyperArray2(vArrayClass<type> * array, long dim1, long dim2)
{
  array->Initialize(2, dim1, dim2);
}


template<class type>
void vHyperArray3(vArrayClass<type> * array, 
                   long dim1, long dim2, long dim3)
{
  array->Initialize(3, dim1, dim2, dim3);
}


template<class type>
void vHyperArray4(vArrayClass<type> * array, 
                   long dim1, long dim2, long dim3, long dim4)
{
  array->Initialize(3, dim1, dim2, dim3, dim4);
}


template<class type>
void vHyperArray5(vArrayClass<type> * array, 
                   long dim1, long dim2, long dim3, long dim4, long dim5)
{
  array->Initialize(3, dim1, dim2, dim3, dim4, dim5);
}


template<class type>
void vHyperArray2(type *** array, long dim1, long dim2)
{
  *array = new type*[dim1];
  long i;
  for (i = 0; i < dim1; i++)
  {
    (*array)[i] = new type[dim2];
  }
}

template<class type>
void vHyperArray3(type **** array, long dim1, long dim2, long dim3)
{
  *array = new type**[dim1];
  long i;
  for (i = 0; i < dim1; i++)
  {
    vHyperArray2(&((*array)[i]), dim2, dim3);
  }
}


template<class type>
void vHyperArray4(type ***** array,
                   long dim1, long dim2, long dim3, long dim4)
{
  *array = new type***[dim1];
  long i;
  for (i = 0; i < dim1; i++)
  {
    vHyperArray3(&((*array)[i]), dim2, dim3, dim4);
  }
}



template<class type>
void vHyperArray5(type ****** array,
                   long dim1, long dim2, long dim3, long dim4, long dim5)
{
  *array = new type****[dim1];
  long i;
  for (i = 0; i < dim1; i++)
  {
    vHyperArray5(&((*array)[i]), dim2, dim3, dim4, dim5);
  }
}





template<class type>
void vSetZero(vArrayClass<type> & array)
{
  array = vArrayClass<type>();
  array.remove_reference();
}


template<class type>
void vSetZero(type *& pointer)
{
  pointer = 0;
}


template<class type>
ushort vIsZero(vArrayClass<type> & array)
{
  return (array.Length() == 0);
}


template<class type>
ushort function_test_zero(vArrayClass<type> & array)
{
  return (array.Length() == 0);
}


template<class type>
ushort vIsZero(type * pointer)
{
  return (pointer == 0);
}


template<class type>
ushort function_test_zero(type * pointer)
{
  return (pointer == 0);
}


template<class type>
void vDelete2(vArrayClass<vArrayClass<type> > & array, vint8 dim1)
{
  assert(dim1 == array.Length());
  vint8 i;
  for (i = 0; i < dim1; i++)
  {
    vdelete2(array[i]);
  }
  vdelete2(array);
}

template<class type>
void vDelete2(type ** array, vint8 dim1)
{
  if (array == 0)
  {
    return;
  }
  vint8 i;
  for (i = 0; i < dim1; i++)
  {
    vdelete2(array[i]);
  }
  vdelete2(array);
}

template<class type>
void vDelete3(vArrayClass<vArrayClass<vArrayClass<type> > > & array, 
               vint8 dim1, vint8 dim2)
{
  assert(dim1 == array.Length());
  vint8 i;
  for (i = 0; i < dim1; i++)
  {
    vDelete2(array[i], dim2);
  }
  vdelete2(array);
}


template<class type>
void vDelete3(type *** array, vint8 dim1, vint8 dim2)
{
  vint8 i;
  for (i = 0; i < dim1; i++)
  {
    vDelete2(array[i], dim2);
  }
  vdelete2(array);
}


// vTypeMax and vTypeMin find the maximum and minimum allowable 
// values for given types. The assumptions are:
// - An unsigned integer is maxed when all its bits are 1.
// - A signed integer is maxed when its first bit is 0 and the rest are one.
// - The minimum possible value for an integer is negative its maximum value.
//
// For the float and double types, those assumptions are not true, so we
// just return some values predefined in <limits.h>, which are guaranteed
// to be valid values for those types, but are not necessarily the min 
// and max values. 

// I define the float and double cases separately, because otherwise I get
// compiler warnings in returning FLT_MAX and DBL_MAX when the templates 
// are instantiated with integer types.
static float vTypeMax(float a)
{
  return FLT_MAX;
}

static double vTypeMax(double a)
{
  return DBL_MAX;
}


template<class type>
type vTypeMax(type a)
{
  type result;
  
  if (0.5 == (type) 0.5)
  {
    if (sizeof(type) == sizeof(float)) 
    {
      // We should never get to this line, because if the type is a float,
      // the compiler should call the vTypeMax function that is explicitily
      // defined for floats (Annotated C++ Reference Manual, on order in 
      // which the compiler looks for matches to a function).
      assert(0);
    }
    else if (sizeof(type) == sizeof(double)) 
    {
      // See comment for the float case.
      assert(0);
    }
    else return 0;
  }

  if (0 > (type) -1) result = 0;
  else result = 1;

  long type_size = sizeof(type);
  long bit_size = type_size * 8;
  long i;
  for(i = 0; i < bit_size - 1; i++)
  {
    result = result << 1;
    result = result | 1;
  }
  return result;
}

template<class type>
type vTypeMin(type a)
{
  type result;
  if (0 > (type) -1) 
  {
    result = 0 - vTypeMax(a);
    return result;
  }
  else return 0;
}


template<class type>
void vPrintVector(std::vector<type> * items, long items_per_line)
{
  long number = items->size();
  long i;
  for (i = 0; i < number; i++)
  {
    const type entry = (const type) (*items)[i];
    cout << entry;
    if ((i+1) % items_per_line == 0) cout << endl;
    else cout << " ";
  }
  if (i % items_per_line != 0) cout << endl;
}


template <class type>
long vector_delete_data (vector <type> * data_vector, long index)
{
  long number = data_vector->size ();
  if (number == 0) 
  {
    return 0;
  }

  if ((index < 0) || (index >= number))
  {
    exit_error ("error: in vector_delete_data, number = %li, index = %li\n", number, index);
  }

  
  long counter;
  for (counter = index; counter < number-1; counter ++)
  {
    (*data_vector)[counter] = (*data_vector) [counter +1];
  }
  data_vector->pop_back ();

  return 1;
}

  




// If the cpp template file is included as part of the header file,
// we must undefine certain things.
#include "undefine.h"

#endif // VASSILIS_PROCESS_FILE
