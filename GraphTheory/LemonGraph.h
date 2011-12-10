/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <lemon/smart_graph.h>
#include <lemon/concepts/graph.h>
#include <lemon/concepts/maps.h>

namespace vpl {

/*!
	Graph implementation based on the LEMON library.
*/
template <typename N_TYPE = int, typename E_TYPE = int, typename G_TYPE = lemon::SmartGraph> 
class LemonGraph : public G_TYPE
{
public:
	//typedef G_TYPE Graph;
	typedef typename G_TYPE::Node node;
	typedef typename G_TYPE::Edge edge;
	typedef typename G_TYPE::EdgeMap<E_TYPE> EdgeMapT;
	typedef typename G_TYPE::NodeMap<N_TYPE> NodeMapT;

	enum TYPE {UNDIRECTED, DIRECTED};

protected:
	NodeMapT m_nodeAttributes;
	EdgeMapT m_edgeAttributes;

public:
	LemonGraph(unsigned approxNumNodes = 0, unsigned approxNumEdges = 0)
		: m_nodeAttributes(*this), m_edgeAttributes(*this)
	{
		reserve(approxNumNodes, approxNumEdges);
	}

	void reserve(unsigned approxNumNodes, unsigned approxNumEdges)
	{
		if (approxNumNodes > 0)
			reserveNode(approxNumNodes);

		if (approxNumEdges > 0)
			reserveEdge(approxNumEdges);
	}

	node addNode(const N_TYPE& att)
	{
		node v = G_TYPE::addNode();
		
		m_nodeAttributes[v] = att;

		return v;
	}

	edge addEdge(node u, node v, const E_TYPE& att)
	{
		edge e = G_TYPE::addEdge(u, v);

		m_edgeAttributes[e] = att;

		return e;
	}

	std::vector<node> addNodes(unsigned numNodes, const N_TYPE& att)
	{
		std::vector<node> nodes(numNodes);

		for (unsigned i = 0; i < numNodes; i++)
			nodes[i] = addNode(att);

		return nodes;
	}

	/*! 
		Finds the connected components of an undirected graph. 

		@param compMap it is an output parameter with the mapping from
		nodes to components.
	*/
	int connectedComponents(typename G_TYPE::NodeMap<int>& compMap);
};

} // namespace vpl
