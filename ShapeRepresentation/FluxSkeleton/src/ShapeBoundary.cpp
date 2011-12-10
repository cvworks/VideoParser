/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeBoundary.h"
#include "DDSGraph.h"
#include "DDSGraphUtils.h"

using namespace sg;

/*!
	Sets the info of all the spokes ending at each boundary point. 
*/
void ShapeBoundary::setSkeletalPoints(const DDSGraph* pDDSGraph)
{
	const DDSEdgeVect& edges = pDDSGraph->getEdges();
	const SkelBranch* pBranch;
	unsigned int i;

	// Right now, we assume that there is only one curve. In teh future,
	// the boundary info of each skeletal point has to be extended to
	// represent the specific contour curve that they map to.
	ASSERT(m_contourCurves.size() == 1);

	// The skeletal info should not have any data already
	ASSERT(m_skeletalInfo.empty());

	m_skeletalInfo.Initialize(size());

	forall_const_branches(pBranch, edges)
	{
		const BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

		for (i = 0; i < bil.size(); i++)
		{
			m_skeletalInfo.Add(bil[i].first.index, pBranch, i, '1');
			m_skeletalInfo.Add(bil[i].second.index, pBranch, i, '2');
		}
	}
}
