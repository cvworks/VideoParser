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

#ifndef SG_BOUNDARY_INFO_H
#define SG_BOUNDARY_INFO_H

#include <vector>
#include "Vector.h"
#include "BoundaryPoint.h"

namespace sg {
	
struct BoundaryInfo
{
	BoundaryPoint first;
	BoundaryPoint second;
	double cumAxisDist; //<! cumulative skeleton distance from pt 0 to current

	BoundaryInfo() 
	{ 
		cumAxisDist = 0; 
	}

	void clear()
	{
		first.clear();
		second.clear();
		cumAxisDist = 0;
	}

	//! [static] Gets the opposite side to the given side
	static char FlipSide(char side)
	{
		ASSERT(side == '1' || side == '2');

		return (side == '1') ? '2' : '1';
	}

	const BoundaryPoint& operator[](char i) const { return (i == '1') ? first:second; }
	BoundaryPoint& operator[](char i)             { return (i == '1') ? first:second; }

	void SwapSides()
	{ 
		BoundaryPoint aux = first; 
		first = second; 
		second = aux; 
	}

	//! It is true iff at least one bndry point belongs to a bndry gap segment
	bool IsInterpolated() const
	{
		return (first.IsInterpolated() || second.IsInterpolated());
	}

	friend std::ostream& operator<<(std::ostream& os, const BoundaryInfo& bi)
	{
		return os << "[bp1:" << bi.first << ",\n bp2:" << bi.second
			<< ", cumAxisDist:" << bi.cumAxisDist << ']';
			//<< ",\n alpha:" << bi.alpha << ", alpha_p:" << bi.alpha_p
			//<< ", tangent:" << bi.tangent << ", cumAxisDist:" << bi.cumAxisDist << ']';
	}
};

typedef std::vector<BoundaryInfo> BoundaryInfoArray;

} //namespace sg

#endif // SG_BOUNDARY_INFO_H
