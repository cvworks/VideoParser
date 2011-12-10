/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include "Graph.h"
#include "Exceptions.h"

using namespace vpl;

typedef graph::node node;
typedef graph::edge edge;

typedef graph::node node;
typedef graph::edge edge;

/*!
	Sets the indices of each node using a node array. The given
	node array should represent a one to one mapping between 
	indices and the nodes in the graph.
*/
void graph::set_node_indices(const NodeArray& na)
{
	UnvisitAllNodes();

	NodeData* p;

	for (unsigned i = 0; i < na.size(); ++i)
	{
		p = GetNodeData(na[i]);

		if (p->m_visited)
			THROW_BASIC_EXCEPTION("Duplicated node in NodeArray");

		p->m_visited = true;
		p->m_index = i;
	}

	node v;

	forall_nodes(v, *this)
	{
		if (!Visited(v))
			THROW_BASIC_EXCEPTION("Missing node in NodeArray");
	}
}

/*!
	Sets the indices of each edge using a node array. The given
	edge array should represent a one to one mapping between 
	indices and the edges in the graph.
*/
void graph::set_edge_indices(const EdgeArray& ea)
{
	UnvisitAllEdges();

	EdgeData* p;

	for (unsigned i = 0; i < ea.size(); ++i)
	{
		p = GetEdgeData(ea[i]);

		if (p->m_visited)
			THROW_BASIC_EXCEPTION("Duplicated edge in EdgeArray");

		p->m_visited = true;
		p->m_index = i;
	}

	edge e;

	forall_edges(e, *this)
	{
		if (!Visited(e))
			THROW_BASIC_EXCEPTION("Missing edge in EdgeArray");
	}
}

/*!
	Returns a list of all nodes collected in a depth-first search
	manner.
*/
void graph::dfs_node_list(NodeList& nl) const
{
	node v;

	UnvisitAllNodes();

	if (m_directed)
	{
		forall_nodes(v, *this)
		{
			if (indeg(v) == 0)
				dfs_node_list_helper(nl, v);
		}
	}
	else
	{
		forall_nodes(v, *this)
		{
			if (!Visited(v))
				dfs_node_list_helper(nl, v);
		}
	}
}

void graph::dfs_node_list(NodeList& nl, node root) const
{
	UnvisitAllNodes();

	dfs_node_list_helper(nl, root);
}

/*! 
	This function assumes that the visited attribute of all nodes
	has been initialized. Ie, it must be preceeded by UnvisitAllNodes()
*/
void graph::dfs_node_list_helper(NodeList& nl, node root) const
{
	SetVisited(root, true);

	nl.push_back(root);

	node v;

	forall_adj_nodes(v, root)
	{
		if (!Visited(v))
			dfs_node_list(nl, v);
	}
}

/*void f()
{
	AttributedGraph<int, int> g;

	node u = g.new_node(5);
	node v = g.new_node(7);

	edge e = g.new_edge(u, v, 10);

	forall_nodes(v, g)
	{
		g.inf(v);
	}
}*/
