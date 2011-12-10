/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __CUBIC_BEZIER_H__
#define __CUBIC_BEZIER_H__

#include "../MathUtils.h"
#include "../SmartArray.h"
#include "../BasicTypes.h"

namespace vpl {

/*!
	@brief Cubic Bezier class
*/
struct CubicBezier
{
	Point p0;
    Point p1;
	Point p2;
	Point p3;

	CubicBezier& Set(const CubicBezier& cbs)
	{
		p0 = cbs.p0;
		p1 = cbs.p1;
		p2 = cbs.p2;
		p3 = cbs.p3;
		
		return *this;
	}

	double Y0() const { return p0.y; }
    double Y1() const { return p1.y; }

	Point GetP0() const { return p0; }
	Point GetP1() const { return p1; }
	Point GetP2() const { return p2; }
	Point GetP3() const { return p3; }
    
	CubicBezier& operator=(const CubicBezier& rhs) { return Set(rhs); }

	bool operator==(const CubicBezier& rhs) const
	{
		return (p0 == rhs.p0 && p1 == rhs.p1 && p2 == rhs.p2 && p3 == rhs.p3);
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
};

/*!
	Parameters derived from the four points that define a cubic bezier curve.

	The precomputation of these parameters imprueves the performance when
	computing the cubic bezier function.
*/
class CubicBezierParams
{
	double a0, b0, a1, b1, a2, b2, a3, b3;

	friend struct CubicBezier;
	friend class PolyBezierApprox;

protected:
	void Rasterize(double u, const Point& endPt, double stepsize, POINTS& pts,
		SmartArray<double>* pParamMap);

public:
	CubicBezierParams(const CubicBezier& cb);

	void GetCurvePoint(const double& u, double* pX, double* pY) const;
	Point GetTangent(const double& u) const;
	Point SecondDerivative(const double& u) const;

	double MaximumCurvature(const double& resolution, 
		double* pMaxPtParam = NULL) const;

	double Curvature(const double& u) const
	{
		Point first = GetTangent(u);
		Point second = SecondDerivative(u);

		double d = first.x * first.x + first.y * first.y;
		double D = sqrt(d * d * d);

		return (D == 0) ? 1 : fabs(first.x * second.y - first.y * second.x) / D;
	}

	POINTS Rasterize(int nApproxNumPts, SmartArray<double>* pParamMap = NULL,
		double u0 = -1, double u1 = 1);
};

//! Array of cubic bezier segments
typedef SmartArray<CubicBezier> BezierSegmentArray;

} //namespace vpl

#endif //__CUBIC_BEZIER_H__
