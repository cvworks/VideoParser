/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __POLY_BEZIER_APROX_H__
#define __POLY_BEZIER_APROX_H__

#include "PolyLineApprox.h"
#include "CubicBezier.h"

namespace vpl {

class PolyBezierApprox : public PiecewiseApprox<CubicBezier>
{
	int m_nMaxRetry;
	double* m_curvePoints;
	
public:
	PolyBezierApprox(double dMinError, int nMaxSegments, int nMaxRetry = 1);
	~PolyBezierApprox() { delete[] m_curvePoints; }
	
	virtual double LeastSquares(const Point* vertices, int n, CubicBezier& s);
	virtual int GetSegmentDirection(const CubicBezier& s) const { return 0; }
	virtual void PlotKnots(int seg_num) const { /*Must write code here*/ };

	void Fit(const POINTS vertices);

	//! Returns the tanget of the point on the curve closest to input point i in constant time
	Point PolyBezierApprox::GetTangent(int i) const
	{
		ASSERT(m_curvePoints);

		CubicBezierParams cbp(GetPointSegment(i));

		return cbp.GetTangent(m_curvePoints[i]);
	}

	/*! 
		@brief Returns the curve "segment" parameter 'u' corresponding to the point, 
		on "some" curve segment, that is the closest, between ALL curve points, to 
		the input point i. It takes constant time. To access the actual point coordinates, 
		the curve segment associated with paramater 'u' must be known. 
		
		@See GetPointSegment().
	*/
	double GetCurveParam(int i) const
	{
		ASSERT(m_curvePoints);
		return m_curvePoints[i];
	}
	
	 // The following functions depend on the current values of shared variables
protected:
	void GetDistanceFromPointToCurvePoint(const Point& pt, const double& u, 
		const CubicBezierParams& cbp, double* s, double* z);

	double GetTotalDistance(const Point* rawdata, int n, const CubicBezier& cbs, 
		double* curvePoints = NULL);
};

} //namespace vpl

#endif //__POLY_BEZIER_APROX_H__
