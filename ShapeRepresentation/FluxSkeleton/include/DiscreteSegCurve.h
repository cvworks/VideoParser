/**************************************************************************

   File:                DiscreteSegCurve.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: This is a type of ContourCurve (see ContourCurve.h) that is made up of
                some number of CurveSeg's. These can be anything, so
		long as they provide the necessary functionality; that
		is the same as a ContourCurve but being aware that they
		follow one another (see CurveSeg.h).

		This class is meant as a "container" for any mix of
		curve m_segments, e.g. lines, arcs or spline m_segments.

   $Revision: 1.3 $

   $Log: DiscreteSegCurve.h,v $
   Revision 1.3  2002/07/25 20:50:47  pdimit
   Making release 0.1

   Revision 1.2  2002/06/27 14:14:40  pdimit
   Just updated the descriptions in the header files.

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import
	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef DISCRETESEGCURVE_H
#define DISCRETESEGCURVE_H

#include <vector>
#include "defines.h"
#include "Point.h"
#include "Vector.h"
#include "CurveSeg.h"
#include "ContourCurve.h" // parent class

namespace sg{

  class DiscreteSegCurve : public ContourCurve
  {
  public:
    std::vector<CurveSeg*> m_segments;

	static int s_curveSegmentClassId;

	static CurveSeg* CreateCurveSegment(const Point& p0, const Point& p1);

  public:
    // constructors/destructors
    DiscreteSegCurve() {}

	DiscreteSegCurve(const std::vector<Point>& pts) 
		: ContourCurve(pts)
	{
		// Make the shape's curve
		std::vector<Point>::const_iterator I = pts.begin();
		
		Point fp = *I; // the first point
		Point pp = *I; // the previous point

		m_segments.reserve(pts.size() + 1);

		m_length = 0;

		// Go to the next point and keep iterating
		for(++I; I != pts.end(); ++I)
		{
			m_segments.push_back(CreateCurveSegment(pp, *I));

			m_length += m_segments.back()->getLength(); // update total curve length

			pp = *I; // update previous point
		}

		// Close curve
		if(pp.x != fp.x || pp.y != fp.y)
			m_segments.push_back(CreateCurveSegment(pp, fp));
	}
    
    virtual ~DiscreteSegCurve() 
	{ 
		// remove segs since they were cloned
		std::vector<CurveSeg*>::iterator I;

		for(I = m_segments.begin(); I != m_segments.end(); I++)
		{
			delete *I;
		}
    }

	virtual void Serialize(OutputStream& os) const
	{
		ContourCurve::Serialize(os);

		//@todo add needed code
		ASSERT(false);
	
		/*
		int size = c->m_segments.size();
		os.write((char*) &size, sizeof(size));

		std::vector<CurveSeg*>::iterator I;
		for(I = c->m_segments.begin(); I != c->m_segments.end(); I++)
		{
			pLineSeg = dynamic_cast<LineSeg*>(*I);
			ASSERT(pLineSeg);

			os.write((char*) &pLineSeg->startLength, sizeof(pLineSeg->startLength));
			os.write((char*) &pLineSeg->startPt, sizeof(pLineSeg->startPt));
			os.write((char*) &pLineSeg->endPt, sizeof(pLineSeg->endPt));
		}
		*/
	}

	virtual void Deserialize(InputStream& is)
	{
		ContourCurve::Deserialize(is);

		//@todo add needed code
		ASSERT(false);

		/*
		LineSeg* pLineSeg;
		bool closed;
		int size;
		double startLength;
		Point startPt, endPt;

		std::vector<CurveSeg*> segments;
		is.read((char*) &size, sizeof(size));

		for(int i = 0; i < size; i++)
		{
			is.read((char*) &startLength, sizeof(startLength));
			is.read((char*) &startPt, sizeof(startPt));
			is.read((char*) &endPt, sizeof(endPt));

			pLineSeg = new LineSeg(startPt, endPt, startLength);
			segments.push_back(pLineSeg);
		}*/
	}

    /////////////////////////////////////////////////////////////////////
    // Required methods
    virtual double distToPt(const Point &p);
	virtual Point atT(const double& t);
	
	virtual Vector tangent(unsigned int idx)
	{
		ASSERT(idx >= 0 && idx < m_segments.size());

		CurveSeg* cs = m_segments[idx];

		return cs->tangent(cs->startLength);
	}

	virtual Vector tangent(const double& t);
	virtual double closestToPt(const Point &p);
	virtual void computeDistance(const Point &p, Distance& dst);
	
	virtual ContourCurve* clone() const
	{
		DiscreteSegCurve *dsc = new DiscreteSegCurve(*this);

		std::vector<CurveSeg*>::const_iterator I;

		for(I = m_segments.begin(); I != m_segments.end(); ++I)
		{
			dsc->m_segments.push_back((*I)->clone());
		}  

		return dsc;
	}

	virtual unsigned int numPts() const
	{
		return m_segments.size();
	}

	virtual void getPoint(unsigned int i, Point* pt) const
	{
		*pt = m_segments[i]->firstPt();
	}
    
    // Non reqyuired methods
	int get_num_segs() { return m_segments.size(); }

    friend std::ostream &operator<< (std::ostream &Out, DiscreteSegCurve &dsc){

      Out << "[DiscreteSegCurve]: length:" << dsc.m_length;
      Out << " #segs:" << dsc.m_segments.size();
      
      return Out;
    }    
  };
}

#endif  /* DISCRETESEGCURVE_H */
