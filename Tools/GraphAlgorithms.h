/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Graph.h"
#include "NodeMap.h"
#include "EdgeMap.h"
#include "Exceptions.h"

#include <utility>

namespace vpl {

//! Pair of nodes
typedef std::pair<graph::node, graph::node> NodePair;

//! Pair of edges
typedef std::pair<graph::edge, graph::edge> EdgePair;

struct TreeAttributes
{
	struct NodeAttributes
	{
		graph::node root; //!< The root of the tree that the node belongs to
		int level;        //!< The level in the tree of the node
		int childOrder;   //!< The order among its siblings

		NodeAttributes()
		{
			root = nil;
		}

		void set(graph::node r, int l, int co)
		{
			root = r;
			level = l;
			childOrder = co;
		}
	};

	graph::node root; //!< The root of the tree
	int width;        //!< The maximum width of any level in the tree
	int height;       //!< The length of the longest path from the root to  a leaf node

	//! Each tree shares the map of node attributes
	std::shared_ptr<NodeMap<NodeAttributes>> ptrNodeAttr;

	TreeAttributes()
	{
		height = 0;
		width = 0;
	}

	void operator=(const TreeAttributes& rhs)
	{
		root   = rhs.root;
		width  = rhs.width;
		height = rhs.height;
		ptrNodeAttr = rhs.ptrNodeAttr;
	}

	bool visited(graph::node v) const
	{
		return (*ptrNodeAttr)[v].root != nil;
	}

	const NodeAttributes& attributes(graph::node v) const
	{
		return (*ptrNodeAttr)[v];
	}

	void set_tree_attributes(graph::node r, int w, int h)
	{
		root   = r;
		width  = w;
		height = h;
	}

	void set_node_attributes(graph::node v, graph::node root, 
		int level, int childOrder)
	{
		return (*ptrNodeAttr)[v].set(root, level, childOrder);
	}
};

//! Returns the node shared by both edges
inline graph::node SharedNode(graph::edge e1, graph::edge e2)
{
	if (source(e1) == source(e2) || source(e1) == target(e2))
		return source(e1);
	else if (target(e1) == source(e2) || target(e1) == target(e2))
		return target(e1);
	else
		return nil;
}

//! Returns the node that is not shared by both edges
inline graph::node NotSharedNode(graph::edge e1, graph::edge e2)
{
	if (source(e1) != source(e2) && source(e1) != target(e2))
		return source(e1);
	else if (target(e1) != source(e2) && target(e1) != target(e2))
		return target(e1);
	else
		return nil;
}

std::list<TreeAttributes> compute_forest_info(const graph& G, 
    const std::list<graph::node>& roots = std::list<graph::node>());

void compute_tree_attributes(const graph& G, graph::edge srcEdge, 
	graph::node subRootNode, TreeAttributes* rootAttr);

} // namespace vpl


