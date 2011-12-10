/**************************************************************************

   File:                CircSeg.h

   Author(s):           Pavel Dimitrov

   Created:             21 Jun 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: Implements CurveSeg (see CurveSeg.h). Provides
                circular arc segments.

   $Revision: 1.3 $

   $Log: CircSeg.h,v $
   Revision 1.3  2002/07/25 20:50:47  pdimit
   Making release 0.1

   Revision 1.2  2002/06/27 14:14:39  pdimit
   Just updated the descriptions in the header files.

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef CIRCSEG_H
#define CIRCSEG_H

#include <Tools/MathUtils.h>
#include "defines.h"
#include "Point.h"
#include "Vector.h"
#include "CurveSeg.h" // base class

namespace sg{

  // concrete class
  class CircSeg : public CurveSeg
  {
    // private variables

    double length;

    Point p0, p1;  // start and end point resp.
    double t0, t1; // angles for 1st and 2nd point resp.
    double r;      // radius
    Point centre;

    double sign;  // this gives the orientation (counter-clockwise CCW...)

    // private methods

    double point_to_rad(const Point &p); // returns the standard 0 to 2Pi pos
    bool rad_in_arc(double t); // verifies if an angle is inside the arc

	// reparametrization -- t is for the curve and this is for the circle
    double s(const double& t) 
    { 
		return t0 + sign * t / r; 
	}
    
  public:
    
    CircSeg() {}
    CircSeg(Point p0, Point p1, Point c, double sign=CCW);

	virtual Point firstPt() const
	{
		return p0;
	}

	virtual Point lastPt() const
	{
		return p1;
	}

	virtual double squaredDistToFirstPt(const Point& p)
	{
		return p.SqDist(p0);
	}

	virtual double squaredDistToLastPt(const Point& p)
	{
		return p.SqDist(p1);
	}

    virtual Point atT(const double& t)
	{
      return Point(centre.x + r * cos(s(t)),
		   centre.y + r * sin(s(t)));
    }

    virtual void drawSeg(Drawable &d) {}

    virtual double getLength() { return length; }

	virtual void computeDistance(const Point& p, Distance& ret);

    virtual Vector tangent(const double& t)
	{
      return Vector(-sin(s(t)), cos(s(t)));
    }

    virtual CurveSeg* clone()
	{
      CircSeg *cs = new CircSeg();

      cs->p0 = p0;
      cs->p1 = p1;
      cs->centre = centre;
      cs->sign = sign;
      cs->t0 = t0;
      cs->t1 = t1;
      cs->r = r;
      cs->length = length;

      return cs;
    }


    //  debugging code
    friend std::ostream &operator<< (std::ostream &Out, CircSeg &cs)
	{
      Out << "CircSeg[(" << cs.p0 << ", " << cs.p1 << ", ";
      Out << cs.centre << ", r=" << cs.r << ", l=" << cs.length << ", t0=";
      Out << cs.t0 << ", t1=" << cs.t1 << "]";// << cs.length << ", ";

      return Out;
    }
  };

}

#endif  /* CIRCSEG_H */
