/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "Blossom5BGMatcher.h"
#include "Blossom5/PerfectMatching.h"

using namespace vpl;

Blossom5BGMatcher::~Blossom5BGMatcher()
{
	delete m_pMatcher;
}

/*!
	Note: init must be called before any call to SolveMinCost().

	This is because the maxNumEdges changes after a call to
	SolveMinCost(). Not calling Init() leads to a reallocation of edges
	due to lack of space.
*/
void Blossom5BGMatcher::Init(unsigned numNodes1, unsigned numNodes2)
{
	delete m_pMatcher;
	
	m_numNodes1 = numNodes1;
	m_numNodes2 = numNodes2;

	m_pMatcher = new PerfectMatching(
		numNodes1 + numNodes2, numNodes1 * numNodes2);

	m_pMatcher->options.verbose = false;
}

double Blossom5BGMatcher::SolveMinCost(const Matrix& costMat)
{
	ASSERT(m_pMatcher);
	ASSERT(costMat.rows() == m_numNodes1 && costMat.cols() == m_numNodes2);

	unsigned i, j;
	std::vector<double> weights(m_numNodes1 * m_numNodes2);

	for (i = 0; i < costMat.rows(); i++)
	{
		for (j = 0; j < costMat.cols(); j++)
		{
			ASSERT(costMat(i, j) >= 0);
			m_pMatcher->AddEdge(i, m_numNodes1 + j, costMat(i, j));
		}
	}


	m_pMatcher->Solve();

	double cost = 0;

	for (i = 0; i < m_numNodes1; i++)
	{
		for (j = 0; j < m_numNodes2; j++)
		{
			if (m_pMatcher->GetSolution(i * m_numNodes2 + j))
				cost += costMat(i, j);
		}
	}

	ASSERT(cost >= 0);

	return cost;
}

/*! 
	Returns the correspondences found by the SolveMinCost() function.
		
	Stores results in the row and col maps s.t.
	row2colMap(r) = c means that col c is assigned to row r
	col2rowMap(c) = r means that row r is assigned to column c
*/
void Blossom5BGMatcher::GetCorrespondences(UIntVector& row2colMap, UIntVector& col2rowMap)
{
	unsigned i, j;

	// Init vectors to "known" invalid values
	row2colMap.set_size(m_numNodes1);
	row2colMap.fill(UINT_MAX);
		
	col2rowMap.set_size(m_numNodes2);
	col2rowMap.fill(UINT_MAX);

	for (i = 0; i < m_numNodes1; i++)
	{
		for (j = 0; j < m_numNodes2; j++)
		{
			if (m_pMatcher->GetSolution(i * m_numNodes2 + j))
			{
				ASSERT(row2colMap[i] == UINT_MAX && col2rowMap[j] == UINT_MAX);

				row2colMap[i] = j;
				col2rowMap[j] = i;
			}
		}
	}
}

bool Blossom5BGMatcher::CheckMatch()
{
	/*int i, j;

	//int node_num = m_numNodes1 + m_numNodes2;
	//int* nodes = new int[node_num];
	//memset(nodes, 0, node_num * sizeof(int));

	for (i = 0; i < m_numNodes1; i++)
	{
		for (j = 0; j < m_numNodes2; j++)
		{
			if (m_pMatcher->GetSolution(i * m_numNodes2 + j))
			{
				++nodes[i];
				++nodes[j];
			}
		}
	}

	for (i=0; i<node_num; i++)
	{
		if (nodes[i] != 1)
		{
			printf("ComputeCost(): degree = %d!\n", nodes[i]);
			exit(1);
		}
	}

	delete [] nodes;*/
	
	return true;
}
