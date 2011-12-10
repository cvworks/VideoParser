/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <list>
#include <vector>
#include "BasicUtils.h"
#include "GraphUtils.h"
#include "Serialization.h"
#include "SimpleList.h"

namespace vpl {

class NodeArray;
class EdgeArray;

/*!
	Generic graph class. The nodes and edges of this graph class have 
	no meaningful attributes. However, they do have a "visited" field
	that is used by some graph algorithms. Such variables
	can be modified even when the graph is const.
*/
class graph
{
public:
	enum TYPE {UNDIRECTED, DIRECTED};

	class NodeData;

	typedef SimpleList<NodeData*> NodeDataList;
	typedef NodeDataList::iterator NodeDataIt;
	typedef NodeDataList::const_iterator NodeDataConstIt;

	typedef NodeDataList::CellPtr node;

	typedef std::list<node> NodeList;
	typedef NodeList::iterator NodeIt;
	typedef NodeList::const_iterator ConstNodeIt;

	class EdgeData;

	typedef SimpleList<EdgeData*> EdgeDataList;
	typedef EdgeDataList::iterator EdgeDataIt;
	typedef EdgeDataList::const_iterator EdgeDataConstIt;

	typedef EdgeDataList::CellPtr edge;

	typedef std::list<edge> EdgeList;
	typedef EdgeList::iterator EdgeIt;
	typedef EdgeList::const_iterator ConstEdgeIt;

	class NodeData
	{
		friend class graph;
	protected:
		graph* m_pContainer;

		EdgeList m_inEdges; //!< Used only for directed graphs
		EdgeList m_outEdges; //!< Used for ALL edges in undirected graphs

		bool m_visited;
		unsigned m_index;

	protected:
		EdgeList& in_edges()  { return m_inEdges; }
		EdgeList& out_edges() { return m_outEdges; }

	public:
		NodeData(graph* pContainer) 
		{ 
			m_pContainer = pContainer;
		}

		graph* ContainerGraph() const
		{
			return m_pContainer;
		}

		virtual ~NodeData()
		{
			// nothing to do, but virtual 
			// destructor must be declared
		}
		
		const EdgeList& in_edges() const  { return m_inEdges; }
		const EdgeList& out_edges() const { return m_outEdges; }
	};

	class EdgeData
	{
		friend class graph;

		//friend node source(edge e);
		//friend node target(edge e);

	protected:
		node m_source;
		node m_target;

		bool m_visited;
		unsigned m_index;

	public:
		EdgeData(node u, node v) 
		{ 
			m_source = u;
			m_target = v; 
		}

		node source() const { return m_source; }
		node target() const { return m_target; }

		//! returns target() if v = source() and source() otherwise. 
		node opposite(node v) const
		{
			return (v == m_source) ? m_target : m_source;
		}

		virtual ~EdgeData()
		{
			// nothing to do, but virtual 
			// destructor must be declared
		}
	};

	friend const graph* graph_of(node v);
	friend node source(edge e);
	friend node target(edge e);
	friend node opposite(node v, edge e);

protected:
	bool m_directed;
	NodeDataList m_nodes;
	EdgeDataList m_edges;

public:
	static const NodeData& DerefData(node v) { return *NodeDataList::GetRefData(v); }
	static const EdgeData& DerefData(edge e) { return *EdgeDataList::GetRefData(e); }

protected:
	NodeData* GetNodeData(node v) { return m_nodes.GetData(v); }
	const NodeData* GetNodeData(node v) const { return m_nodes.GetData(v); }

	EdgeData* GetEdgeData(edge e) { return m_edges.GetData(e); }
	const EdgeData* GetEdgeData(edge e) const { return m_edges.GetData(e); }

	/*!
		If the graph is directed, it adds edge 'e' to the list of
		out-edges of node 'u' and to the list of in-edges of node 'v'. 
		If the graph is undirected,	the edge is added to the out-edges 
		lists of both 'u' and 'v'.
	*/
	void AddEdgeToNodeData(edge e, node u, node v)
	{
		GetNodeData(u)->m_outEdges.push_back(e);

		if (m_directed)
			GetNodeData(v)->m_inEdges.push_back(e);
		else
			GetNodeData(v)->m_outEdges.push_back(e);
	}

	/*!
		Sets the "visited" property of a node.

		By doing some "ugly" casting, we can let a const function change the 
		visited field of nodes, which simplifies many things.
	*/
	void SetVisited(node v, bool val) const
	{
		const_cast<NodeData*>(GetNodeData(v))->m_visited = val;
	}

	/*!
		Sets the "visited" property of an edge.

		By doing some "ugly" casting, we can let a const function change the 
		visited field of edges, which simplifies many things.
	*/
	void SetVisited(edge e, bool val) const
	{
		const_cast<EdgeData*>(GetEdgeData(e))->m_visited = val;
	}

	bool Visited(node v) const { return GetNodeData(v)->m_visited; }

	bool Visited(edge e) const { return GetEdgeData(e)->m_visited; }

	void UnvisitAllNodes() const
	{
		node v;

		forall_nodes(v, *this)
			SetVisited(v, false);
	}

	void UnvisitAllEdges() const
	{
		edge e;

		forall_edges(e, *this)
			SetVisited(e, false);
	}

	void dfs_node_list_helper(NodeList& nl, node root) const;

public:
	int number_of_nodes() const { return m_nodes.size(); }
	int number_of_edges() const { return m_edges.size(); }

	bool empty() const { return m_nodes.empty(); }

	node first_node() const { return m_nodes.GetBeginPtr(); }
	node last_node()  const { return m_nodes.GetEndPtr(); }

	node succ_node(node v) const 
	{ 
		return m_nodes.Successor(v);
	}

	node pred_node(node v) const 
	{ 
		return m_nodes.Predecessor(v);
	}

	edge first_edge() const { return m_edges.GetBeginPtr(); }
	edge last_edge()  const { return m_edges.GetEndPtr(); }

	edge succ_edge(edge e) const 
	{ 
		return m_edges.Successor(e);
	}

	edge pred_edge(edge e) const 
	{ 
		return m_edges.Predecessor(e);
	}

	EdgeList& in_edges(node v)
	{
		return GetNodeData(v)->m_inEdges;
	}

	edge first_in_edge(node v)
	{
		return in_edges(v).front();
	}

	const EdgeList& in_edges(node v) const
	{
		return GetNodeData(v)->m_inEdges;
	}

	//! Returns the i'th edge if it exists and a nil edge otherwise
	edge in_edge(node v, unsigned int i) const
	{
		const EdgeList& edges = in_edges(v);
		EdgeList::const_iterator it;
		int n;

		for (n = 0, it = edges.begin(); it != edges.end(); ++n, ++it)
			if (n == i)
				return *it;

		return NULL;
	}

	bool has_edge(node u, node v) const
	{
		edge e;

		if (m_directed)
		{
			forall_adj_edges(e, u)
				if (target(e) == v)
					return true;
		}
		else
		{
			if (outdeg(u) <= outdeg(v))
			{
				forall_adj_edges(e, u)
					if (source(e) == v || target(e) == v)
						return true;
			}
			else
			{
				forall_adj_edges(e, v)
					if (source(e) == u || target(e) == u)
						return true;
			}
		}

		return false;
	}

	graph(TYPE type)
	{
		m_directed = (type == DIRECTED);
	}

	graph(const graph& g)
	{
		operator=(g);
	}

	void operator=(const graph& rhs)
	{
		// @TODO this only works with "empty" 
		// graphs due to pointers
		ASSERT(rhs.empty());

		m_directed = rhs.m_directed;

		//m_nodes = rhs.m_nodes;
		//m_edges = rhs.m_edges;
	}

	void clear()
	{
		// Call the virtual destructors of all EdgeData objects
		{
			edge e;

			forall_edges(e, *this)
				delete GetEdgeData(e);

			m_edges.clear();
		}

		// Call the virtual destructors of all NodeData objects
		{
			node v;

			forall_nodes(v, *this)
				delete GetNodeData(v);

			m_nodes.clear();
		}
	}

	void del_edge(edge e)
	{
		delete GetEdgeData(e);
		// @TODO find the edge in the source and target lists
		// and delete it from there too!!!!
		m_edges.erase(e);
	}

	void del_node(node v)
	{
		edge e;

		forall_adj_edges(e, v)
			del_edge(e);

		delete GetNodeData(v);
		m_nodes.erase(v);
	}

	~graph()
	{
		clear();
	}

	//! Returns the number of edges adjacent to node v (|adj_edges(v)|).
	int outdeg(node v) const { return DerefData(v).m_outEdges.size(); }

	/*! 
		Returns the number of edges ending at v (|in_edges(v)|) if G 
	    is directed and zero if G is undirected). 
	*/
	int indeg(node v) const { return DerefData(v).m_inEdges.size(); }

	// Returns outdeg(v) + indeg(v). 
	int degree(node v) const { return indeg(v) + outdeg(v); }

	//! Returns true iff G is directed.
	bool is_directed() const { return m_directed; }
	
	//!	Returns true iff G is undirected. 
	bool is_undirected() const { return !m_directed; }

	void dfs_node_list(NodeList& nl) const;

	void dfs_node_list(NodeList& nl, node root) const;

	//! Sets zero-based unique and consecutive indices for all nodes
	void set_node_indices()
	{
		node v;
		int idx = 0;

		forall_nodes(v, *this)
		{
			GetNodeData(v)->m_index = idx++;
		}
	}

	//! Sets zero-based unique and consecutive indices for all edges
	void set_edge_indices()
	{
		edge e;
		int idx = 0;

		forall_edges(e, *this)
		{
			GetEdgeData(e)->m_index = idx++;
		}
	}

	//! Sets zero-based unique and consecutive indices for all nodes and edges
	void set_indices()
	{
		set_node_indices();
		set_edge_indices();
	}

	void set_node_indices(const NodeArray& na);

	void set_edge_indices(const EdgeArray& ea);

	//! Returns the index of the node set by a prior call to set_node_indices()
	unsigned index(node v) const { return GetNodeData(v)->m_index; }

	//! Returns the index of the edge set by a prior call to set_edge_indices()
	unsigned index(edge e) const { return GetEdgeData(e)->m_index; }
};

/*inline graph* graph_of(graph::node v)
{
	return (**v)->ContainerGraph();
}*/

inline const graph* graph_of(graph::node v)
{
	return graph::DerefData(v).ContainerGraph();
}

inline graph::node source(graph::edge e)
{
	return graph::DerefData(e).source();
}

inline graph::node target(graph::edge e)
{
	return graph::DerefData(e).target();
}

//! Returns target(e) if v = source(e) and source(e) otherwise. 
inline graph::node opposite(graph::node v, graph::edge e)
{
	return graph::DerefData(e).opposite(v);
}

/*!
	Array of the nodes in the graph index according to their 
	index(node) function. 

	That is:

	NodeArray a(G);

	a[i] = v; // => index(v) == i

	It adds the nodes in the order that set_node_indices() enumerates
	the nodes, so indices and node arrays can be used as bi-directed
	maps.
*/
class NodeArray : public std::vector<graph::node>
{
public:
	NodeArray() { }
	
	NodeArray(const graph& G)
	{
		init(G);
	}

	void init(const graph& G)
	{
		resize(G.number_of_nodes());

		graph::node v;
		unsigned i = 0;

		forall_nodes(v, G)
		{
			operator[](i++) = v;
		}
	}
};

/*!
	Array of the edges in the graph index according to their 
	index(edge) function. 

	That is:

	NodeArray a(G);

	a[i] = e; // => index(e) == i

	It adds the edges in the order that set_edge_indices() numerates
	the edges, so indices and node arrays can be used as bi-directed
	maps.
*/
class EdgeArray : public std::vector<graph::edge>
{
public:
	EdgeArray() { }
	
	EdgeArray(const graph& G)
	{
		init(G);
	}

	void init(const graph& G)
	{
		resize(G.number_of_edges());

		graph::edge e;
		unsigned i = 0;

		forall_edges(e, G)
		{
			operator[](i++) = e;
		}
	}
};

} // namespace vpl

//! Declare the (De)serialization of node references
DECLARE_BASIC_SERIALIZATION(vpl::graph::node)

//! Declare the (De)serialization of edge references
DECLARE_BASIC_SERIALIZATION(vpl::graph::edge)
