/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <SkeletalInfo.h>
#include <DDSGraph.h>
#include <DDSGraphUtils.h>

using namespace sg;

void BranchPointIndex::GetSpokeIndices(unsigned int* pIdx1, unsigned int* pIdx2) const
{
	const BoundaryInfo& bi = pBranch->boundaryInfo(index);

	*pIdx1 = bi.first.index;
	*pIdx2 = bi.second.index;
}

const Point& BranchPointIndex::GetSkelPointCoord() const
{
	return pBranch->fluxPoint(index).p;
}

/*!
	Finds symmetric points of the boundary point represented
	by this map wrt each of its axis of symmetry (ie, branches)

	The map is cleared before the points are added.
*/
void SpokesMap::FindSymmetricPoints(SymPtListMap* pSymMap) const
{
	const_iterator mapIt;
	SpokeInfoList::const_iterator spokeIt;
	const DDSEdge* pBranch;
	SymmetricPoint sp;
	char otherSide;

	pSymMap->clear();

	for (mapIt = begin(); mapIt != end(); ++mapIt)
	{
		pBranch = mapIt->first;
		const SpokeInfoList& sil = mapIt->second;

		SymPtList& spl = (*pSymMap)[pBranch];

		for (spokeIt = sil.begin(); spokeIt != sil.end(); ++spokeIt)
		{
			otherSide = BoundaryInfo::FlipSide(spokeIt->side);
			
			sp.index = pBranch->bndryPtIndex(spokeIt->skelPtIndex, otherSide);
			sp.medialPtIdx = spokeIt->skelPtIndex;
			sp.radius = pBranch->radius(spokeIt->skelPtIndex);

			spl.push_back(sp);
		}
	}
}

/*!
	Finds the origin of the spoke with maximum length. That is, it returns the
	x-y coordinates of the skeletal point with greatest radius that has a 
	spoke (on ANY branch) that terminates at the boundary point represented 
	by this map.
*/
bool SpokesMap::FindMaxLengthSpokeOrigin(BranchPointIndex* pBPI) const
{
	if (empty())
		return false;

	const_iterator mapIt;
	SpokeInfoList::const_iterator spokeIt;
	const DDSEdge* pBranch;
	double r, maxLen = -1;

	// Iterate over the mapping from skeletal branches to lists of spokes
	for (mapIt = begin(); mapIt != end(); ++mapIt)
	{
		pBranch = mapIt->first;

		//std::cerr << PRN_BRANCH_PTR(pBranch) << std::endl;

		const SpokeInfoList& sil = mapIt->second;

		// The lengh of a spoke is simply the radius of its "skeletal" endpoint
		for (spokeIt = sil.begin(); spokeIt != sil.end(); ++spokeIt)
		{
			r = pBranch->radius(spokeIt->skelPtIndex); 

			if (r > maxLen || (r == maxLen && 
				pBranch->spokeEndpointDistance(spokeIt->skelPtIndex) > 
				pBPI->pBranch->spokeEndpointDistance(pBPI->index)))
			{
				maxLen = r;
				pBPI->pBranch = pBranch;
				pBPI->index = spokeIt->skelPtIndex;
			}
		}
	}

	ASSERT(maxLen >= 0);

	return maxLen >= 0;
}

