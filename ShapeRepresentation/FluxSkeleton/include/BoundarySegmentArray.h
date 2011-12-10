/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef SG_BOUNDARY_SEGMENT_LISTS_H
#define SG_BOUNDARY_SEGMENT_LISTS_H

#include <vector>
#include "Point.h"

namespace sg {

//! Boundary point array used to represent a contour with "gaps" in it
struct BoundarySegment
{
	int start, end;            //<! limits of the boundary segment
	std::vector<Point> gapPts; //<! points between this segment and the next bndry segment

	const Point& operator[](unsigned int i) const { return gapPts[i]; }
	Point& operator[](unsigned int i) { return gapPts[i]; }

	//! Add pt at the back of the gapPts vector
	void addGapPoint(const Point& pt) { gapPts.push_back(pt); }

	void addGapPoints(const Point& pt0, const Point& pt1, bool bIncludeLast);
};

typedef std::vector<BoundarySegment> BoundarySegments;

//! Lists of boundary segments and gaps on each side of the branch
struct BoundarySegmentArray
{
	BoundarySegments first, second;

	BoundarySegments& operator[](char i)
	{
		return (i == '1') ? first : second;
	}

	const BoundarySegments& operator[](char i) const
	{
		return (i == '1') ? first : second;
	}

	bool empty() const { return first.empty() && second.empty(); }
};

} //namespace sg

#endif // SG_BOUNDARY_SEGMENT_LISTS_H
