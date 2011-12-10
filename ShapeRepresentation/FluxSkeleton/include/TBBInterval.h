/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __TBB_INTERVAL_H__
#define __TBB_INTERVAL_H__

#include <list>
#include "DDSGraph.h"
#include "BoundaryInterval.h"

namespace sg {

/*!
	@brief TwoBranchBndryInterval class. Defines an interval of the boundary 
	curve associated with two adjacent branches in a DDSGraph.
*/
struct TBBInterval
{
	DDSEdge *pBranch0, *pBranch1;
	BoundaryInfo extPt0, intPt0, extPt1, intPt1;
	BoundaryInterval firstSideInt, secondSideInt;

	TBBInterval() { }

	TBBInterval(DDSGraph* pG, DDSEdge* p0, DDSEdge* p1) 
	{ 
		pBranch0 = p0;
		pBranch1 = p1;

		SetLimits((int) pG->getShape()->size());
	}

	void SetLimits(int bndryLength);
	int Side(const DDSEdge* pBranch, const DDSNode* pJoint) const;
	bool IsValid() const;
};

/*!
	@brief List of TwoBranchBndryInterval's.
*/
class TBBIntervalList : public std::list<TBBInterval>
{
	int m_nBndrySize;

public:
	typedef std::list<TBBInterval>::iterator iterator;

	TBBIntervalList(DDSGraph* pGraph)
	{
		m_nBndrySize = (int) pGraph->getShape()->size();
		ASSERT(m_nBndrySize > 0);
	}

	void FindAllIntervals(DDSEdgeVect::iterator iBranch, 
		DDSEdgeVect::iterator iEnd);
};

} // namespace sg

#endif // __TBB_INTERVAL_H__
