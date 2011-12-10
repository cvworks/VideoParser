/**************************************************************************

File:                ContourCurve.h

Author(s):           Pavel Dimitrov

Created:             11 Jun 2002

Last Revision:       $Date: 2002/07/25 20:50:47 $

Description: This provides the definition of a general purpose ContourCurve.
It is an abstract class that defines the
interface. The curve is supposed to be parametrize by
parameter t, i.e. c(t). One can get c'(t)/|c'(t)| (the
unit tangent) and hence a unit normal. It is also
possible to get the closest point on the curve for any
point in the plane. The distance is SIGNED. The actual
sign depends on the parametrization and the
calculation is as follows:
given point P in the plane
1) find point Q on c(t) closest to P.
2) Let T(Q) be the unit tangent of c(t) at Q and
denote by v the vector (P-Q), i.e. from the curve to
the point P; then the result (signed distance) is
given by
|T(Q) x v| = T(Q)_x * v_y - T(Q)_y * v_x

The sign of this gives the sign of the distance. Note
that it may be the case that the above expression is
not equal to the signed distance -- this happens
whenever there is no unambiguous tangent to the curve
at the closest point on the curve. If there is a
point Q on the curve where the tangent (derivative c'(t))
is not well defined, then it is -- in general --
impossible to decide the sign of the points on the
plane for which P is the closest point on the curve.

Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.
**************************************************************************/
#ifndef CURVE_H
#define CURVE_H

#include "defines.h"
#include "Drawable.h"
#include "Vector.h"
#include <Tools/KDTree.h>

namespace sg {

//! a structure for returning the closest distance to the curve
struct Distance
{
	double dist;  // the distance
	double t;     // the point as c(t) where c is the curve/segment
	Point p;      // the actual point
};

/*! 
	An concrete closed curve class with boundary points stored in a kd-tree
*/
class ContourCurve 
{  
public:
	typedef PointArray::const_iterator const_iterator;

protected:
	PointArray m_pts;
	double m_length;

private:
	KDTree* m_pKDTree; //!< Contructed only if requested with getKDTree()

public:
	ContourCurve() 
	{ 
		m_length = 0; 
		m_pKDTree = NULL;
	}

	ContourCurve(const ContourCurve& c)
	{
		m_pts     = c.m_pts;
		m_length  = c.m_length;
		m_pKDTree = NULL; // don't copy c's jd-tree!!!
	}

	ContourCurve(const std::vector<Point>& pts)
	{
		m_pts     = pts;
		m_length  = 0;
		m_pKDTree = NULL;
	}
	
	void BuildKDTree()
	{
		ASSERT(!m_pKDTree);

		m_pKDTree = new KDTree(m_pts.size());
		m_pKDTree->AddDataPoints(m_pts);
		m_pKDTree->Build();
	}

	const_iterator begin() const
	{
		return m_pts.begin();
	}

	const_iterator end() const
	{
		return m_pts.end();
	}

	const Point& operator[](unsigned i) const
	{
		return m_pts[i];
	}

	virtual ~ContourCurve()
	{
		delete m_pKDTree;
	}

	virtual void Serialize(OutputStream& os) const
	{
		::Serialize(os, m_length);
		::Serialize(os, m_pts);	
	}

	virtual void Deserialize(InputStream& is)
	{
		if (m_pKDTree)
		{
			delete m_pKDTree;
			m_pKDTree = NULL;
		}

		::Deserialize(is, m_length);
		::Deserialize(is, m_pts);
	}

	/////////////////////////////////////////////////////////////////////
    // Pure virtual methods
	virtual double distToPt(const Point& p) = 0;
	virtual Point atT(const double& t) = 0;
	virtual Vector tangent(unsigned int idx) = 0;

	//! The unit tangent at c(t),i.e. c'(t)
	virtual Vector tangent(const double& t) = 0;

	virtual double closestToPt(const Point &p) = 0;

	virtual void computeDistance(const Point &p, Distance& dst) = 0;

	virtual ContourCurve* clone() const = 0;

	/////////////////////////////////////////////////////////////////////
    // Non pure virtual methods

	KDTree* getKDTree()
	{
		if (!m_pKDTree)
			BuildKDTree();

		return m_pKDTree;
	}

	double getLength()
	{ 
		return m_length; 
	}

	unsigned int numPts() const
	{
		return m_pts.size();
		//return m_pKDTree->Size();
	}

	double distToPt(const double& x, const double& y)
	{
		return distToPt(Point(x,y));
	}

	//! Reference to the array of boundary points
	const PointArray& getPoints() const
	{
		return m_pts;
	}

	//! Copies all boundary points into the array
	void getPoints(PointArray* pts) const
	{
		*pts = m_pts;
		//m_pKDTree->GetDataPoints(*pts);
	}

	void getPoint(unsigned int i, Point* pt) const
	{
		*pt = m_pts[i];
		//m_pKDTree->GetDataPoint((int)i, pt->x, pt->y);
	}

	//! It lets i be smaller tan 0 and greater than numPts()
	void getPointCircular(int i, Point* pt) const
	{
		int idx;
			
		if (i < 0)
			idx = numPts() - (abs(i) % numPts());
		else
			idx = i % numPts();

		*pt = m_pts[idx];
		//m_pKDTree->GetDataPoint(idx, pt->x, pt->y);
	}
	
	Point getPoint(const double& t) 
	{ 
		return atT(t);
	}
	
	Vector normal(const double& t) 
	{ 
		return tangent(t).getPerp(); 
	}
};

}

#endif  /* CURVE_H */
