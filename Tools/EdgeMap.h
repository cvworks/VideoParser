/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Graph.h"

namespace vpl {

/*!
	This edge map class attempts to immitate the leda::edge_map.

	It adds the nodes in the order that set_node_indices() enumerates
	the nodes, so indices and node arrays can be used as bi-directed
	maps.
*/
template <typename T> class EdgeMap : public std::vector<T>
{
protected:
	const graph* m_pGraph;

public:
	typedef std::vector<T> BASE_CLASS;
	typedef typename BASE_CLASS::reference reference;
	typedef typename BASE_CLASS::const_reference const_reference;

public:
	EdgeMap() 
	{
		m_pGraph = NULL;
	}

	EdgeMap(const graph& G) : BASE_CLASS(G.number_of_edges())
	{
		m_pGraph = &G;
	}

	EdgeMap(const graph& G, T val) : BASE_CLASS(G.number_of_edges(), val)
	{
		m_pGraph = &G;
	}

	EdgeMap(const EdgeMap& rhs) : BASE_CLASS(rhs)
	{
		m_pGraph = rhs.m_pGraph;
	}

	void operator=(const EdgeMap& rhs)
	{
		BASE_CLASS::operator=(rhs);
		m_pGraph = rhs.m_pGraph;
	}

	void Init(const graph& G) 
	{
		//ASSERT(!m_pGraph && empty());

		m_pGraph = &G;

		resize(m_pGraph->number_of_edges());
	}

	void Init(const graph& G, T value) 
	{
		//ASSERT(!m_pGraph && empty());

		m_pGraph = &G;

		resize(m_pGraph->number_of_edges(), value);
	}

	const_reference operator[](graph::edge e) const
	{
		return BASE_CLASS::operator[](m_pGraph->index(e));
	}

	reference operator[](graph::edge e)
	{
		return BASE_CLASS::operator[](m_pGraph->index(e));
	}
};

} // namespace vpl

