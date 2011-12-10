/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <functional>
#include "Graph.h"
#include "NodeMatch.h"
#include "STLUtils.h"

#include "HungarianAlgorithm.h"

#define UNKNOWN_GRAPH_MATCH_COST -1.0

namespace vpl {

/*!
	Finds one-to-one correspondeces among the nodes of
	each graph and computes a graph similarity value.
*/
class GraphMatcher
{
protected:
	typedef graph::node node;
	typedef graph::edge edge;

	DoubleMatrix m_nodeSimMat; //!< Node similarity matrix 

	NodeArray m_nodes1; //!< Row nodes
	NodeArray m_nodes2; //!< Column rows

	const graph* m_pG1;
	const graph* m_pG2;

	NodeMap<double> m_nodeCost1;
	NodeMap<double> m_nodeCost2;

protected:

	GraphMatcher()
	{
		m_pG1 = NULL;
		m_pG2 = NULL;
	}

	void Init(const graph& g1, const graph& g2)
	{
		m_pG1 = &g1;
		m_pG2 = &g2;

		m_nodes1.init(g1);
		m_nodes2.init(g2);
	}

	template <class NodeCompFunc>
	void ComputeNodeCostArrays(NodeCompFunc comp)
	{
		m_nodeCost1.Init(*m_pG1, -1);
		
		for (auto it = m_nodes1.begin(); it != m_nodes1.end(); ++it)
			m_nodeCost1[*it] = comp(*it, nil);

		m_nodeCost2.Init(*m_pG2, -1);

		for (auto it = m_nodes2.begin(); it != m_nodes2.end(); ++it)
			m_nodeCost2[*it] = comp(nil, *it);
	}

	template <class NodeCompFunc>
	void ComputeNodeSimilarityMatrix(NodeCompFunc comp)
	{
		m_nodeSimMat.set_size(m_nodes1.size(), m_nodes2.size());

		for (unsigned i = 0; i < m_nodeSimMat.rows(); ++i)
		{
			ASSERT(i == m_pG1->index(m_nodes1[i]));

			for (unsigned j = 0; j < m_nodeSimMat.cols(); ++j)
			{	
				ASSERT(j == m_pG2->index(m_nodes2[j]));

				m_nodeSimMat[i][j] = comp(m_nodes1[i], m_nodes2[j]);
			}
		}
	}

	const double& NodeDistance(node v1, node v2) const
	{
		if (v1 == nil)
		{
			ASSERT(v2 != nil);

			return m_nodeCost2[v2];
		}
		else if (v2 == nil)
		{
			ASSERT(v1 != nil);

			return m_nodeCost1[v1];
		}
		else
		{
			return m_nodeSimMat[m_pG1->index(v1)][m_pG2->index(v2)];
		}
	}

	/*!
		Find the node correspondences with minimum cumulative cost.

		If maxCost != UNKNOWN_GRAPH_MATCH_COST, then search for correspondences
		is aborted as soon as the cumulative cost is matchCost. In that case,
		the function returns UNKNOWN_GRAPH_MATCH_COST.
	*/
	virtual double FindNodeCorrespondences(const double& maxCost) = 0;

public:
	//! Required virtual destructor
	virtual ~GraphMatcher() { }

	/*!
		Matches the nodes of two graphs and returns a measure of the similarity between
		the graphs.

		@param NodeCompFunc is a binary function taking a node of g1 and a node of g2,
							and returning a similarity weight. This can either be a 
							pointer to a function or an object whose class overloads operator().
	*/
	template <class NodeCompFunc>
	double Match(const graph& g1, const graph& g2, NodeCompFunc comp,
		double maxCost = UNKNOWN_GRAPH_MATCH_COST)
	{
		Init(g1, g2);

		ComputeNodeSimilarityMatrix(comp);

		ComputeNodeCostArrays(comp);

		return FindNodeCorrespondences(maxCost);
	}

	/*void G1ToG2Map(NodeArray& nm2, DoubleArray& weights) const;

	void G1ToG2Map(NodeArray& nm1, NodeArray& nm2, DoubleArray& weights) const
	{
		nm1 = m_nodes1;

		G1ToG2Map(nm2, weights);
	}

	void GetRowToColMap(UIntArray& indices2, DoubleArray& weights) const
	{
		unsigned i, j;

		indices2.resize(m_row2colMap.size());
		weights.resize(m_row2colMap.size());

		for (i = 0; i < indices2.size(); ++i)
		{
			j = m_row2colMap[i];

			indices2[i] = j;

			if (j < UINT_MAX)
				weights[i] = m_nodeSimMat[i][j];
		}
	}

	void GetColToRowMap(UIntArray& indices1, DoubleArray& weights) const
	{
		unsigned i, j;

		indices1.resize(m_col2rowMap.size());
		weights.resize(m_col2rowMap.size());

		for (j = 0; j < indices1.size(); ++j)
		{
			i = m_col2rowMap[j];

			indices1[j] = i;

			if (i < UINT_MAX)
				weights[j] = m_nodeSimMat[i][j];
		}
	}*/

	/*void GetColToRowMap(UIntArray& indices1, DoubleArray& weights) const
	{
		NodeMatchMap nodeMap(*m_pG1);

		GetNodeMapF2S(nodeMap);

		graph::node v;
		unsigned i, j;

		indices1.resize(m_pG2->number_of_nodes(), UINT_MAX);
		weights.resize(m_pG2->number_of_nodes(), 0);

		forall_nodes(v, *m_pG1)
		{
			const NodeMatch& nm = nodeMap[v];

			i = m_pG1->index(v);

			if (nm.mappedNode != nil)
			{
				j = m_pG2->index(nm.mappedNode);

				indices1[j] = i;
				weights[j] = nm.nodeAttDist;
			}
		}
	}*/

	//! Get the map from node in first graph to nodes in second graph
	virtual void GetNodeMapF2S(NodeMatchMap& nodeMap) const = 0;
};

/*!
	Solves the maximum weight assignment problem by
	creating a bipartite graph with the nodes of
	two graphs.

	The structure of the graph (ie, node adjacecies and edge
	information) is not taken into account to stablish
	the node correspondeces.
*/
class NaiveGraphMatcher : public GraphMatcher
{
	HungarianAlgorithm m_ha;

	UIntVector m_row2colMap; //!< Mapping from row nodes to column nodes
	UIntVector m_col2rowMap; //!< Mapping from column nodes to rows nodes 
public:
	virtual double FindNodeCorrespondences()
	{
		return -m_ha.Solve(-m_nodeSimMat, &m_row2colMap, &m_col2rowMap);
	}
};

} // namespace vpl
