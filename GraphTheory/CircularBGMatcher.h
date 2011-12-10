/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BipartiteGraphMatcher.h"

namespace vpl {

/*!
*/
class CircularBGMatcher : public BipartiteGraphMatcher
{	
	unsigned m_numRows;
	unsigned m_numCols;
	int m_bestOffset;

public:
	CircularBGMatcher()
	{
		m_numRows = 0;
		m_numCols = 0;
		m_bestOffset = 0;
	}

	virtual void Init(unsigned numNodes1, unsigned numNodes2);

	virtual double SolveMinCost(const Matrix& costMat);

	virtual void GetCorrespondences(UIntVector& rowMap, UIntVector& colMap);
};

} // namespace vpl
