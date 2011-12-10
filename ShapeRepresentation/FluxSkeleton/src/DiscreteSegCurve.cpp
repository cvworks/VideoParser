/************************************************************************

File:		DiscreteSegCurve.cpp

Author(s):		Pavel Dimitrov

Created:		12 Jun 2002

Last Revision:	$Date: 2002/07/25 20:50:49 $

Description:	

$Revision: 1.3 $

$Log: DiscreteSegCurve.cpp,v $
Revision 1.3  2002/07/25 20:50:49  pdimit
Making release 0.1

Revision 1.2  2002/06/26 04:30:44  pdimit
After the failed attempt at getting the spline smoothing to work

Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
Initial import



Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

***********************************************************************/
#define DISCRETESEGCURVE_CPP

#include "DiscreteSegCurve.h"
#include <stdio.h>

#include "LineSeg.h"
#include "DiscreteSeg.h"
#include "CircSeg.h"

#include "Tools/BasicUtils.h"

using namespace sg;

//! @see ReadUserArguments() for id order
int DiscreteSegCurve::s_curveSegmentClassId = 0;

//! @see ReadUserArguments() for id order
/*static*/ 
CurveSeg* DiscreteSegCurve::CreateCurveSegment(const Point& p0, const Point& p1)
{
	// ID order: DiscreteSeg,LineSeg,CircSeg
	switch (s_curveSegmentClassId)
	{
		case 0: return new DiscreteSeg(p0, p1);
		case 1: return new LineSeg(p0, p1);
		case 2: return new CircSeg(/*p0, p1*/);
	}

	return NULL;
}

void DiscreteSegCurve::computeDistance(const Point& p, Distance& dst)
{
	std::vector<CurveSeg*>::iterator I;
	double dist = -1;
	CurveSeg* cs;
	double d2p;

	ASSERT(!m_segments.empty());

	for(I = m_segments.begin(); I != m_segments.end(); ++I)
	{
		d2p = (*I)->distToPt(p);

		if (dist < 0 || dist > d2p)
		{
			cs = *I;
			dist = d2p;
		}
	}

	// Computing the signed distance now. see ContourCurve.h for a description.
	dst = cs->computeDistance(p);
	double t = dst.t;
	Vector TQ = cs->tangent(t);
	Point Q = cs->atT(t);
	Vector v(p.x-Q.x, p.y-Q.y);

	if( TQ.x*v.y - TQ.y*v.x > 0)
		dst.dist = dist;
	else
		dst.dist = -dist;

	dst.t = dst.t + cs->startLength;
}


/*!
	Finds the curve segment that is closest to the given point
*/
double DiscreteSegCurve::distToPt(const Point &p)
{
	std::vector<CurveSeg*>::iterator I;
	CurveSeg* cs;
	Distance dst, bestDst;

	bestDst.dist = -1;

	ASSERT(!m_segments.empty());

	// Test all curve m_segments
	for(I = m_segments.begin(); I != m_segments.end(); ++I)
	{
		(*I)->computeDistance(p, dst);

		if(bestDst.dist < 0 || bestDst.dist > dst.dist)
		{
			cs = *I;
			bestDst = dst;
		}
	}

	// Computing the signed distance now. see ContourCurve.h for a description.
	Vector TQ = cs->tangent(bestDst.t);
	const Point& Q = bestDst.p;   //Diego, old code: Point Q = cs->atT(bestDst.t);
	Vector v(p.x - Q.x, p.y - Q.y);

	if (TQ.x * v.y - TQ.y * v.x <= 0)
		bestDst.dist = -bestDst.dist;

	return bestDst.dist;
}

// returns the t value for which curve(t) is closest to p
// NOTE: ineficient !!!
double DiscreteSegCurve::closestToPt(const Point &p)
{
	std::vector<CurveSeg*>::iterator I;
	double dist = -1;
	double t = 0;
	double d, st;

	for(I=m_segments.begin(); I!=m_segments.end(); ++I)
	{
		d = (*I)->distToPt(p);

		if(dist < 0 || dist > d)
		{
			dist = d;
			st = (*I)->closestToPt(p); 
			t = st + (*I)->startLength;
		}
	}

	return t;
}

Point DiscreteSegCurve::atT(const double& t)
{
	if(t > m_length) 
		return Point(-1,-1); // this should really be an exception or smthg...

	std::vector<CurveSeg*>::iterator I;
	CurveSeg* cs;

	for(I=m_segments.begin(); I!=m_segments.end(); ++I)
	{
		cs = *I;

		if (cs->startLength <= t && cs->getLength() + cs->startLength >= t)
		{
			return cs->atT(t - cs->startLength);
		}
	}

	return Point(-1,-1);
}

Vector DiscreteSegCurve::tangent(const double& t)
{
	ASSERT(t >= 0 && t <= m_length);

	std::vector<CurveSeg*>::iterator I;
	CurveSeg* cs;

	for(I = m_segments.begin(); I != m_segments.end(); ++I)
	{
		cs = *I;

		if (cs->startLength <= t &&	cs->getLength() + cs->startLength >= t)
			return cs->tangent(t - cs->startLength);
	}

	return Vector(-1,-1);
}
