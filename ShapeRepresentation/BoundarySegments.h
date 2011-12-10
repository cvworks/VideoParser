/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "FluxSkeleton/include/BoundaryInterval.h"
#include <Tools/STLUtils.h>
#include <Tools/HelperFunctions.h>

namespace vpl {

/*!
	Abstract class representing a boundary segment.
*/
class BoundarySegment
{
public:
	BoundarySegment() { }

	virtual double Length() const = 0;

	//! Necessary virtual destructor
	virtual ~BoundarySegment() { }

	//! Moves the internal pointer to boundary points. Call GetPoint() after.
	virtual void Seek(const double& offset, double* pRemainder) = 0;

	//! Returns the boundary point referred by the internal pointer.
	virtual Point GetPoint() const = 0;

	virtual const Point& FirstPoint() const = 0;

	virtual const Point& LastPoint() const = 0;

	virtual void AppendTo(PointArray* pPts) const = 0;
};

/*!
	Boundary segments representes as a discrete array of points.

	Note: we don't need to define operator=() because, finally, the
	default behavior is actually what we want.
*/
class DiscreteBoundarySegment : public BoundarySegment
{
protected:
	const PointArray* m_pPts;
	sg::BoundaryInterval m_bndryInt;

	bool m_ownsPts;
	unsigned m_currPtIdx;

public:
	/*!
		m_numPts = bi.Size();
	*/
	DiscreteBoundarySegment(const PointArray* pPts, const sg::BoundaryInterval& bi)
	{
		ASSERT(pPts->size() == bi.BoundarySize());

		m_pPts = pPts;
		m_bndryInt = bi;

		m_currPtIdx = bi.First();
		m_ownsPts = false;
	}

	/*!
		m_first = 0;
		m_last = m_pPts->size() - 1;
		m_numPts = m_pPts->size();

		m_length = p0.dist(p1); // DIEGO: + 1
	*/
	DiscreteBoundarySegment(const Point& p0, const Point& p1)
	{
		PointArray* pPts = new PointArray;
		
		RasterizeLine(p0, p1, pPts);

		m_pPts = pPts;

		m_bndryInt.Set(m_pPts->size(), 0, m_pPts->size() - 1);

		m_currPtIdx = m_bndryInt.First();
		m_ownsPts = true;
	}

	~DiscreteBoundarySegment()
	{
		if (m_ownsPts)
			delete m_pPts;
	}

	//! Computes the length of segment
	double Length() const
	{
		double length = 0;
		int j;

		if (m_bndryInt.First() != m_bndryInt.Last())
		{
			for (int i = m_bndryInt.First(); ; i = j)
			{
				j = m_bndryInt.Succ(i);

				length += (*m_pPts)[i].dist((*m_pPts)[j]);

				if (j == m_bndryInt.Last())
					break;
			}
		}

		return length;
	}

	/*!
		Moves to the point "current + offset". If there is no such
		point, the remainder distance between the last point of the
		segment and the requested offset is set.
	*/
	void Seek(const double& offset, double* pRemainder)
	{
		//ASSERT(m_currPtIdx >= m_first);
		//ASSERT(m_currPtIdx <= m_last);
		ASSERT(offset > 0);

		*pRemainder = offset;

		// If we are at the last point already, there is no place to go
		if (m_currPtIdx == m_bndryInt.Last())
			return;

		// Accumulate distances and subtract them from the requested offset
		for (++m_currPtIdx; ; ++m_currPtIdx)
		{
			// See if we need to wrap around the endpoints of the "circular" 
			// array of boundary points
			if (m_currPtIdx == m_pPts->size()) 
			{
				m_currPtIdx = 0;
				*pRemainder -= m_pPts->front().dist(m_pPts->back());
			}
			else
			{
				*pRemainder -= (*m_pPts)[m_currPtIdx].dist((*m_pPts)[m_currPtIdx - 1]);
			}

			// If the remainder is zero, we are done, but...
			if (*pRemainder <= 0)
			{
				// ...make sure that the remainder returned is zero
				*pRemainder = 0; 
				break;
			}

			// If we reached the last point, we are done too
			if (m_currPtIdx == m_bndryInt.Last())
				break;
		}
	}

	Point GetPoint() const
	{
		return (*m_pPts)[m_currPtIdx];
	}

	const Point& FirstPoint() const
	{
		return (*m_pPts)[m_bndryInt.First()];
	}

	const Point& LastPoint() const
	{
		return (*m_pPts)[m_bndryInt.Last()];
	}

	virtual void AppendTo(PointArray* pPts) const
	{
		ASSERT(m_pPts && !m_pPts->empty());
		
		for (int i = m_bndryInt.First(); ; i = m_bndryInt.Succ(i))
		{
			pPts->push_back((*m_pPts)[i]);

			if (i == m_bndryInt.Last())
				break;
		}
	}
};

/*!
	Boundary segments representes as a discrete array of points.

	Note: we don't need to define operator=() because, finally, the
	default behavior is actually what we want.
*/
/*class EvenlySpacedDiscreteBoundarySegment : public DiscreteBoundarySegment
{
public:
	EvenlySpacedDiscreteBoundarySegment(const PointArray* pPts, 
		const sg::BoundaryInterval& bi) : DiscreteBoundarySegment(pPts, bi)
	{
	}

	EvenlySpacedDiscreteBoundarySegment(const Point& p0, const Point& p1)
		: DiscreteBoundarySegment(p0, p1)
	{
	}

	void Seek(const double& offset, double* pRemainder)
	{
		ASSERT(false);

		ASSERT(offset > 0);

		unsigned newPointIdx = m_currPtIdx + (unsigned) offset;
					
		if (newPointIdx < m_numPts)
		{
			m_currPtIdx = newPointIdx;
			*pRemainder = 0;
		}
		else
		{
			*pRemainder = offset - (m_numPts - m_currPtIdx - 1);
			ASSERT(*pRemainder >= 0);
		}
	}*/

/*!
	Boundary segments representes parameterically as a line.
*/
/*class LinearBoundarySegment : public BoundarySegment
{
	Point m_firstPt;
	Point m_lastPt;
	double m_t;

public:
	LinearBoundarySegment(const Point& p0, const Point& p1)
		: m_firstPt(p0), m_lastPt(p1)
	{
		m_length = p0.dist(p1); // Diego: + 1
		m_t = 0;
	}

	void Seek(const double& offset, double* pRemainder)
	{
		ASSERT(false);
	}
	
	Point GetPoint() const
	{
		ASSERT(m_t >= 0 && m_t <= 1);

		return Point(0, 0);
	}

	const Point& FirstPoint() const
	{
		return m_firstPt;
	}

	const Point& LastPoint() const
	{
		return m_lastPt;
	}
};*/

/*!
	A closed contour represented as a connected chain of
	boundary segments.

	Usually, a CBS is used to represent the countour of 
	a shape part, which is given as a chain of
	segments of different types.
*/
class ConnectedBoundarySegments
{
	typedef std::list<BoundarySegment*> BoundarySegmentList;

	BoundarySegmentList m_segments;
	PointArray m_pts;
	double m_length;

public:
	ConnectedBoundarySegments()
	{
		// Set a negative length to indicate that
		// the curve has note been "closed" yet
		m_length = -1;
	}

	~ConnectedBoundarySegments()
	{
		std_forall(it, m_segments)
			delete *it;
	}

	void AddSegment(BoundarySegment* pSegment)
	{
		m_segments.push_back(pSegment);		

		pSegment->AppendTo(&m_pts);
	}

	/*!
		@brief Computes tangent vactor at the given point

		The tangent is directed from the point ptIndex + 1 to the point 
		ptIdx - 1. If there is only one point or none, 
		the tangent (1,0) is returned.

		The tangent is approximated by the secant line that goes throw
		the two immediate neighbours of the given point, if it's not 
		and endpoint. Otherwise, a totalk least square is computed using
		the points close to the endpoint.
	*/
	Point ComputeTangent(unsigned ptIdx) const
	{
		ASSERT(ptIdx < m_pts.size());
		
		if (m_pts.size() <= 2)
			return Point(1,0);

		unsigned a, b;

		if (ptIdx == 0)
		{
			a = m_pts.size() - 1;
			b = ptIdx + 1;
		}
		else if (ptIdx == m_pts.size() - 1)
		{
			a = ptIdx - 1;
			b = 0;
		}
		else
		{
			a = ptIdx - 1;
			b = ptIdx + 1;
		}

		return m_pts[a] - m_pts[b];

		/*
		// This code is not needed, but useful to consider
		double a, b, c; // coefficient of the line: a * x + b * y + c = 0
		LinearTotalLeastSquares(m_pts, nPts, a, b, c);

		tangentVector = GetClosestPointOnLine(a, b, c, m_pts[0]) -
		GetClosestPointOnLine(a, b, c, m_pts[nPts - 1]);
		*/
	}

	//! Angle in radians (atan2) of the tangent vectors at each point
	double Tangent(unsigned ptIdx) const
	{
		return ComputeTangent(ptIdx).Atan2();
	}

	/*!
		Finalizes the curve by assuming that it has been closed
		and computes its length.
	*/
	void CloseCurve()
	{
		ASSERT(m_length < 0);
		ASSERT(m_pts.front().dist(m_pts.back()) < 2);

		// If the curve is not closed, then close it
		if (m_pts.front() != m_pts.back())
			m_pts.push_back(m_pts.front());

		// sum of the length of all segments added assuming
		// that they are connected by straight line segments
		auto it1 = m_pts.begin();
		auto it0 = it1++;

		m_length = 0;

		for (; it1 != m_pts.end(); ++it0, ++it1)
			m_length += it0->dist(*it1);

		// old code: before curve was not closed
		// consider the length of the segment that closes the curve
		//m_length += m_pts.front().dist(m_pts.back());
	}

	/*! 
		Returns the sum of the length of all segments added
		assuming that CloseCurve() has been called.
	*/
	double Length() const
	{
		// Make sure that CloseCurve() has been called
		ASSERT(m_length >= 0);

		return m_length;
	}

	void CopyPoints(PointArray* pSamples) const
	{
		pSamples->assign(m_pts.begin(), m_pts.end());
	}

	void CopyTangents(DoubleArray* pTangents) const
	{
		pTangents->resize(m_pts.size());

		for (unsigned i = 0; i < pTangents->size(); i++)
			(*pTangents)[i] = Tangent(i);
	}

	void SubsampleExact(unsigned numSamples, PointArray* pSamples, 
		DoubleArray* pTangents) const
	{
		ASSERT(pSamples && pTangents);

		pSamples->clear();
		pTangents->clear();

		if (Length() == 0 || numSamples == 0)
			return;

		const double delta = Length() / numSamples;

		pSamples->reserve(numSamples + 1);
		pTangents->reserve(numSamples + 1);

		double d, t;
		double cumDist = 0;

		// Start by adding the first point
		pSamples->push_back(m_pts.front());
		pTangents->push_back(Tangent(0));

		// The previous point might not always be one in m_pts. Initially,
		// it is the one that we just added, ie, the first.
		Point prevPt = m_pts.front();

		// Not that dist between first and last is included, so
		// the true last point is the first point, which is not reached twice
		for (unsigned i = 1; i < m_pts.size() && pSamples->size() < numSamples;)
		{
			const Point& pt = m_pts[i];

			d = pt.dist(prevPt);

			cumDist += d;

			if (cumDist == delta)
			{
				pSamples->push_back(pt);
				pTangents->push_back(Tangent(i));

				cumDist = 0;
				prevPt = pt;
				i++;
			}
			else if (cumDist > delta) // inc total distance by d * t
			{
				t = (delta - cumDist + d) / d;
				ASSERT(t >= 0 && t <= 1);

				Point pt_prime = LineSegmentPoint(prevPt, pt, t);
				
				pSamples->push_back(pt_prime);

				// Compute the weighted average tangent using
				// the last added tangent and the next point's one
				double tan_prime = (prevPt == pSamples->back()) 
					? pTangents->back() : Tangent(i - 1);

				pTangents->push_back((1 - t) * tan_prime + t * Tangent(i));
				
				cumDist = 0;
				prevPt = pt_prime;
	
				// do not increment 'i' yet
			}
			else
			{
				prevPt = pt;
				i++;
			}
		}

		/*if (numSamples != pSamples->size())
		{
			DBG_PRINT4(numSamples, pSamples->size(), Length(), delta)
		}*/
		ASSERT(numSamples == pSamples->size());
	}

	/*void Subsample(unsigned numSamples, PointArray* pPts) const
	{
		const double delta = Length() / (double)numSamples;
		
		Subsample(delta, pPts);
	}

	void Subsample(const double& delta, PointArray* pPts) const
	{
		BoundarySegmentList::const_iterator it;
		BoundarySegment* pCurrSeg;
		BoundarySegment* pPrevSeg = NULL;

		double remainder = 0;

		ASSERT(delta > 0);

		pPts->clear();
		pPts->reserve((int) (Length() / delta) + 1);

		std_forall(it, m_segments)
		{
			pCurrSeg = *it;

			// If it's not the first segment, we need to take into account the 
			// distance between the endpoints of the current and previous segments
			if (pPrevSeg)
			{
				// note that the remainder will always be greater than zero in this case!
				remainder -= pCurrSeg->FirstPoint().dist(pPrevSeg->LastPoint());

				// we can't have negative remainders, so correct it if necessary
				if (remainder < 0)
					remainder = 0;
			}

			// If the remainder isn't zero, we need a point that isn't the seg's first
			if (remainder != 0)
				pCurrSeg->Seek(remainder, &remainder);

			// Get all the segment's points at delta intervals
			while (remainder == 0)
			{
				pPts->push_back(pCurrSeg->GetPoint());
				pCurrSeg->Seek(delta, &remainder);
			}		

			pPrevSeg = pCurrSeg;
		}
	}*/
};

} // namespace vpl
