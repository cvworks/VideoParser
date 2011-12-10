/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __POLY_LINE_APROX_H__
#define __POLY_LINE_APROX_H__

#include "PiecewiseApprox.h"
#include "LineSegment.h"

namespace vpl {

class PolyLineApprox : public PiecewiseApprox<LineSegment>
{
protected:
	double m_dMinSlope;

public:
	PolyLineApprox(double dMinError, double dMinSlope, int nMaxSegments, double dMaxYDiff)
		: PiecewiseApprox<LineSegment>(dMinError, nMaxSegments, dMaxYDiff) 
	{ 
		m_dMinSlope =  dMinSlope;
	}

	virtual double LeastSquares(const Point* vertices, int n, LineSegment& s);
	virtual int GetSegmentDirection(const LineSegment& s) const;
	virtual void PlotKnots(int seg_num) const;

	const double& GetSlope(int i) {	return GetPointSegment(i).m; }
};

} //namespace vpl

#endif //__POLY_LINE_APROX_H__
