/**************************************************************************

File:                ShapeBoundary.h

Author(s):           Pavel Dimitrov

Created:             17 Jun 2002

Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
McGill University, Montreal, QC.  Please see the copyright notice
included in this distribution for full details.

**************************************************************************/

#ifndef _SHAPE_BOUNDARY_H_
#define _SHAPE_BOUNDARY_H_

#include <vector>
#include "defines.h"
#include "Point.h"
#include "ContourCurve.h"
#include "CurveSeg.h"
#include "SkeletalInfo.h"
#include "BoundaryInterval.h"

namespace sg {

class DDSGraph;

/*! 
	Concrete shape class. It represents a shape boundary as an
	array of curves of generic type. The curves may encode
	any type of shape, such as a closed curve or a shape with holes.
	It has only been tested with shapes encoded by a single closed curve.
*/
class ShapeBoundary 
{
protected:
	double m_xmin, m_xmax, m_ymin, m_ymax;

	std::vector<ContourCurve*> m_contourCurves; //!< All boundary points

	/*! 
		Mapping from boundary points to skeletal points. 
		
		Its is not serialized explicitly because it includes references 
		to branch pointers. It must be recomputed by calling 
		setSkeletalPoints() once the DDSGraph is deserialized.
		
		@see DDSGraph::Deserialize
	*/
	SkeletalInfo m_skeletalInfo; 

public:
	//! Default constructor needed for deserialization only
	ShapeBoundary()
	{
	}

	ShapeBoundary(std::vector<CurveSeg*>& segs, bool closed)
	{
		ASSERT(false);
		//m_contourCurves.push_back(
		//	ShapeMaker::CreateCurve(segs, closed));
	}

	ShapeBoundary(ContourCurve* pContour) : m_contourCurves(1, pContour)
	{
		// nothing else to do
	}

	ShapeBoundary(const ContourCurve& contour) : m_contourCurves(1, contour.clone())
	{
		// nothing else to do
	}

	~ShapeBoundary()
	{ 
		for (unsigned int i = 0; i < m_contourCurves.size(); i++)
			delete m_contourCurves[i];
	}

	ShapeBoundary* clone() const
	{
		ASSERT(m_contourCurves.size() == 1);

		// @todo This assumes that there is onlyone curve
		ShapeBoundary* pShape = new ShapeBoundary(*m_contourCurves.front());

		pShape->setBounds(m_xmin, m_xmax, m_ymin, m_ymax);

		return pShape;
	}

	void Serialize(OutputStream& os) const;
	void Deserialize(InputStream& is);

	/*!
		Gets the spokes map associated with the i'th boundary point.
		It assumes that there is only one curve!
	*/
	const SpokesMap& getSpokesMap(unsigned int i) const
	{
		return m_skeletalInfo[i];
	}

	const std::vector<ContourCurve*>* getCurves() const 
	{ 
		return &m_contourCurves; 
	}

	std::vector<ContourCurve*>* getCurves()
	{ 
		return &m_contourCurves; 
	}

	void getBounds(double *xmin, double *xmax,
		double *ymin, double *ymax) const
	{
		*xmin = m_xmin;
		*xmax = m_xmax;
		*ymin = m_ymin;
		*ymax = m_ymax;
	}

	void setBounds(const double& xmin, const double& xmax,
		const double& ymin, const double& ymax)
	{
		m_xmin = xmin;
		m_xmax = xmax;
		m_ymin = ymin;
		m_ymax = ymax;
	}

	//! Gets the boundary points in the specified contour curve
	KDTree* getKDTree(unsigned int curveIdx = 0)
	{
		return m_contourCurves.at(curveIdx)->getKDTree();
	}

	//! Gets the number of points in the specified contour curve
	unsigned int size(unsigned int curveIdx = 0) const
	{ 
		return m_contourCurves.at(curveIdx)->numPts();
	}

	//! Gets the i'th point in the specified contour curve
	void getPoint(unsigned int curveIdx, 
		unsigned int i, Point* pt) const
	{
		m_contourCurves.at(curveIdx)->getPoint(i, pt);
	}

	//! Gets the i'th point in the first contour curve
	void getPoint(unsigned int i, Point* pt) const
	{
		m_contourCurves.front()->getPoint(i, pt);
	}

	//! Gets the wrap-around i'th point in the first contour curve
	void getPointCircular(int i, Point* pt) const
	{
		m_contourCurves.front()->getPointCircular(i, pt);
	}

	//! Gets a copy of all the points in the first contour curve
	void getPoints(PointArray* pts) const
	{
		m_contourCurves.front()->getPoints(pts);
	}

	//! Gets a reference to all the points in the first contour curve
	const PointArray& getPoints() const
	{
		return m_contourCurves.front()->getPoints();
	}

	void setSkeletalPoints(const DDSGraph* pDDSGraph);
};

} //namespace sg

#endif  // _SHAPE_BOUNDARY_H_
