#ifndef _APPROX_DISCRETE_SEG_CURVE_H_
#define _APPROX_DISCRETE_SEG_CURVE_H_

#include "DiscreteSegCurve.h"

namespace sg {

class ApproxDiscreteSegCurve : public DiscreteSegCurve
{
public:
	ApproxDiscreteSegCurve() { }

    ApproxDiscreteSegCurve(std::vector<CurveSeg*>& segs, bool closed)
		: DiscreteSegCurve(segs, closed)
	{
	}

	/////////////////////////////////////////////////////////////////////
    // Required methods
    
	/*!
		Finds the curve segment that is closest to the given point
	*/
	virtual double distToPt(const Point& p)
	{
		std::vector<CurveSeg*>::iterator I = segments.begin();
		
		double minDist = (*I)->squaredDistToFirstPt(p);
		CurveSeg* cs = *I;

		double d;

		for(++I; I != segments.end(); ++I)
		{
			d = (*I)->squaredDistToFirstPt(p);

			if (d < minDist)
			{
				minDist = d;
				cs = *I;
			}
		}

		// Note that min distance is squared. Get actual distance.
		minDist = sqrt(minDist);

		// Computing the signed distance now. see Curve.h for a description.
		Vector TQ = cs->tangent(0);
		Point Q = cs->atT(0);

		Vector v(p.x - Q.x, p.y - Q.y);

		return (TQ.x * v.y - TQ.y * v.x > 0) ? minDist : -minDist;
	}

	virtual Point atT(double t)
	{
		ASSERT(false);
		return Point();
	}

	virtual Vector tangent(double t)
	{
		ASSERT(false);
		return Vector();
	}

	virtual Curve* subCurve(double t0, double t1) 
	{
		return this;
	}

	virtual double closestToPt(const Point &p)
	{
		ASSERT(false);
		return 0;
	}

	virtual void computeDistance(const Point& p, Distance& dst)
	{
		std::vector<CurveSeg*>::iterator I = segments.begin();
		
		double minDist = (*I)->squaredDistToFirstPt(p);
		CurveSeg* cs = *I;

		double d;

		for(++I; I != segments.end(); ++I)
		{
			d = (*I)->squaredDistToFirstPt(p);

			if (d < minDist)
			{
				minDist = d;
				cs = *I;
			}
		}

		// Note that min distance is squared. Get actual distance.
		minDist = sqrt(minDist);

		// Fill in dst values

		dst.t = 0; // + cs->startLength;
		dst.p = cs->atT(0);

		// Computing the signed distance now. see Curve.h for a description.
		Vector TQ = cs->tangent(0);

		Vector v(p.x - dst.p.x, p.y - dst.p.y);

		dst.dist = (TQ.x * v.y - TQ.y * v.x > 0) ? minDist : -minDist;
	}

    virtual Distance computeDistance(const Point &p, double t1, double t2)
	{
		ASSERT(false);
		return Distance();
	}

	virtual Curve* clone() const
	{
		ApproxDiscreteSegCurve *dsc = new ApproxDiscreteSegCurve();

		dsc->length = length;
		dsc->closed = closed;  
		dsc->shortest_seg = shortest_seg;

		std::vector<CurveSeg*>::const_iterator I;

		for(I = segments.begin(); I != segments.end(); ++I)
		{
			dsc->segments.push_back((*I)->clone());
		}  

		return dsc;
	}
};

} // namespace sg

#endif // _APPROX_DISCRETE_SEG_CURVE_H_
