/**************************************************************************

   File:                Vector.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/11/20 22:03:03 $

   Description: A simple 2D vector with dot and cross products.

   $Revision: 1.3 $

   $Log: Vector.h,v $
   Revision 1.3  2002/11/20 22:03:03  pdimit
   A LOT has changed since last time.

   Revision 1.2  2002/06/27 14:14:40  pdimit
   Just updated the descriptions in the header files.

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef VECTOR_H
#  define VECTOR_H

/**********************
     Include Files
 **********************/

/**********************
     Public Defines
 **********************/

/**********************
      Public Types
 **********************/

/********************
   Public Variables 
 ********************/

#include <Tools/MathUtils.h>
#include "defines.h"
#include "Point.h"

namespace sg{
  
  // concrete class
  class Vector{
  public:
    double x, y;

    // constructors
    Vector() : x(0.0), y(0.0) {}

	Vector(const double& xx, const double& yy) : x(xx), y(yy) { }

	Vector(const Point& rhs) : x(rhs.x), y(rhs.y) { }
	
	Vector& operator=(const Point& rhs)
	{
		x = rhs.x;
		y = rhs.y;

		return (*this);
	}

	void set(const double& xx, const double& yy) 
	{ 
		x = xx; 
		y = yy; 
	}

	Vector operator+(const Vector& rhs) const
	{
		return Vector(x + rhs.x, y + rhs.y);
	}

	Vector operator-(const Vector& rhs) const
	{
		return Vector(x - rhs.x, y - rhs.y);
	}

	Vector operator-() const
	{
		return Vector(-x, -y);
	}

	void operator/=(const double& k) 
	{ 
		x /= k; 
		y /= k; 
	}

	void operator*=(const double& k) 
	{ 
		x *= k; 
		y *= k; 
	}

	double sqDist(const Vector& v) const
	{
		return EUCSQDIST(x, y, v.x, v.y);
	}

    int operator==(const Vector &p) const {
      return x==p.x && y==p.y;
    }
    
    bool isClose(Vector p, double epsilon) const {
      return (EUCSQDIST(x,y, p.x,p.y) < (epsilon));
    }

    double norm() const { return sqrt(x * x + y * y); }
    double dot(const Vector& v) const { return x * v.x + y * v.y; }

    Vector getPerp() { return Vector(y, -x); }

    // state changing
    void normalize() 
	{ 
      double n = norm(); 

      if(n != 0)
	  {
		  x = x / n;
		  y = y / n;
      }
    }

    void scale(double s) { x = s*x; y = s*y;}

    void rotate(const double& a) 
	{
      double ox = x, oy = y;
	  double cos_a = cos(a);
	  double sin_a = sin(a);

      x = ox * cos_a - oy * sin_a;
      y = ox * sin_a + oy * cos_a;
    }

    friend std::ostream& operator<< (std::ostream &Out, const Vector &p){
      Out << "(" << p.x << ", " << p.y << ")";
      return Out;
    }

	DECLARE_BASIC_MEMBER_SERIALIZATION
  }; //end of class Vector
  
}

#endif  /* VECTOR_H */
