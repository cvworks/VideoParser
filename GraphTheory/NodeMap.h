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

	Note: to acess elements by index use the at() function.
*/
template <typename T> class NodeMap : public std::vector<T>
{
protected:
	const graph* m_pGraph;

public:
	typedef std::vector<T> BASE_CLASS;
	typedef typename BASE_CLASS::reference reference;
	typedef typename BASE_CLASS::const_reference const_reference;

public:
	NodeMap() 
	{
		m_pGraph = NULL;
	}

	NodeMap(const graph& G) : BASE_CLASS(G.number_of_nodes())
	{
		m_pGraph = &G;
	}

	NodeMap(const graph& G, T val) : BASE_CLASS(G.number_of_nodes(), val)
	{
		m_pGraph = &G;
	}

	NodeMap(const NodeMap& rhs) : BASE_CLASS(rhs)
	{
		m_pGraph = rhs.m_pGraph;
	}

	void operator=(const NodeMap& rhs)
	{
		BASE_CLASS::operator=(rhs);
		m_pGraph = rhs.m_pGraph;
	}

	void Init(const graph& G) 
	{
		//ASSERT(!m_pGraph && empty());

		m_pGraph = &G;

		resize(m_pGraph->number_of_nodes());
	}

	void Init(const graph& G, T value) 
	{
		//ASSERT(!m_pGraph && empty());

		m_pGraph = &G;

		resize(m_pGraph->number_of_nodes(), value);
	}

	void Clear()
	{
		BASE_CLASS::clear();

		m_pGraph = NULL;
	}

	const_reference operator[](graph::node v) const
	{
		return BASE_CLASS::operator[](m_pGraph->index(v));
	}

	reference operator[](graph::node v)
	{
		return BASE_CLASS::operator[](m_pGraph->index(v));
	}
};

} // namespace vpl

