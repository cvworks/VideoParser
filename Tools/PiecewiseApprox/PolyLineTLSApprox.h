/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __POLY_LINE_TLS_APPROX_H__
#define __POLY_LINE_TLS_APPROX_H__

#include "PiecewiseApprox.h"
#include "EuclideanLineSegment.h"

namespace vpl {

/*!
	The approx poly class that computes the least squares fit as a
	total least squares problem (i.e., perpendicular distances from
	the points to the fitting line).
*/
class PolyLineTLSApprox : public PiecewiseApprox<EuclideanLineSegment>
{	
public:
	PolyLineTLSApprox(double dMinError, int nMaxSegments)
		: PiecewiseApprox<EuclideanLineSegment>(dMinError, nMaxSegments) 
	{ 
		/* nothing else to do*/ 
	}
		
	virtual double LeastSquares(const Point* vertices, int n, EuclideanLineSegment& s);
	virtual int GetSegmentDirection(const EuclideanLineSegment& s) const { return 0; }
	virtual void PlotKnots(int seg_num) const { /*Must write code here*/ };
};

} //namespace vpl

#endif //__POLY_LINE_TLS_APPROX_H__
