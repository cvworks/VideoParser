/**************************************************************************

   File:                DDSGraph.h

   Author(s):           Pavel Dimitrov

   Created:             22 Jul 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: 

   $Revision: 1.2 $

   $Log: DDSGraph.h,v $
   Revision 1.2  2002/07/25 20:50:47  pdimit
   Making release 0.1

   Revision 1.1  2002/07/23 21:02:51  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DDSGraph is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef SG_FLUX_POINT_H
#define SG_FLUX_POINT_H

#include <vector>
#include "Point.h"

namespace sg {

struct FluxPoint
{
	Point p;     //<! position
	double val;  //<! flux value
	double dist; //<! closest distance 
	char col;    //<! for the thinning and skeleton

	FluxPoint() 
	{ 
		dist = 0;
		val  = 0;
		col  = -1;
	}

	FluxPoint(const Point& pt, const double& div, const double& d, char c)
	{
		dist = d;
		p    = pt;
		val  = div;
		col  = c;
	}

	friend std::ostream& operator<<(std::ostream& os, const FluxPoint& fp)
	{
		return os << " (" << fp.p.x << ", " << fp.p.y << ", " << fabs(fp.dist) << ')';
	}

	bool operator==(const FluxPoint& fp) const
	{
		return ((p == fp.p) && (val == fp.val) && (dist == fp.dist) && (col == fp.col));
	}

	bool operator!=(const FluxPoint& fp) const
	{
		return !operator==(fp);
	}

	bool operator<(const FluxPoint &dp) const
	{
		return val < dp.val;
	}

	double radius() const { return fabs(dist); }
	double squaredRadius() const { return dist * dist; }
};

typedef std::vector<FluxPoint> FluxPointArray;

} //namespace sg

#endif // SG_FLUX_POINT_H
