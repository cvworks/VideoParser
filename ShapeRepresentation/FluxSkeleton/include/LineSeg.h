/**************************************************************************

   File:                LineSeg.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: This is an implementation of CurveSeg. It provides
                line segments. (See CurveSeg.h)

   $Revision: 1.3 $

   $Log: LineSeg.h,v $
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

#ifndef LINESEG_H
#define LINESEG_H

#include <iostream>

#include "defines.h"

#include "CurveSeg.h"
#include "Point.h"
#include "Vector.h"

namespace sg{
  
// concrete class
class LineSeg : public CurveSeg
{
public:
    Point startPt;
    Point endPt;
    
    Vector v; // tangent vector
    double length; // of the line segment
 
public:
    //constructors
    LineSeg(const Point& sP, const Point& eP, const double& sl = 0) 
		: startPt(sP), endPt(eP)
	{
      startLength = sl;

      v.x = eP.x - sP.x;
      v.y = eP.y - sP.y;
      
      length = v.norm();

      v.x /= length; // in order to parametrize by t=arc_length
      v.y /= length;
    }

	//////////////////////////////////////////////////////////////
    // Required method definitions
    virtual double getLength()
	{ 
		return length; 
	}

	virtual Point firstPt() const
	{
		return startPt;
	}

	virtual Point lastPt() const
	{
		return endPt;
	}

	virtual double squaredDistToFirstPt(const Point& p)
	{
		return p.SqDist(startPt);
	}

	virtual double squaredDistToLastPt(const Point& p)
	{
		return p.SqDist(endPt);
	}

    //! Returns the Point at location t
    virtual Point atT(const double& t)
	{
		if (t <= length)
			return Point(startPt.x + t * v.x, startPt.y + t * v.y);
		else
			return Point(0, 0);
	}

	virtual void computeDistance(const Point& p, const double& t1, 
		const double& t2, Distance& ret);

	virtual void computeDistance(const Point& p, Distance& ret)
	{
		computeDistance(p, 0, length, ret);
	}

    virtual Vector tangent(const double& t) 
	{
		return v;
    }

    virtual CurveSeg* clone()
	{
      LineSeg *ls = new LineSeg(startPt, endPt, startLength);

      ls->v = v;
      ls->length = length;

      return ls;
    }
    
    virtual void drawSeg(Drawable &d)
	{ 
      d.drawLine(startPt, endPt);
    } 

    friend std::ostream &operator<< (std::ostream& Out, LineSeg& ls)
	{
		Out << "LineSeg[(" << ls.startPt.x << ", " << ls.startPt.y << ")-";
		Out << "(" << ls.endPt.x << ", " << ls.endPt.y << "), " << ls.length << "]";

		return Out;
    }
};

}

#endif  /* LINESEG_H */
