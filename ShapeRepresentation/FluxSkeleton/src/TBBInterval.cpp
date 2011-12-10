/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "TBBInterval.h"
#include "DDSGraphUtils.h"

using namespace sg;

//////////////////////////////////////////////////////////////////////////////////////////
// TBBInterval member functions

/*!
	@brienf Sets the boundary interval defined by the pair
	of adjacent branches pBranch0 and pBranch1. Sides are set wrt the side
	of branch pBranch0.

	Note: when creating the interval we need to make sure that
	side 1 and 2 in one branch correspond to side 1 and 2 in the other.
	There are at least two ways of checking this. One is evaluating whether
	the tangents use to decide sides in each branch are equaly oriented or not.
	The other one is by analyzing an XOR property of inclusion that the 
	internal/external boundary endpoint must satisfy. This property says that
	if we associate the external point in one branch with the correct internal point
	on the other (ie, correct sides), then it must be true that there are no other 
	internal/external points in between these two OR, if we defined the interval
	backwards, then we should find both internal/external points included. It's good
	to draw the cases to see the validity of property (the is a special case when 
	indices are equal). Here we test both properties to make sure that all is right.
*/
void TBBInterval::SetLimits(int bndryLength)
{
	ASSERT(bndryLength > 0);

	// Create handy aliases
	const SkelPtCoord& b0p0 = pBranch0->firstXYPoint();
	const SkelPtCoord& b0pN = pBranch0->lastXYPoint();

	const SkelPtCoord& b1p0 = pBranch1->firstXYPoint();
	const SkelPtCoord& b1pN = pBranch1->lastXYPoint();

	const SkelPtBndryInfo& b0i0 = pBranch0->firstBndryInfo();
	const SkelPtBndryInfo& b0iN = pBranch0->lastBndryInfo();

	const SkelPtBndryInfo& b1i0 = pBranch1->firstBndryInfo();
	const SkelPtBndryInfo& b1iN = pBranch1->lastBndryInfo();

	bool bSwapSides;

	// Set the external and internal point of each branch
	if (b0p0 == b1pN) // case: N...0~N...0
	{
		extPt0 = b0iN;
		intPt0 = b0i0;

		extPt1 = b1i0;
		intPt1 = b1iN;

		bSwapSides = false;
	}
	else if (b0p0 == b1p0) // case: N...0~0...N
	{
		extPt0 = b0iN;
		intPt0 = b0i0;

		extPt1 = b1iN;
		intPt1 = b1i0;

		bSwapSides = true; // tangents are computed differently
	}
	else if (b0pN == b1p0) // case: 0...N~0...N
	{
		extPt0 = b0i0;
		intPt0 = b0iN;

		extPt1 = b1iN;
		intPt1 = b1i0;

		bSwapSides = false;
	}
	else // case: 0...N~N...0
	{
		ASSERT(b0pN == b1pN);

		extPt0 = b0i0;
		intPt0 = b0iN;

		extPt1 = b1i0;
		intPt1 = b1iN;

		bSwapSides = true; // tangents are computed differently
	}

#ifdef _DEBUG
	// Check that XOR condition (see note above) agrees with bSwapSides.
	// If needed, swap the sides of branch 1
	BoundaryInterval test(bndryLength, extPt0.first.index, intPt1.first.index);
	bool bSidesWereSwapped = false;

	bool incExtPt0 = test.Includes(extPt0.second.index);
	bool incIntPt1 = test.Includes(intPt1.second.index);

	// Test the XOR property of internal/external branch inclusion
	if ((incExtPt0 && !incIntPt1) || (!incExtPt0 && incIntPt1) ||
		(extPt0.second.index == intPt1.first.index))
	{
		extPt1.SwapSides();
		intPt1.SwapSides();
		bSidesWereSwapped = true;
	}

	if (bSwapSides != bSidesWereSwapped)
	{
		/*DBG_MSG2(bSwapSides, bSidesWereSwapped)
		
		DBG_PRINT1(b0p0)
		DBG_PRINT1(b0pN)
		DBG_PRINT1(b1p0)
		DBG_PRINT1(b1pN)
		DBG_LINE
		DBG_PRINT1(b0i0)
		DBG_PRINT1(b0iN)
		DBG_PRINT1(b1i0)
		DBG_PRINT1(b1iN)*/

		extPt1.SwapSides();
		intPt1.SwapSides();
	}

	//ASSERT(bSwapSides == bSidesWereSwapped);
	WARNING(bSwapSides != bSidesWereSwapped, 
		"XOR condition disagrees with tangent-based condition");

	// the assertion does not work in a "star" case with a tiny branch
	// in the middle where the BI of endpoints is rather arbitrary
#else
	//  If needed, swap the sides of branch 1
	if (bSwapSides)
	{
		extPt1.SwapSides();
		intPt1.SwapSides();
	}
#endif //_DEBUG

	// Set the interval defined by the two branches
	firstSideInt.Set(bndryLength, extPt0.first.index, extPt1.first.index);
	secondSideInt.Set(bndryLength, extPt0.second.index, extPt1.second.index);

	// Make sure that the first interval is well-defined. It should be
	// true that the interval doesn't include the second side's internal 
	// and external points. It's enough to test one internal point only.

	int n = 0;

	if (firstSideInt.Includes(intPt0.second.index)) n++;
	if (firstSideInt.Includes(intPt1.second.index)) n++;
	if (firstSideInt.Includes(extPt0.second.index)) n++;
	if (firstSideInt.Includes(extPt1.second.index)) n++;

	WARNING1(n > 0 && n < 4, "Strange case 1", n);

	if (n > 2)
		firstSideInt.Swap();

	/*if (firstSideInt.Includes(intPt0.second.index))
	{
		ASSERT(firstSideInt.Includes(intPt1.second.index));
		firstSideInt.Swap();
	}*/

	if (!firstSideInt.Includes(intPt0.first.index) && 
		!firstSideInt.Includes(intPt1.first.index))
	{
		WARNING1(true, "Fixing boundary interval 1...", n)
		firstSideInt.Swap();
	}

	// Interval should include AT LEAST ONE internal branch
	WARNING(!firstSideInt.Includes(intPt0.first.index) && 
		    !firstSideInt.Includes(intPt1.first.index),
		    "Boundary interval problem (1) could not be fixed!!!");

	// Make sure that the second interval is well-defined. It should be
	// true that the interval doesn't include the first side's internal 
	// and external points. It's enough to test one internal point only.
	n = 0;

	if (secondSideInt.Includes(intPt0.first.index)) n++;
	if (secondSideInt.Includes(intPt1.first.index)) n++;
	if (secondSideInt.Includes(extPt0.first.index)) n++;
	if (secondSideInt.Includes(extPt1.first.index)) n++;

	WARNING1(n > 0 && n < 4, "Strange case 2", n);

	if (n > 2)
		secondSideInt.Swap();

	/*if (secondSideInt.Includes(intPt0.first.index))
	{
		ASSERT(secondSideInt.Includes(intPt1.first.index));
		secondSideInt.Swap();
	}*/

	if (!secondSideInt.Includes(intPt0.second.index) && 
		!secondSideInt.Includes(intPt1.second.index))
	{
		WARNING1(true, "Fixing boundary interval 2...", n)
		secondSideInt.Swap();
	}

	// Interval should include AT LEAST ONE internal branch
	WARNING(!secondSideInt.Includes(intPt0.second.index) && 
		    !secondSideInt.Includes(intPt1.second.index),
			"Boundary interval problem (2) could not be fixed!!!");
}

/*!
	@brief Returns the attachment side of pBranch wrt the interval.
	It checks that ALL the 4 boundary points of the given
	branch are included within either side of the two branch interval.

	@return 1 if all 4 points are within side 1
	        2 if all 4 points are within side 2
			0 otherwise
*/
int TBBInterval::Side(const SkelBranch* pBranch, const SkelJoint* pJoint) const
{
	const SkelPtBndryInfo& bi = GetEndpointInfo(pBranch, pJoint);
	
	if (intPt0.first.pt == bi.first.pt || intPt0.first.pt == bi.second.pt ||
		intPt1.first.pt == bi.first.pt || intPt1.first.pt == bi.second.pt)
		return 1;
	else if (intPt0.second.pt == bi.first.pt || intPt0.second.pt == bi.second.pt ||
		     intPt1.second.pt == bi.first.pt || intPt1.second.pt == bi.second.pt)
		return 2;

	WARNING(true, "No clear side");

	ASSERT(false);

	return 1;

	/*int indices[4] = {
		pBranch->firstBndryInfo().first.index,
		pBranch->lastBndryInfo().first.index,
		pBranch->firstBndryInfo().second.index,
		pBranch->lastBndryInfo().second.index,
	};

	int nIncInSide1 = 0;
	int i;

	for (i = 0; i < 4; i++)
		if (firstSideInt.Includes(indices[i]))
			nIncInSide1++;

	if (nIncInSide1 == 4)
		return 1;

	int nIncInSide2 = 0;

	for (i = 0; i < 4; i++)
		if (secondSideInt.Includes(indices[i]))
			nIncInSide2++;
			
	if (nIncInSide2 == 4)
		return 2;

	WARNING2(true, "No clear side", nIncInSide1, nIncInSide2);

	return (nIncInSide1 >= nIncInSide2) ? 1 : 2;*/
}

/*!
	@brief 
*/
bool TBBInterval::IsValid() const
{
	if (firstSideInt.Inside(extPt0.second.index) || firstSideInt.Inside(extPt1.second.index))
		return false;

	if (secondSideInt.Inside(extPt0.first.index) || secondSideInt.Inside(extPt1.first.index))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// TBBIntervalList member functions

/*!
	@brief Helper function for LabelJoint(). It's a recursive function.
*/
void TBBIntervalList::FindAllIntervals(BranchIt iBranch, BranchIt iEnd)
{
	if (iBranch != iEnd)
	{
		TBBInterval interval;

		interval.pBranch0 = *iBranch;

		// Recursive call to add all intervals not related to the current branch
		FindAllIntervals(++iBranch, iEnd);

		// For each branch, make sure that we
		for (; iBranch != iEnd; iBranch++)
		{
			interval.pBranch1 = *iBranch;

			interval.SetLimits(m_nBndrySize);

			push_back(interval);
		}
	}
}

/*!
	@brief Rasterizes a line using Bresenham's line algorithm
*/
/*public void RasterizeLine(int x0, int y0, int x1, int y1, Color color)
{
    int pix = color.getRGB();
    int dy = y1 - y0;
    int dx = x1 - x0;
    int stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;                                                  // dy is now 2*dy
    dx <<= 1;                                                  // dx is now 2*dx

    raster.setPixel(pix, x0, y0);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
        while (x0 != x1) {
            if (fraction >= 0) {
                y0 += stepy;
                fraction -= dx;                                // same as fraction -= 2*dx
            }
            x0 += stepx;
            fraction += dy;                                    // same as fraction -= 2*dy
            raster.setPixel(pix, x0, y0);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y0 != y1) {
            if (fraction >= 0) {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            raster.setPixel(pix, x0, y0);
        }
    }
}*/