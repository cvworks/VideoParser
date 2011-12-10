/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BipartiteGraphMatcher.h"

class PerfectMatching;

namespace vpl {

/*!
*/
class Blossom5BGMatcher : public BipartiteGraphMatcher
{
	PerfectMatching* m_pMatcher;
	
	unsigned m_numNodes1;
	unsigned m_numNodes2;

	bool CheckMatch();

public:
	Blossom5BGMatcher()
	{
		m_pMatcher = NULL;
		m_numNodes1 = 0;
		m_numNodes2 = 0;
	}

	virtual ~Blossom5BGMatcher();

	virtual void Init(unsigned numNodes1, unsigned numNodes2);

	virtual double SolveMinCost(const Matrix& costMat);

	virtual void GetCorrespondences(UIntVector& rowMap, UIntVector& colMap);
};

} // namespace vpl
