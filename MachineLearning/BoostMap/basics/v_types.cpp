
#include <ostream>

#include "v_types.h"
#include "definitions.h"

using namespace std;

const vint4 vas_int8::limit = 2000000000;  // 2 billion

inline vint4 S_Add(vint4 number1, vint4 number2, vint4 & carry)
{
  static const vint4 limit = vas_int8::limit;
  carry = 0;

  // Adjust carry and number1
  if (number1 >= limit)
  {
    carry++;
    number1 = number1 - limit;
  }
  else 
  {
    if (number1 <= -limit)
    {
      carry--;
      number1 = number1 + limit;
    }
  
    if (number1 < 0)
    {
      carry--;
      number1 = number1 + limit;
    }
  }

  // Adjust carry and number2
  if (number2 >= limit)
  {
    carry++;
    number2 = number2 - limit;
  }
  else 
  {
    if (number2 <= -limit)
    {
      carry--;
      number2 = number2 + limit;
    }
  
    if (number2 < 0)
    {
      carry--;
      number2 = number2 + limit;
    }
  }

  if (limit - number1 <= number2)
  {
    carry++;
    number1 = number1 - limit;
  }
  return number1 + number2;
}

vas_int8 operator + (vas_int8 number1, vas_int8 number2)
{
  vas_int8 result;
  vint4 carry;
  
  result.high0 = S_Add(number1.high0, number2.high0, carry);
  result.high1 = number1.high1 + number2.high1 + carry;
  return result;
}

vas_int8 operator - (vas_int8 & number1, vas_int8 & number2)
{
  vas_int8 result;
  vint4 carry;
  
  result.high0 = S_Add(number1.high0, -(number2.high0), carry);
  result.high1 = number1.high1 - number2.high1 + carry;
  return result;
}


void vas_int8::operator ++()
{
  high0++;
  if (high0 == limit)
  {
    high0 -= limit;
    high1++;
  }
}


void vas_int8::operator --()
{
  high0--;
  if (high0 == -1)
  {
    high0 = limit-1;
    high1--;
  }
}


ushort operator == (vas_int8 & number1, vas_int8 & number2)
{
  return ((number1.high0 == number2.high0) &&
          (number1.high1 == number2.high1));
}


ushort operator != (vas_int8 & number1, vas_int8 & number2)
{
  return ((number1.high0 != number2.high0) ||
          (number1.high1 != number2.high1));
}


vas_int8::vas_int8()
{
  high0 = 0;
  high1 = 0;
}


vas_int8::vas_int8(vint4 integer)
{
  high0 = S_Add(0, integer, high1);
}


void vas_int8::Print()
{
  printf("(vas_int8: %li, %li) ", high1, high0);
}


ostream & operator << (ostream & output, vas_int8 & number)
{
  output << "(vas_int8: " << number.high1 << ", " << number.high0 << ") ";
  return output;
}


istream & operator >> (istream & input, vas_int8 & number)
{
  input >> number.high1 >> number.high0;
  return input;
}


vas_int16 operator + (vas_int16 number1, vas_int16 number2)
{
  vas_int16 result;
  vint4 carry, carry2;
  
  result.high0 = S_Add(number1.high0, number2.high0, carry);
  result.high1 = S_Add(number1.high1, carry, carry);
  result.high1 = S_Add(result.high1, number2.high1, carry2);
  result.high2 = S_Add(number1.high2, carry + carry2, carry);
  result.high2 = S_Add(result.high2, number2.high2, carry2);
  result.high3 = number1.high3 + number2.high3 + carry + carry2;
  
  return result;
}

vas_int16 operator - (vas_int16 & number1, vas_int16 & number2)
{
  vas_int16 result;
  vint4 carry;
  
  result.high0 = S_Add(number1.high0, -number2.high0, carry);
  result.high1 = S_Add(number1.high1, carry, carry);
  result.high1 = S_Add(result.high1, -number2.high1, carry);
  result.high2 = S_Add(number1.high2, carry, carry);
  result.high2 = S_Add(result.high2, -number2.high2, carry);
  result.high3 = number1.high3 - number2.high3 + carry;
  return result;
}


void vas_int16::operator ++()
{
  high0++;
  if (high0 == limit)
  {
    high0 -= limit;
    high1++;
    if (high1 == limit)
    {
      high1 -= limit;
      high2++;
      if (high2 == limit)
      {
        high2 -= limit;
        high3++;
      }
    }
  }
}


void vas_int16::operator --()
{
  high0--;
  if (high0 == -1)
  {
    high0 = limit-1;
    high1--;
    if (high1 == -1)
    {
      high1 = limit - 1;
      high2--;
      if (high2 == -1)
      {
        high2 = limit - 1;
        high3--;
      }
    }
  }
}


ushort operator == (vas_int16 & number1, vas_int16 & number2)
{
  return ((number1.high0 == number2.high0) &&
          (number1.high1 == number2.high1) &&
          (number1.high2 == number2.high2) &&
          (number1.high3 == number2.high3));
}


ushort operator != (vas_int16 & number1, vas_int16 & number2)
{
  return ((number1.high0 != number2.high0) ||
          (number1.high1 != number2.high1) ||
          (number1.high2 != number2.high2) ||
          (number1.high3 != number2.high3));
}


vas_int16::vas_int16()
{
  high1 = 0;
  high2 = 0;
  high2 = 0;
  high3 = 0;
}


vas_int16::vas_int16(vint4 integer)
{
  high0 = S_Add(0, integer, high1);
  high1 = S_Add(0, high1, high2);
  high2 = S_Add(0, high2, high3);
}


void vas_int16::Print()
{
  printf("(vas_int16: %i, %i, %i, %i) ", (int) high3, (int) high2, (int) high1, (int) high0);
}


ostream & operator << (ostream & output, vas_int16 & number)
{
  output << "(vas_int16: " << number.high3 << ", " << number.high2 << ", ";
  output << number.high1 << ", " << number.high0 << ") ";
  return output;
}


istream & operator >> (istream & input, vas_int16 & number)
{
  input >> number.high3 >> number.high2 >> number.high1 >> number.high0;
  return input;
}
