/**************************************************************************

   File:                Point.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/06/27 14:14:40 $

   Description: A basic 2D point.

   $Revision: 1.2 $

   $Log: Point.h,v $
   Revision 1.2  2002/06/27 14:14:40  pdimit
   Just updated the descriptions in the header files.

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/
#ifndef POINT_H
#define POINT_H

#include <Tools/Point.h>

namespace sg {

typedef vpl::Point Point;

}

/*#include <iostream>
#include <Tools/MathUtils.h>

namespace sg{
  
  // concrete class
class Point
{
public:
	static double sqDist(const double& x1, const double& y1, 
		const double& x2, const double& y2)
	{
		const double dx = x1 - x2;
		const double dy = y1 - y2;

		return (dx * dx + dy * dy);
	}

public:
    double x, y;

    // constructors
    Point() : x(0.0), y(0.0) {}

    Point(const double& xx, const double& yy) { 
		x = xx;
		y = yy; 
	}

    bool operator==(const Point& p) const {
      return (x == p.x && y == p.y);
    }

    bool operator!=(const Point &p) const {
      return !operator==(p);
    }

	Point operator-(const Point& rhs) const
	{
		return Point(x - rhs.x, y - rhs.y);
	}

	Point operator+(const Point& rhs) const
	{
		return Point(x + rhs.x, y + rhs.y);
	}

	Point operator*(const double& k) const  
	{ 
		return Point(x * k, y * k); 
	}

	Point operator/(const double& k) const  
	{ 
		return Point(x / k, y / k); 
	}

	void operator-=(const Point& rhs) 
	{ 
		x -= rhs.x; 
		y -= rhs.y; 
	}

	void operator+=(const Point& rhs) 
	{ 
		x += rhs.x; 
		y += rhs.y; 
	}

	double sqNorm() const 
	{ 
		return x * x + y * y; 
	}

	double norm() const 
	{ 
		return sqrt(x * x + y * y); 
	}

	void set(const double& xx, const double& yy) { 
		x = xx; 
		y = yy; 
	}

    bool isClose(const Point& p, const double& epsilon) {
      return (sqDist(x, y, p.x, p.y) < (epsilon));
    }

    double distanceToPt(const Point& p) const { 
      return sqrt(sqDist(x, y, p.x, p.y)); 
    }

	double dist(const Point& p) const { 
      return sqrt(sqDist(x, y, p.x, p.y)); 
    }

	double sqDist(const Point& p) const
	{
		return sqDist(x, y, p.x, p.y);
	}

	double sqDist(const double& rhs_x, const double& rhs_y) const
	{
		return sqDist(x, y, rhs_x, rhs_y);
	}

	//! The dot product of the poins when seen as vectors.
	double dot(const Point& b) const { return x * b.x + y * b.y; }

    friend std::ostream &operator<< (std::ostream &os, const Point& p){
      os << " (" << p.x << ", " << p.y << ")";
      return os;
    }

  }; //end of class Point
  
}*/

#endif  // POINT_H
