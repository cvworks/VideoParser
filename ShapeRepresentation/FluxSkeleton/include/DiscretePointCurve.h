#ifndef _DISCRETE_POINT_CURVE_H_
#define _DISCRETE_POINT_CURVE_H_

#include "ContourCurve.h"
#include "Vector.h"

namespace sg {

class DiscretePointCurve : public ContourCurve
{
	std::vector<Vector> m_tangents;

public:
	DiscretePointCurve() 
	{ 

	}

    DiscretePointCurve(const std::vector<Point>& pts)
		: ContourCurve(pts), m_tangents(pts.size())
	{
		std::vector<Point>::const_iterator ptIt0, ptIt1;
		std::vector<Vector>::iterator tanIt;

		ptIt1 = pts.begin();
		ptIt0 = ptIt1++;
		tanIt = m_tangents.begin();
		double vecNorm;

		for (; ptIt1 != pts.end(); ++ptIt0, ++ptIt1, ++tanIt)
		{
			tanIt->set(ptIt1->x - ptIt0->x, ptIt1->y - ptIt0->y);
			vecNorm = tanIt->norm();
			(*tanIt) /= vecNorm;
			m_length += vecNorm;
		}

		// Assume that the curve is closed and copy first tangent
		ASSERT(pts.front() == pts.back());
		m_tangents.back() = m_tangents.front();
	}

	virtual void Serialize(OutputStream& os) const
	{
		ContourCurve::Serialize(os);

		::Serialize(os, m_tangents);
	}

	virtual void Deserialize(InputStream& is)
	{
		ContourCurve::Deserialize(is);

		::Deserialize(is, m_tangents);
	}

	/////////////////////////////////////////////////////////////////////
    // Required methods
    
	virtual ContourCurve* clone() const
	{
		DiscretePointCurve* dsc = new DiscretePointCurve(*this);

		dsc->m_length = m_length;
		
		//dsc->m_tangents = m_tangents;

		return dsc;
	}

	/*!
		Finds the curve segment that is closest to the given point
	*/
	virtual double distToPt(const Point& p)
	{
		Distance dst;

		computeDistance(p, dst);

		return dst.dist;
	}

	virtual Point atT(const double& t)
	{
		ASSERT(false);
		return Point();
	}

	virtual Vector tangent(unsigned int idx)
	{
		ASSERT(idx < m_tangents.size());
		return m_tangents[idx];
	}

	virtual Vector tangent(const double& t)
	{
		ASSERT(false);
		return Vector();
	}

	virtual double closestToPt(const Point &p)
	{
		ASSERT(false);
		return 0;
	}

	virtual void computeDistance(const Point& p, Distance& dst)
	{
		getKDTree()->KNNSearch(p.x, p.y, 1);

		int idx;

		getKDTree()->GetNNInfo(0, &idx, &dst.p, &dst.dist);

		dst.t = idx;

		// Find out the sign of the distance
		const Vector& TQ = m_tangents[idx];

		Vector v(p.x - dst.p.x, p.y - dst.p.y);

		if (TQ.x * v.y - TQ.y * v.x <= 0) 
			dst.dist = -dst.dist;
	}
};

} // namespace sg

#endif // _DISCRETE_POINT_CURVE_H_
