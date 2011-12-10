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

#ifndef SG_BOUNDARY_POINT_H
#define SG_BOUNDARY_POINT_H

#include <iostream>
#include "Point.h"

namespace sg {

struct BoundaryPoint 
{
	Point pt;            //<! boundary point coordinates
	int index;           //<! position in the bndry-point array. (idx < 0) => gap pt array index = -idx - 1
	int subindex;        //<! index on bndry-point array for gap pts (-1 for non gap points)
	double cumBndryDist; //<! cumulative boundary distance from pt 0 to current

	BoundaryPoint() 
	{ 
		clear(); 
	}

	void clear() 
	{ 
		index = -1; 
		subindex = -1;
		cumBndryDist = -1;
	}

	void set(const double& x, const double& y, int idx, int subidx = -1)
	{
		pt.x = x;
		pt.y = y;
		index = idx;
		subindex = subidx;
	}

	//! It is true iff the point blongs to a boundary gap segment
	bool IsInterpolated() const
	{
		return (index < 0 && subindex >=0);
	}

	friend std::ostream& operator<<(std::ostream& os, const BoundaryPoint& bp)
	{
		return os << "(x:" << bp.pt.x << ", y:" << bp.pt.y << ", i:" << bp.index  
			<< ", si:" << bp.subindex << ", cumBD:" << bp.cumBndryDist << ')';
			//<< ", BAR:" << bp.bar << ", cumBD:" << bp.cumBndryDist << ", dt:" << bp.dtValue << ')';
	}
};

} //namespace sg

#endif // SG_BOUNDARY_POINT_H
