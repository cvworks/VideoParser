/**************************************************************************

   File:                CurveSeg.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: This is an abstract class. It is like ContourCurve, but it
                allows CurveSeg's to be stuck one after the
		other. Technically, this could be a subclass of
		ContourCurve; it is not. The main reason is that the notion
		of a subsegment (coming from ContourCurve's subCurve()) does
		not make much sense.

		The initial intent is to have these as self-contained
		pieces of cuves that can compute distances and such to
		themselves precisely. In other words, in using them,
		it may be reasonable to assume that the distance or
		any other calculation performed by a CurveSeg will
		take a short (constant) time, i.e. there would not be
		subparts called. This is not a strict rule, however.

   $Revision: 1.3 $

   $Log: CurveSeg.h,v $
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

#ifndef CURVESEG_H
#  define CURVESEG_H

#include "Drawable.h"
#include "Point.h"
#include "Vector.h"
#include "ContourCurve.h"

namespace sg {

// abstract class
class CurveSeg
{
public:
    //! the position of the starting point of this segment in the curve
    double startLength;

public:
    virtual ~CurveSeg() {};

	virtual Vector tangent(const double& t) = 0; // unit tangent
    virtual Point atT(const double& t) = 0;
	virtual double getLength() = 0;
    virtual CurveSeg* clone() = 0;
    virtual void drawSeg(Drawable& d) = 0;

	virtual Point firstPt() const = 0;
	virtual Point lastPt() const = 0;

	virtual double squaredDistToFirstPt(const Point& p) = 0;
	virtual double squaredDistToLastPt(const Point& p) = 0;

	virtual void computeDistance(const Point& p, Distance& ret) = 0;

	//! By default, it ignores the limits t1 and t2
	virtual void computeDistance(const Point& p, const double& t1, 
		const double& t2, Distance& ret)
	{
		return computeDistance(p, ret);
	}

	//////////////////////////////////////////////////////////////
	// Non virtual functions

	Vector normal(const double& t) 
	{ 
		return tangent(t).getPerp(); 
	}

	Distance computeDistance(const Point& p, const double& t1, const double& t2)
	{
		Distance ret;

		computeDistance(p, t1, t2, ret);

		return ret;
	}

	Distance computeDistance(const Point& p)
	{
		Distance ret;

		computeDistance(p, ret);

		return ret;
	}

	//! Returns the from p to the closest point on the segment
    double distToPt(const Point& p)
	{
		return computeDistance(p).dist;
	}

	/*! 
		Returns the t-value of the closest point on the segment,
		i.e. if startPt is t=0 and endPt is t=length
	*/
    double closestToPt(const Point &p)
	{
		return computeDistance(p).t;
	}
};

}

#endif  /* CURVESEG_H */
