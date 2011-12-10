#ifndef VASSILIS_TYPES_H
#define VASSILIS_TYPES_H

#include "vplatform.h"
#ifdef VASSILIS_SGI_PLATFORM
#include <iostream>
#else 
#include <iostream>
#endif


typedef char * charp;
typedef unsigned char uchar;
typedef unsigned char * ucharp;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef void * voidp;

typedef vint4 integer;

class vas_int8
{
public:
  static const vint4 limit;
  vint4 high1;
  vint4 high0;
  vas_int8();
  vas_int8(vint4 integer);

  friend vas_int8 operator + (vas_int8 number1, vas_int8 number2);
  friend vas_int8 operator - (vas_int8 & number1, vas_int8 & number2);
  friend ushort operator == (vas_int8 & number1, vas_int8 & number2);
  friend ushort operator != (vas_int8 & number1, vas_int8 & number2);
  void operator++ ();
  void operator-- ();

  void Print();
  friend std::ostream & operator << (std::ostream & output, vas_int8 & number);
  friend std::istream & operator >> (std::istream & input, vas_int8 & number);
};


class vas_int16 : public vas_int8
{
public:
  vint4 high3;
  vint4 high2;
  vas_int16();
  vas_int16(vint4 integer);
  vas_int16(vas_int8 integer);

  friend vas_int16 operator + (vas_int16 number1, vas_int16 number2);
  friend vas_int16 operator - (vas_int16 & number1, vas_int16 & number2);
  void operator++ ();
  void operator-- ();
  friend ushort operator == (vas_int16 & number1, vas_int16 & number2);
  friend ushort operator != (vas_int16 & number1, vas_int16 & number2);

  void Print();
  friend std::ostream & operator << (std::ostream & output, vas_int16 & number);
  friend std::istream & operator >> (std::istream & input, vas_int16 & number);
};

#endif // VASSILIS_TYPES_H
