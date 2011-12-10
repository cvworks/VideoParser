/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BipartiteGraphMatcher.h"
#include <lemon/smart_graph.h>
#include <lemon/concepts/graph.h>
#include <lemon/concepts/maps.h>

namespace vpl {

/*!
*/
class LemonBGMatcher : public BipartiteGraphMatcher
{
	typedef lemon::SmartGraph Graph;
	typedef Graph::Node node;
	typedef Graph::Edge edge;
	typedef Graph::EdgeMap<double> Weights;
	typedef Graph::NodeMap<unsigned> Indices;

	Graph m_bg;
	Weights* m_pWeightMap;
	Indices* m_pIndexMap;
	bool m_hasNegatedWeights;
	
	std::vector<node> m_nodes1;
	std::vector<node> m_nodes2;
	STLMatrix<edge> m_edges;

	static const double DOUBLE_MAX;

public:
	LemonBGMatcher()
	{
		m_pWeightMap = NULL;
		m_pIndexMap = NULL;
		m_hasNegatedWeights = false;
	}

	virtual ~LemonBGMatcher();

	void Init(unsigned numNodes1, unsigned numNodes2);

	void SetEdgeCosts(const Matrix& costMat);

	virtual double Solve(const Matrix& costMat, UIntVector* pRowMap, 
		UIntVector* pColMap)
	{
		SetEdgeCosts(costMat);

		return SolveMaxWeightedMatching(pRowMap, pColMap);
	}

	double SolveMaxWeightedMatching(UIntVector* pRowMap, UIntVector* pColMap);

	bool HasNegatedWeights() const
	{
		return m_hasNegatedWeights;
	}
};

} // namespace vpl
