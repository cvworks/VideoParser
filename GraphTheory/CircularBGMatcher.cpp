/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CircularBGMatcher.h"

using namespace vpl;

/*!
	Note: init must be called before any call to SolveMinCost().

	This is because the maxNumEdges changes after a call to
	SolveMinCost(). Not calling Init() leads to a reallocation of edges
	due to lack of space.
*/
void CircularBGMatcher::Init(unsigned numNodes1, unsigned numNodes2)
{	
	m_numRows = numNodes1;
	m_numCols = numNodes2;
	m_bestOffset = 0;
}

double CircularBGMatcher::SolveMinCost(const Matrix& costMat)
{
	ASSERT(m_numRows == costMat.rows() && m_numCols == costMat.cols());

	// Solve min circular histo distance
	unsigned i;
	const unsigned R = m_numCols - 1;
	
	// Init matchCost by summing over diagonal
	double costF = 0, costB = 0;

	for (i = 0; i < m_numRows; ++i)
	{
		costF += costMat(i, i);
		costB += costMat(i, R - i);
	}

	double matchCost = MIN(costF, costB);
	
	unsigned j, offset;
	
	m_bestOffset = 0;

	// See if we can improve min cost by shifting histo2
	// both foreward and backwards (it, with histo2 reversed)
	for (offset = 1; offset < m_numCols; offset++)
	{
		costF = 0;
		costB = 0;

		for (i = 0; i < m_numRows; ++i)
		{
			j = (i + offset) % m_numCols;

			costF += costMat(i, j);
			costB += costMat(i, R - j);
		}

		if (costF < matchCost)
		{
			matchCost = costF;
			m_bestOffset = offset;
		}

		if (costB < matchCost)
		{
			matchCost = costB;
			m_bestOffset = -int(offset);
		}
	}

	return matchCost;
}

/*! 
	Returns the correspondences found by the SolveMinCost() function.
		
	Stores results in the row and col maps s.t.
	row2colMap(r) = c means that col c is assigned to row r
	col2rowMap(c) = r means that row r is assigned to column c
*/
void CircularBGMatcher::GetCorrespondences(UIntVector& row2colMap, UIntVector& col2rowMap)
{
	unsigned i, j;

	// Init vectors to "known" invalid values
	row2colMap.set_size(m_numRows);
	row2colMap.fill(UINT_MAX);
		
	col2rowMap.set_size(m_numCols);
	col2rowMap.fill(UINT_MAX);

	for (i = 0; i < m_numRows; i++)
	{
		j = (i + m_bestOffset) % m_numCols;
		
		row2colMap[i] = j;
		col2rowMap[j] = i;
	}
}

