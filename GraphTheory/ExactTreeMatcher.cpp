/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ExactTreeMatcher.h"
#include <Tools/STLMatrix.h>

using namespace vpl;

/*!
	@brief 

	Note: g1 is assumed to be the query graph and g2 the model graph.
*/
double ExactTreeMatcher::FindNodeCorrespondences(const double& maxDist)
{
	NodeAssignmentPtr ptrNodeAss;
	double minDist = -1;
	graph::node v1, v2;

	m_ptrRootAssignment.reset();

	// Create a graph with n1 + n2 nodes and save the nodes 
	// associated with the g1 and g2 in different sets
	forall_nodes(v1, *m_pG1)
	{
		forall_nodes(v2, *m_pG2)
		{

			// Create a new match info object
			ptrNodeAss.reset(new NodeAssignment(v1, v2, nil, nil));

			// Compute node similarity and save it into the match info object
			ComputeRootedTreeDistance(ptrNodeAss);

			if (m_ptrRootAssignment == NULL || 
				ptrNodeAss->Distance() < minDist)
			{
				m_ptrRootAssignment = ptrNodeAss;
				minDist = ptrNodeAss->Distance();
			}
		}
	}

	ASSERT(minDist >= 0);

	if (maxDist != UNKNOWN_GRAPH_MATCH_COST && minDist >= maxDist)
	{
		ASSERT(false);
		m_ptrRootAssignment->SetDistance(UNKNOWN_GRAPH_MATCH_COST);

		return UNKNOWN_GRAPH_MATCH_COST;
	}
	else
	{
		return minDist;
	}
}

/*!
	The rooted tree distance is computed based on the NodeDistance(v1, v2)
	cost function defined for two non-nil nodes and also for
	a non-nil node and a nil node. Allowing for one nil node allows us
	to decend recursively down a whole nil subtree and accumulate
	the cost of unmatched nodes (ie, the node that are assigned to a nil node).
*/
void ExactTreeMatcher::ComputeRootedTreeDistance(NodeAssignmentPtr ptrNodeAss) //const
{
	node v1 = ptrNodeAss->Node(0);
	node v2 = ptrNodeAss->Node(1);

	// At least one node should be not nil
	ASSERT(v1 != nil || v2 != nil);

	// Compues the distance between two nodes OR
	// between a node and a nil "node"
	double nodeAttDist = NodeDistance(v1, v2);

	ptrNodeAss->SetNodeAttributeDistance(nodeAttDist);

	unsigned numDesc1 = ptrNodeAss->NumChildren(0);
	unsigned numDesc2 = ptrNodeAss->NumChildren(1);

	unsigned maxNumDesc = MAX(numDesc1, numDesc2);

	if (maxNumDesc == 0)
	{
		ptrNodeAss->SetDistance(nodeAttDist);
	}
	else
	{
		DoubleMatrix costs(maxNumDesc, maxNumDesc);
		STLMatrix<NodeAssignmentPtr> childAssigMat(maxNumDesc, maxNumDesc);

		std::vector<graph::edge> desc1(maxNumDesc, nil);
		graph::edge e1;
		unsigned childIdx1 = 0;

		if (v1 != nil)
		{
			forall_adj_edges(e1, v1)
			{
				if (e1 != ptrNodeAss->InEdge(0))
					desc1[childIdx1++] = e1;
			}
		}

		std::vector<graph::edge> desc2(maxNumDesc, nil);
		graph::edge e2;
		unsigned childIdx2 = 0;

		if (v2 != nil)
		{
			forall_adj_edges(e2, v2)
			{
				if (e2 != ptrNodeAss->InEdge(1))
					desc2[childIdx2++] = e2;
			}
		}

		NodeAssignmentPtr ptrChildAss;
		graph::node u1, u2;

		for (childIdx1 = 0; childIdx1 < maxNumDesc; childIdx1++)
		{
			e1 = desc1[childIdx1];
			u1 = (e1 == nil) ? nil : opposite(v1, e1);

			for (childIdx2 = 0; childIdx2 < maxNumDesc; childIdx2++)
			{
				e2 = desc2[childIdx2];
				u2 = (e2 == nil) ? nil : opposite(v2, e2);

				ptrChildAss.reset(new NodeAssignment(u1, u2, e1, e2));

				ComputeRootedTreeDistance(ptrChildAss);

				childAssigMat(childIdx1, childIdx2) = ptrChildAss;

				costs[childIdx1][childIdx2] = ptrChildAss->Distance();
			}
		}

		if (maxNumDesc == 1)
		{
			ptrNodeAss->SetDistance(nodeAttDist + costs(0, 0));
			ptrNodeAss->AddChildAssignment(childAssigMat(0, 0));
		}
		else
		{
			HungarianBGMatcher ha;
			UIntVector row2colMap, col2rowMap;

			double descDist = ha.SolveMinCost(costs);

			ha.GetCorrespondences(row2colMap, col2rowMap);

			ptrNodeAss->SetDistance(nodeAttDist + descDist);

			// Add children assignments to ptrNodeAss
			for (unsigned i = 0; i < row2colMap.size(); ++i)
			{
				ptrNodeAss->AddChildAssignment(childAssigMat(i, row2colMap[i]));
			}
		}
	}
}

/*void ExactTreeMatcher::GetNodeMap(NodeMatchMap& nodeMap) const
{
	const DAG& dag0 = *m_ptrRootAssignment->Graph(0);

	nodeMap.init(dag0);

	m_ptrRootAssignment->CopyAssignment(nodeMap);
	m_ptrRootAssignment->CopyChildAssignments(nodeMap);
	m_ptrRootAssignment->CopyParentAssignments(nodeMap);

}*/
