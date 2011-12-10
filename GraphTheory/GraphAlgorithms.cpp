/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "GraphAlgorithms.h"

using namespace vpl;

void vpl::compute_tree_attributes(const graph& G, graph::edge srcEdge, 
	graph::node subRootNode, TreeAttributes* treeAttr)
{
	TreeAttributes subTreeAttr;
	graph::edge e;
	graph::node v;

	int cumChildWidth = 0;
	int maxChildHeight = 0;
	
	// Copy info from parent to child attrs
	subTreeAttr.root = treeAttr->root; // root of the tree, not sub-root of sub-tree
	subTreeAttr.ptrNodeAttr = treeAttr->ptrNodeAttr; // share the node attributes

	// Compute the level of this subtree
	const int level = (srcEdge == nil) ? 0 
		: treeAttr->attributes(opposite(subRootNode, srcEdge)).level + 1;

	int childOrder = 0;

	// Process all child nodes
	forall_adj_edges(e, subRootNode)
	{
		// Check that the edge is not the src edge,
		// which happens with undirected graphs
		if (e == srcEdge)
			continue;

		v = opposite(subRootNode, e);

		if (!treeAttr->visited(v))
		{
			// Set the attributes of each child "sub-root" node
			treeAttr->set_node_attributes(v, treeAttr->root, level, childOrder++);

			// Compute attribute of the child sub-tree
			compute_tree_attributes(G, e, v, &subTreeAttr);

			// Accumulate info from all child nodes
			cumChildWidth += subTreeAttr.width;

			if (maxChildHeight < subTreeAttr.height)
				maxChildHeight = subTreeAttr.height;
		}
	}

	// Combine info from child sub-trees with the attributes of the tree
	treeAttr->height = maxChildHeight + 1;

	if (cumChildWidth >= G.outdeg(subRootNode))
	{
		treeAttr->width = cumChildWidth;
	}
	else
	{
		treeAttr->width = G.outdeg(subRootNode);

		// In an undirected graphs, the outdeg includes
		// the src edge, so we have to account for it
		if (G.is_undirected() && srcEdge != nil)
			treeAttr->width--;
	}

	// If there are no child nodes, the width is 1
	if (treeAttr->width == 0)
		treeAttr->width = 1;
}

/*!
	Computes attribues of each tree in a forest.

	The graph G can be directed or undirected. If it is undirected,
	and roots is the empty list, then arbitrary roots are chosen 
	to form a forest.

	If the graph is directed and roots is the empty list, each
	node with indeg equal to zero is chosen as a root.
*/
std::list<TreeAttributes> vpl::compute_forest_info(const graph& G, 
	const std::list<graph::node>& roots)
{
	std::list<TreeAttributes> forest;
	TreeAttributes treeAttr;
	graph::node v;
	
	treeAttr.ptrNodeAttr.reset(new NodeMap<TreeAttributes::NodeAttributes>(G));
	
	if (roots.empty())
	{
		forall_nodes(v, G)
		{
			// This shoud work for both directed and undirected graphs.
			// If directed, the roots have indeg 0 and can't be visited already.
			// If undirected, all nodes have indeg 0, but some will be visited.
			if (G.indeg(v) == 0 && !treeAttr.visited(v))
			{
				treeAttr.set_tree_attributes(v, 1, 1);

				treeAttr.set_node_attributes(v, v, 0, 0);

				compute_tree_attributes(G, nil, v, &treeAttr);
			
				forest.push_back(treeAttr);
			}
		}
	}
	else
	{
		for (auto it = roots.begin(); it != roots.end(); ++it)
		{
			v = *it;

			ASSERT(!treeAttr.visited(v));

			treeAttr.set_tree_attributes(v, 1, 1);

			treeAttr.set_node_attributes(v, v, 0, 0);

			compute_tree_attributes(G, nil, v, &treeAttr);
			
			forest.push_back(treeAttr);
		}
	}

	return forest;
}

