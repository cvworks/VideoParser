/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <stdlib.h>
#include <iostream>
#include "PolyBezierApprox.h"
#include "../HelperFunctions.h"
#include "../BasicUtils.h"

using namespace vpl;

//////////////////////////////////////////////////////////////////////
// CubicBezierParams class

/*!
	@brief Computes a number of parameters that are needed for evaluating
	a cubic Bezier curve. In many situations, these parameters do not change
	across multiple evaluations, and so it helps to have them pre-computed.
*/
CubicBezierParams::CubicBezierParams(const CubicBezier& c)
{
	a3 = (c.p3.x - c.p0.x + 3 * (c.p1.x - c.p2.x)) / 8;
	b3 = (c.p3.y - c.p0.y + 3 * (c.p1.y - c.p2.y)) / 8;
	a2 = (c.p3.x + c.p0.x - c.p1.x - c.p2.x) * 3 / 8;
	b2 = (c.p3.y + c.p0.y - c.p1.y - c.p2.y) * 3 / 8;
	a1 = (c.p3.x - c.p0.x) / 2 - a3;
	b1 = (c.p3.y - c.p0.y) / 2 - b3;
	a0 = (c.p3.x + c.p0.x) / 2 - a2;
	b0 = (c.p3.y + c.p0.y) / 2 - b2;
}

/*!
	@brief Evaluates the cubic Bezier function at 'u'
*/
void CubicBezierParams::GetCurvePoint(const double& u, double* pX, double* pY) const
{
	*pX = a0 + u * (a1 + u *(a2 + u * a3));
	*pY = b0 + u * (b1 + u *(b2 + u * b3));
}

Point CubicBezierParams::GetTangent(const double& u) const
{
	double dx = a1 + u * (2 * a2 + u * 3 * a3);
	double dy = b1 + u * (2 * b2 + u * 3 * b3);

	return Point(dx, dy);
}

Point CubicBezierParams::SecondDerivative(const double& u) const
{
	double dx = 2 * a2 + u * 6 * a3;
	double dy = 2 * b2 + u * 6 * b3;

	return Point(dx, dy);
}

/*!
	@brief Computes the maximum curvature at 1/resolution steps.

	The parameter of the point with the greatest curvature is returned
	in *pMaxPtParam if pMaxPtParam is not null.
*/
double CubicBezierParams::MaximumCurvature(const double& resolution,
										   double* pMaxPtParam /*=NULL*/) const
{
	Point p0, p1;

	GetCurvePoint(-1, &p0.x, &p0.y);
	GetCurvePoint( 1, &p1.x, &p1.y);

	double stepsize = 1.0 / resolution;
	double maxCurv = 0;
	double maxPtParam = -1, k, u = -1;

	while (true)
	{
		k = Curvature(u);

		if (k > maxCurv)
		{
			maxCurv = k;
			maxPtParam = u;
		}

		if (u < 1)
		{
			u += stepsize;
			if (u > 1) u = 1;
		}
		else
		{
			break;
		}
	}

	if (pMaxPtParam)
		*pMaxPtParam = maxPtParam;

	return maxCurv;
}

//////////////////////////////////////////////////////////////////////
// CubicBezier class

//////////////////////////////////////////////////////////////////////
// PolyBezierApprox class

PolyBezierApprox::PolyBezierApprox(double dMinError, int nMaxSegments, int nMaxRetry /*=1*/) 
: PiecewiseApprox<CubicBezier>(dMinError, nMaxSegments)
{
	m_nMaxRetry = nMaxRetry;
	m_curvePoints = NULL;
}

/*!
	@brief Fits a piecewise cubic Bezier curve to the datapoints.
*/
void PolyBezierApprox::Fit(const POINTS vertices)
{
	PiecewiseApprox<CubicBezier>::Fit(vertices);

	// Finally, we must update the m_curvePoints array
	const Point* pts = vertices;

	delete[] m_curvePoints;
	m_curvePoints = new double[vertices.GetSize()];

	for (int i = 0, d = 0; i < m_knots.GetSize(); i++)
	{
		GetTotalDistance(pts + d, m_knots[i].nIndex - d + 1, m_knots[i].seg, m_curvePoints + d);
		d = m_knots[i].nIndex + 1;
	}
}

/*!
	@brief Computes the distance from point 'pt' to the point 'u' in the
	cubic bezier segment with parameters 'cbp'.
*/
void PolyBezierApprox::GetDistanceFromPointToCurvePoint(const Point& pt, const double& u, 
														const CubicBezierParams& cbp,
														double* s, double* z)
{
	double x, y, dx4, dy4, dx, dy;

	x = cbp.a0 + u * (cbp.a1 + u *(cbp.a2 + u * cbp.a3));
	y = cbp.b0 + u * (cbp.b1 + u *(cbp.b2 + u * cbp.b3));
	
	dx4 = x - pt.x;
	dy4 = y - pt.y;

	dx = cbp.a1 + u * (2 * cbp.a2 + u * 3 * cbp.a3);
	dy = cbp.b1 + u * (2 * cbp.b2 + u * 3 * cbp.b3);

	*z = dx * dx4 + dy * dy4;
	*s = dx4 * dx4 + dy4 * dy4;
}

/*!
	@brief Computes the total distance from the 'n' points in the 'vertices'
	arrato to the cubic bezier segment 'cbs'.

	This function also updates the m_curvePoints array with the corresponding
	curve parameter 'u' for each datapoint in 'vertices'.
*/
double PolyBezierApprox::GetTotalDistance(const Point* vertices, int n, const CubicBezier& cbs, 
										  double* curvePoints /*=NULL*/)
{
	CubicBezierParams cbp(cbs);
	double totalerror = 0, stepsize, s, z, u, u0, u1 = 0, s1 = 0, z1 = 0, u2, z2, temp;

	if (curvePoints)
	{
		curvePoints[0] = -1;
		curvePoints[n - 1] = 1;
	}

	// First and last point are assumed to have zero distance
	for (int i = 1; i < n - 1; i++)
	{
		const Point& pt = vertices[i];

		stepsize = 2.0 / (n/* + 1*/); // remember, we go from -1 to 1 => length 2

		for (u = -1; u <= 1.01; u += stepsize)
		{
			GetDistanceFromPointToCurvePoint(pt, u, cbp, &s, &z);

			// Find minimum
			if (u == -1 || s < s1)
			{
				u1 = u;
				z1 = z;
				s1 = s;
				if (s == 0) break; // we found it already
			}
		}

		if (u1 > 1) u1 = 1; // make sure tha we have a valid parameter

		u0 = u1; // save u1 to store it in curvePoints

		// We found the closest point on the curve to datapoint i
		// the point is 'u1' and has 's1' error
		if (s1 != 0)
		{
			u = u1 + stepsize;

			if (u > 1)
				u = 1 - stepsize;

			while (true)
			{
				GetDistanceFromPointToCurvePoint(pt, u, cbp, &s, &z);

				if (s < s1) // improved error. save new params
				{
					s1 = s;
					u0 = u;
				}

				if (s == 0 || z == 0) break;
				
				u2 = u;
				z2 = z;
				temp = z2 - z1;

				if (temp != 0.0)
					u = (z2 * u1 - z1 * u2) / temp;
				else
					u = (u1 + u2) / 2.0;

				//Diego fix: 24/Oct/2006
				/*if (u > 1)
					u = 1;
				else if (u < -1)
					u = -1;*/
				if (u >= 1 || u <= -1) break;

				if (fabs(u - u2) < .0001) break;

				u1 = u2;
				z1 = z2;
			}
		}

		totalerror = totalerror + s1;

		if (curvePoints)
			curvePoints[i] = u1;
	}

	return totalerror;
}

/*!
	@brief Computes least squares fit to the given datapoints. The best fit cubic Bezier
	curve is returnd in 'cb0'.
*/
double PolyBezierApprox::LeastSquares(const Point* vertices, int n, CubicBezier& cb0)
{
	if (n < 2)
		return -1; // real fit error will never be negative.

	double e1, e2, e3;
	double x1a, y1a, x2a, y2a;
	CubicBezier cb1;

	cb1.p0.Set(vertices[0].x, vertices[0].y);
	cb1.p3.Set(vertices[n - 1].x, vertices[n - 1].y);

	// Give some reasonable initial values to the two handle points
	const int offset = (n < 4) ? 1:2;
	
	cb1.p1.Set(vertices[offset].x, vertices[offset].y);
	cb1.p2.Set(vertices[n - offset - 1].x, vertices[n - offset - 1].y);

	// Find distance to the curve we just guessed
	cb0 = cb1;
	e1 = GetTotalDistance(vertices, n, cb1);
	
	for (int Retry = 0; Retry <= m_nMaxRetry; Retry++)
	{
		e3 = .5;
		x1a = cb1.p1.x;
		
		// Minimize first handle on X. Do while we cannot reduce error anymore
		while (true)
		{
			cb1.p1.x = cb1.p1.x + (cb1.p1.x - cb1.p0.x) * e3;
			e2 = GetTotalDistance(vertices, n, cb1);

			if (e2 == e1) 
				break;
			else if (e2 > e1)   // things got worse
			{
				cb1.p1.x = x1a; // restore last good value
				e3 = -e3 / 3;

				if (fabs(e3) < .01)
					break;
			}
			else
			{
				e1 = e2;
				x1a = cb1.p1.x;
				cb0 = cb1;
			}
		}

		e3 = .5;
		y1a = cb1.p1.y;

		// Minimize first handle on Y. Do while we cannot reduce error anymore
		while (true)
		{
			cb1.p1.y = cb1.p1.y + (cb1.p1.y - cb1.p0.y) * e3;
			e2 = GetTotalDistance(vertices, n, cb1);

			if (e2 == e1)
				break;
			else if (e2 > e1)
			{
				cb1.p1.y = y1a;
				e3 = -e3 /3;

				if (fabs(e3) < .01) 
					break;
			}
			else
			{
				e1 = e2;
				y1a = cb1.p1.y;
				cb0 = cb1;
			}
		}

		e3 = .5;
		x2a = cb1.p2.x;

		// Minimize second handle on X. Do while we cannot reduce error anymore
		while (true)
		{
			cb1.p2.x = cb1.p2.x + (cb1.p2.x - cb1.p3.x) * e3;
			e2 = GetTotalDistance(vertices, n, cb1);

			if (e2 == e1)
				break;
			else if (e2 > e1)
			{
				cb1.p2.x = x2a;
				e3 = -e3 / 3;

				if (fabs(e3) < .01)
					break;
			}
			else
			{
				e1 = e2;
				x2a = cb1.p2.x;
				cb0 = cb1;
			}
		}

		e3 =.5;
		y2a = cb1.p2.y;
		
		// Minimize second handle on Y. Do while we cannot reduce error anymore
		while(true)
		{
			cb1.p2.y = cb1.p2.y + (cb1.p2.y - cb1.p3.y) * e3;
			e2 = GetTotalDistance(vertices, n, cb1);

			if (e2 == e1)
				break;
			else if (e2 > e1)
			{
				cb1.p2.y = y2a;
				e3 = -e3 /3;

				if (fabs(e3) < .01)
					break;
			}
			else
			{
				e1 = e2;
				y2a = cb1.p2.y;
				cb0 = cb1;
			}
		}
	}
		
	return e1;
}

/*!
	@brief Returns an array with all the points on the curve between 
	curve points u0 and u1. The points in the array are one-pixel
	thin and connected.

	Optional: if the "parameter map", pParamMap, is given (ie, it's a valid pointer), 
	then the curve parameter 'u' correspoinding to each rasterized point 'i'
	is assigned to position 'i' in the map.
*/
POINTS CubicBezierParams::Rasterize(int nApproxNumPts, SmartArray<double>* pParamMap /*=NULL*/,
									double u0 /*=-1*/, double u1 /*=1*/)
{
	POINTS pts(0, nApproxNumPts);

	if (pParamMap)
	{
		pParamMap->Clear();
		pParamMap->SetGrowFactor(nApproxNumPts);
	}

	ASSERT(u0 >= -1 && u0 < 1 && u1 > -1 && u1 <= 1 && u0 < u1);

	double stepsize = (u1 - u0) / (nApproxNumPts - 1);

	ASSERT(stepsize > 0 && stepsize <= 2);

	Point endPt;

	GetCurvePoint(1, &endPt.x, &endPt.y);
	//endPt.Round(); // point must be rounded (uncomment to get integer coordinates)

	Rasterize(u0, endPt, stepsize, pts, pParamMap);

	WARNING(pts.Size() > nApproxNumPts, "Increase init num pts to speed up exec");
	ASSERT(!pParamMap || pParamMap->Size() == pts.Size());

/*#ifdef _DEBUG
	for (int i = 1; i < pts.Size(); i++)
	{
		ASSERT(pts[i - 1].L2(pts[i]) <= 1.5);
	}
#endif*/

	return pts;
}

/*!
	@brief Fills the point array with all the points on the curve, starting at
	parameter u' >= u and ending at point lastPt (which is also added). If
	pts is empty, the first point added is at u' = u. Otherwise, the first 
	point added is at u' > u and meets the following condition:
	     u' = argmin_u'(u' - u) AND (cb(u').x != x0 || cb(u').y != y0).
	
	The points in the array are one-pixel thin and connected.
*/
void CubicBezierParams::Rasterize(double u, const Point& endPt, double stepsize, 
							      POINTS& pts, SmartArray<double>* pParamMap)
{
	Point pt;

	int sz = pts.Size();

	// If not the first point, then cb(u) is already in the array
	if (sz > 0) 
		u += stepsize;

	do
	{
		GetCurvePoint(u, &pt.x, &pt.y);

		//pt.Round(); //point must be rounded (uncomment to get integer coordinates)

		// check that current point is not equal to last point
		if (sz == 0 || !pt.RoundedIsEqual(pts[sz - 1])/*pt != pts[sz - 1]*/)  /*uncomment for integer coords*/
		{
			// check if we have two disconnected points. max dist is sqrt(2)
			if (sz > 1 && pt.SqDist(pts[sz - 1]) > 2)
			{
				Rasterize(u - stepsize, pt, stepsize / 2, pts, pParamMap);
				sz = pts.Size(); // update current size
			}
			// avoid having a "thick" set of 3 pts
			else if (sz > 2 && pt.SqDist(pts[sz - 2]) <= 2)
			{
				// Replace the last point with the current point
				pts[sz - 1] = pt;

				if (pParamMap) 
					(*pParamMap)[sz - 1] = u;
			}
			else // first iteration enters here ALWAYS
			{
				// Add a new point to the array
				pts.AddTail(pt);

				if (pParamMap) 
					pParamMap->AddTail(u);

				sz = pts.Size(); // update current size
			}
		}

		u += stepsize;   // step to the next point

		// The curve param u must be in [-1, 1]. Adding the step size should
		// keep it smaller than 1 + max_stepsize, for max_stepsize == 2
		ASSERT(u <= 3);

	} while (!pt.RoundedIsEqual(endPt)/*pt != endPt*/); /*uncomment for integer coords*/
}

