/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VPL_POINT_H_
#define _VPL_POINT_H_

#include <iostream>
#include "MathUtils.h"

namespace vpl {

//! 2D Point with coordinates of type double
struct Point 
{
	double x, y;

	Point() { x = 0; y = 0; }

	Point(const double& xx, const double& yy)    { x = xx; y = yy; }
	void Set(const double& xx, const double& yy) { x = xx; y = yy; }

	friend std::ostream& operator<<(std::ostream& os, const Point& p)
	{
		os << " (" << p.x << ", " << p.y << ")";
		//os << p.x << ", " << p.y;
		return os;
	}

	Point& operator=(const Point& rhs)
	{
		x = rhs.x;
		y = rhs.y;

		return *this;
	}

	void Get(double* pX, double* pY) const
	{
		*pX = x;
		*pY = y;
	}

	bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Point& rhs) const { return !operator==(rhs); }

	Point operator/(const double& k) const  { return Point(x / k, y / k); }
	Point operator*(const double& k) const  { return Point(x * k, y * k); }

	void operator/=(const double& k) { x /= k; y /= k; }
	void operator*=(const double& k) { x *= k; y *= k; }

	Point operator-(const Point& rhs) const { return Point(x - rhs.x, y - rhs.y); }
	Point operator+(const Point& rhs) const { return Point(x + rhs.x, y + rhs.y); }
	
	void operator-=(const Point& rhs) { x -= rhs.x; y -= rhs.y; }
	void operator+=(const Point& rhs) { x += rhs.x; y += rhs.y; }

	//! Computes the L2 norm of the point
	double L2() const { return sqrt(x * x + y * y); }

	double SqNorm() const { return x * x + y * y; }

	//! Computes the Euclidean distance between the points
	double L2(const Point& p1) const
	{
		double dx = x - p1.x;
		double dy = y - p1.y;
		return sqrt(dx * dx + dy * dy);
	}

	//! Rounds the coordinates of the point
	void Round()
	{
		x = (x < 0) ? -(int)(0.5 - x) : (int)(x + 0.5);
		y = (y < 0) ? -(int)(0.5 - y) : (int)(y + 0.5);
	}

	void GetRoundedCoords(int* pXCoord, int* pYCoord) const
	{
		*pXCoord = (x < 0) ? -(int)(0.5 - x) : (int)(x + 0.5);
		*pYCoord = (y < 0) ? -(int)(0.5 - y) : (int)(y + 0.5);
	}

	//! Computes the squared Euclidean distance between the points
	double SqDist(const Point& rhs) const
	{
		double dx = x - rhs.x;
		double dy = y - rhs.y;
		return dx * dx + dy * dy;
	}

	//! Computes the squared Euclidean distance between the points
	double SqDist(const double& rhs_x, const double& rhs_y) const
	{
		double dx = x - rhs_x;
		double dy = y - rhs_y;
		return dx * dx + dy * dy;
	}

	double Norm() const
	{
		return sqrt(x * x + y * y);
	}
	
	//! Returns true iff the rounded coordinates of both points are equal
	bool RoundedIsEqual(const Point& rhs) const 
	{ 
		int this_x, this_y, rhs_x, rhs_y;

		GetRoundedCoords(&this_x, &this_y);
		rhs.GetRoundedCoords(&rhs_x, &rhs_y);

		return (this_x == rhs_x && this_y == rhs_y);
	}

	//! dot product. The dot product between perpendicular vectors is zero.
	double Dot(const Point& b) const { return x * b.x + y * b.y; }

	/*! 
		@brief perp product. The perp product betwen parallel vectores is zero.

	    The perp product is the dot product between a perpendicular vector
	    to (*this) and vector b. 
	*/
	double Perp(const Point& b) const { return x * b.y - y * b.x; }

	double Cosine() const
	{
		double cosine = x / norm();

		// Rounding errors might cause issues, so fix them
		if (cosine > 1) 
			return 1;
		else if (cosine < -1) 
			return -1;
		else
			return cosine;
	}

	double Atan2() const
	{
		return atan2(y, x);
	}

	//////////////////////////////////////////////////////////////////////////
	// Compatibility functions for old Point class in the FluxSkeleton library
	static double sqDist(const double& x1, const double& y1, 
		const double& x2, const double& y2)
	{
		const double dx = x1 - x2;
		const double dy = y1 - y2;

		return (dx * dx + dy * dy);
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

    /*friend std::ostream &operator<< (std::ostream &os, const Point& p){
      os << " (" << p.x << ", " << p.y << ")";
      return os;
    }*/
};

}

#endif // _VPL_POINT_H_