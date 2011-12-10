/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <GraphTheory/Graph.h>
#include <algorithm>

namespace vpl {

/*!
	A path of nodes in a shape cut graph.
*/
struct ShapeCutPath
{
	std::list<graph::node> nodes;
	std::list<graph::edge> edges;
	std::list<graph::edge> interiorEdges;
	graph::edge dummyEdge;

	ShapeCutPath()
	{
		dummyEdge = nil;
	}

	ShapeCutPath(graph::node firstNode) 
		: nodes (1, firstNode)
	{
		dummyEdge = nil;
	}

	ShapeCutPath(graph::node u, graph::node v, graph::edge e) 
		: nodes (1, u)
	{
		nodes.push_back(v);
		edges.push_back(e);
	}

	ShapeCutPath& operator=(const ShapeCutPath& rhs)
	{
		nodes = rhs.nodes;
		edges = rhs.edges;
		interiorEdges = rhs.interiorEdges;
		dummyEdge = rhs.dummyEdge;

		return *this;
	}

	unsigned Size() const
	{
		return nodes.size();
	}

	//! Returns true if the nodes list contains v
	bool Contains(graph::node v) const
	{
		return (std::find(nodes.begin(), nodes.end(), v) 
			!= nodes.end());
	}

	/*! 
		Returns true if the nodes list contains v, but excludes 
		first and last elements in the list.
	*/
	bool ContainsProper(graph::node v) const
	{
		auto it = std::find(nodes.begin(), nodes.end(), v);

		return (it != nodes.begin() && it != nodes.end() &&
			++it != nodes.end());
	}
};

} // namespace vpl
