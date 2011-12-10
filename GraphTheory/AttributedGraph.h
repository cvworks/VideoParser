/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Graph.h"

namespace vpl {

/*!
	Generic attributed graph class. 

	The main characteristic of this library and its base
	graph class is that:

	a- Can be serialized.
	b- Iterator proceed in the order of the node/edge labels. Note that
	   in LEMON this is not necessarily the case.
*/
template <class V_TYPE, class E_TYPE> 
class AttributedGraph : public graph
{
	// Cannot have any member variables!
public:
	struct AttrNodeData : public NodeData
	{
		V_TYPE attr;

		AttrNodeData(graph* pContainer, const V_TYPE& x = V_TYPE()) 
			: NodeData(pContainer), attr(x) 
		{ }
	};

	struct AttrEdgeData : public EdgeData
	{
		E_TYPE attr;

		AttrEdgeData(node u, node v, const E_TYPE& x = E_TYPE())
			: EdgeData(u, v), attr(x)
		{ }
	};

protected:
	AttrNodeData* GetAttrNodeData(node v)
	{
		return dynamic_cast<AttrNodeData*>(GetNodeData(v));
	}

	const AttrNodeData* GetAttrNodeData(node v) const
	{
		return dynamic_cast<const AttrNodeData*>(GetNodeData(v));
	}

	AttrEdgeData* GetAttrEdgeData(edge e)
	{
		try {
			return dynamic_cast<AttrEdgeData*>(GetEdgeData(e));
		}
		catch(std::bad_cast) {
			ShowError("Can't cast edge\n");
			return NULL;
		}
	}

	const AttrEdgeData* GetAttrEdgeData(edge e) const
	{
		try {
			return dynamic_cast<const AttrEdgeData*>(GetEdgeData(e));
		}
		catch(std::bad_cast) {
			ShowError("Can't cast edge\n");
			return NULL;
		}
	}

public:
	AttributedGraph(TYPE type = DIRECTED) : graph(type)
	{
		
	}

	AttributedGraph(const graph& g) : graph(g)
	{
		
	}

	void operator=(const AttributedGraph& rhs)
	{
		// @TODO this isn'g going to work due to pointers
		graph::operator=(rhs);
	}

	void Serialize(OutputStream& os) const
	{
		// Write global attributes
		::Serialize(os, m_directed);

		// Write all nodes
		node v;

		::Serialize(os, number_of_nodes());

		forall_nodes(v, *this)
		{
			//::Serialize(os, (void*)v);
			v.Serialize(os);
			::Serialize(os, inf(v));
		}

		// Write all nodes
		edge e;

		::Serialize(os, number_of_edges());

		forall_edges(e, *this)
		{
			//::Serialize(os, (void*)source(e));
			//::Serialize(os, (void*)target(e));

			source(e).Serialize(os);
			target(e).Serialize(os);

			::Serialize(os, inf(e));
		}
	}

	void Deserialize(InputStream& is)
	{
		std::map<node, node> nodeMap;

		// Empty all the contents of the graph
		clear();

		// Read global attributes
		::Deserialize(is, m_directed);

		// Read and create all nodes
		{
			node v;
			unsigned numNodes;
			V_TYPE att;

			::Deserialize(is, numNodes);

			for (unsigned i = 0; i < numNodes; ++i)
			{
				//::Deserialize(is, v);
				v.Deserialize(is);
				::Deserialize(is, att);

				nodeMap[v] = new_node(att);
			}
		}

		// Read and create all edges
		{
			node u, v;
			unsigned numEdges;
			E_TYPE att;

			::Deserialize(is, numEdges);

			for (unsigned i = 0; i < numEdges; ++i)
			{
				//::Deserialize(is, u);
				//::Deserialize(is, v);
				u.Deserialize(is);
				v.Deserialize(is);
				::Deserialize(is, att);

				new_edge(nodeMap[u], nodeMap[v], att);
			}
		}
	}

	node new_node(const V_TYPE& x)
	{
		m_nodes.push_back(new AttrNodeData(this, x));

		return last_node();
	}

	edge new_edge(node u, node v, const E_TYPE& x)
	{
		ASSERT(u != nil && v != nil);

		m_edges.push_back(new AttrEdgeData(u, v, x));

		edge e = last_edge();

		AddEdgeToNodeData(e, u, v);

		return e;
	}

	V_TYPE& attribute(node v)
	{
		return GetAttrNodeData(v)->attr;
	}

	E_TYPE& attribute(edge e)
	{
		return GetAttrEdgeData(e)->attr;
	}

	V_TYPE& operator[](node v)
	{
		return GetAttrNodeData(v)->attr;
	}

	E_TYPE& operator[](edge e)
	{
		return GetAttrEdgeData(e)->attr;
	}

	const V_TYPE& inf(node v) const
	{
		return GetAttrNodeData(v)->attr;
	}

	const E_TYPE& inf(edge e) const
	{
		return GetAttrEdgeData(e)->attr;
	}
};

} // namespace vpl
