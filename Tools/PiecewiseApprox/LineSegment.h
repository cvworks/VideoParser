/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __LINE_SEGMENT_H__
#define __LINE_SEGMENT_H__

#include "../MathUtils.h"
#include "../SmartArray.h"
#include "../BasicTypes.h"

namespace vpl {

struct LineSegment
{
    double m;	//!< slope
    double b;	//!< y-intercept
    Point p0;	//!< segment endpoint 0
    Point p1;	//!< segment endpoint 1
 		
    LineSegment() { m = 0; b = 0; }
    
	/*!
		The segment class cannot be virtual (it would affect read/write operations)
		However, the Set() function needs to be virtual. The solution is to defer
		it's "virtuality" to the PiecewiseApprox class that deals with it.
	*/
    LineSegment& Set(const LineSegment& s)
    {
    	m = s.m;
    	b = s.b;
    	p0 = s.p0;
    	p1 = s.p1;
	
		return *this;
    }

	/*!
		Set the segment by providing the slope-intercept form of the line 
		and the first and last point of the segment
	*/
	void Set(const double& slope, const double y_intercept, 
		const Point& firstPt, const Point& lastPt)
	{
		m = slope;
    	b = y_intercept;
    	p0 = firstPt;
    	p1 = lastPt;
	}
    
    LineSegment& operator=(const LineSegment& rhs) { return Set(rhs); }
    
    double GetLen() const { return p0.L2(p1); }

    double Y(const double& x) const { return m * x + b; }

    double Y0() const { return Y(p0.x); }
    double Y1() const { return Y(p1.x); }

	const double& X0() const { return p0.x; }
	const double& X1() const { return p1.x; }
	
	Point GetP0() const { return Point(p0.x, Y(p0.x)); }
	Point GetP1() const { return Point(p1.x, Y(p1.x)); }

	//! Returns a vector tangent to the segment
	Point GetTangent() const { return Point(p1.x - p0.x, p1.y - p0.y); }

	void Reverse(const double& maxXCoord)
	{
		// Save Y0() before changing m and b, since Y0() depends on them
		const double y0 = Y0();
		DBG_ONLY(const double y1 = Y1())

		p0.x = maxXCoord - p0.x;
		p1.x = maxXCoord - p1.x;

		m = -m;
		b = -p0.x * m + y0;

		std::swap(p0, p1);

		ASSERT(fabs(Y0() - y1) < 0.001 && fabs(Y1() - y0) < 0.001);
	}

	//! Returns true if x >= X0() and x <= X1()
	bool ContainsX(const double& x) const
	{
		return (x >= X0() && x <= X1());
	}

	/*!
		Find the x coordinate at which the line segments cross. 
		@precondition the lines segments must cross at exactly one point
	*/
	double CrossingPoint(const LineSegment& ls) const
	{
		ASSERT(m != ls.m);

		double cx = (ls.b - b) / (m - ls.m); 
		
		ASSERT(ContainsX(cx) && ls.ContainsX(cx));
		ASSERT(fabs(Y(cx) - ls.Y(cx)) < 0.0001);

		return cx;
	}

    std::istream& Read(std::istream& is)
    {
		is.read((char*)this, sizeof(*this));
		return is;
    }

    std::ostream& Write(std::ostream& os) const
    {
		os.write((char*)this, sizeof(*this));
		return os;
    }
	
	std::ostream& Print(std::ostream& os) const
    {
    	return os << "[" << p0 << "," << p1 << "] "
    		<< m << " x + " << b;
    }

	friend std::ostream& operator<<(std::ostream& os, const LineSegment& s) 	{ return s.Print(os); }
	//friend std::istream& operator>>(std::istream& is, LineSegment& s)			{ return s.Read(is); }
};

//! Array of line segments
typedef SmartArray<LineSegment> LineSegmentArray;

} //namespace vpl

#endif //__LINE_SEGMENT_H__
