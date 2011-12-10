/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __BOUNDARY_CORNER_H__
#define __BOUNDARY_CORNER_H__

#include <list>
#include <vector>
#include "FluxSkeleton\include\ShapeBoundary.h"

namespace vpl {

/*! 
	Represents a boundary corner
*/
struct BoundaryCorner
{
	sg::BoundaryInterval interval;
	int index;
	double angleCos;
	int spokeCount;

	void Print(const sg::ShapeBoundary* pShape, 
		const sg::SymPtListMap& splm, std::ostream& os) const;
};

typedef std::list<BoundaryCorner> BoundaryCornerList;

struct CornerLabel
{
	bool isCorner;
	bool isMinimum;
	BoundaryCornerList::const_iterator cornerIt;

	CornerLabel(bool bCorner = false, bool bMinimum = false)
	{
		isCorner = bCorner;
		isMinimum = bMinimum;
	}
};

typedef std::vector<CornerLabel> CornerLabelArray;

} // namespace vpl

#endif //__BOUNDARY_CORNER_H__
