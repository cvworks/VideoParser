/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <GraphTheory/GraphMatcher.h>
#include <GraphTheory/NodeMap.h>

namespace vpl {

// Needed to define a shared pointer type for NodeMatchInfo objects
class NodeAssignment;

//! The shared pointer to a node assignment
typedef std::shared_ptr<NodeAssignment> NodeAssignmentPtr;

/*!
	Represents the information asssociated with an assignment between
	a pair of nodes, one of which is from a ``query'' graph and 
	the other is from a ``model'' graph.
*/
class NodeAssignment
{
	double m_distance;    //!< Absolute difference between the trees rooted at the nodes
	double m_nodeAttributeDistance;

	graph::node m_v0;        //!< Node in graph 0
	graph::node m_v1;        //!< Node in graph 1

	graph::edge m_inEdge0;   //!< In edge connecting mathed parent node in graph 0
	graph::edge m_inEdge1;   //!< In edge connecting mathed parent node in graph 1

	std::vector<NodeAssignmentPtr> m_childAssignments;

public:
	NodeAssignment(graph::node v0, graph::node v1, 
		graph::edge e0, graph::edge e1)
	{
		m_v0 = v0;
		m_v1 = v1;

		m_inEdge0 = e0;
		m_inEdge1 = e1;

		m_distance = -1;
		m_nodeAttributeDistance = -1;
	}

	void operator=(const NodeAssignment& rhs)
	{
		m_distance = rhs.m_distance;

		m_nodeAttributeDistance = rhs.m_nodeAttributeDistance;

		m_v0 = rhs.m_v0;
		m_v1 = rhs.m_v1;

		m_inEdge0 = rhs.m_inEdge0;
		m_inEdge1 = rhs.m_inEdge1;
	}

	bool HasDistance() const
	{
		return m_distance >= 0;
	}

	double Distance() const
	{
		ASSERT(m_distance >= 0);

		return m_distance;
	}

	void SetDistance(const double& dist)
	{
		ASSERT(dist >= 0);

		m_distance = dist;
	}

	void SetNodeAttributeDistance(const double& dist)
	{
		ASSERT(dist >= 0);

		m_nodeAttributeDistance = dist;
	}

	const graph* Graph(int i) const
	{
		ASSERT(i == 0 || i == 1);

		return (i == 0) ? graph_of(m_v0) : graph_of(m_v1);
	}

	graph::node Node(int i) const
	{
		ASSERT(i == 0 || i == 1);

		return (i == 0) ? m_v0 : m_v1;
	}

	graph::edge InEdge(int i) const
	{
		ASSERT(i == 0 || i == 1);

		return (i == 0) ? m_inEdge0 : m_inEdge1;
	}

	graph::node Parent(int i) const
	{
		graph::edge e = InEdge(i);

		return (e == nil) ? nil : opposite(Node(i), e);
	}

	bool IsRootAssignment() const
	{
		return (InEdge(0) == nil && InEdge(1) == nil);
	}

	unsigned NumChildren(int i) const
	{
		graph::node v = Node(i);
		
		if (v == nil)
			return 0;

		const graph* g = graph_of(v);

		unsigned n = g->outdeg(v);

		if (InEdge(i) != nil && g->is_undirected())
			n--;

		return n;
	}
	
	void AddChildAssignment(const NodeAssignmentPtr& ptrNA)
	{
		m_childAssignments.push_back(ptrNA);
	}

	/*!
		Copy node assignments that start from a node in the 
		first graph and end at a node in the second graph.

		The set of assigments include that of this assignment 
		and of all the descendants' assignments.
	*/
	void CopyF2SAssignments(NodeMatchMap& nodeMap) const
	{
		// Copy "root" assignment
		if (m_v0 != nil)
		{
			ASSERT(nodeMap[m_v0].mappedNode == nil);

			nodeMap[m_v0].Set(m_v1,	m_nodeAttributeDistance,
				m_distance, graph_of(m_v0)->index(m_v0), 
				(m_v1 == nil) ? UINT_MAX : graph_of(m_v1)->index(m_v1));
		}

		// Copy child assignments recursively
		for (unsigned i = 0; i < m_childAssignments.size(); i++)
			m_childAssignments[i]->CopyF2SAssignments(nodeMap);
	}

	/*!
		Adds the attribute distances of this node and 
		all its decendants to the value "*pDist". The
		initial value of "*pDist" must be set by
		the caller.

		Only the node assignments formed by a non-nil node in the 
		*second* graph are considered.
	*/
	void AddUpAttributeDistancesS2F(double* pDist) const
	{
		// Add root's attribute distance if *second* node is not nil
		if (m_v1 != nil)
			*pDist += m_nodeAttributeDistance;

		// Add attribute distances of children recursively
		for (unsigned i = 0; i < m_childAssignments.size(); i++)
			m_childAssignments[i]->AddUpAttributeDistancesS2F(pDist);
	}

	/*void CopyAssignment(NodeMatchMap& nodeMap) const
	{
		if (m_v0 != nil)
		{
			ASSERT(nodeMap[m_v0].mappedNode == nil);

			nodeMap[m_v0].Set(m_v1,	m_nodeAttributeDistance,
				m_distance, graph_of(m_v0)->index(m_v0), 
				(m_v1 == nil) ? UINT_MAX : graph_of(m_v1)->index(m_v1));
		}
	}

	void CopyChildAssignments(NodeMatchMap& nodeMap) const
	{
		for (unsigned int i = 0; i < m_childAssignments.size(); i++)
		{
			const NodeAssignmentPtr& ptrNA = m_childAssignments[i];

			ptrNA->CopyAssignment(nodeMap);

			ptrNA->CopyChildAssignments(nodeMap);
		}
	}*/

	void Print(std::ostream& os = std::cout) const
	{
	}

	friend std::ostream& operator<<(std::ostream &os, const NodeAssignment& na)
	{ 
		return (os << na.m_distance); 
	}

	friend std::istream& operator>>(std::istream &is, NodeAssignment& na)
	{ 
		return (is >> na.m_distance); 
	}
};

} //namespace vpl
