/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <GraphTheory/GraphMatcher.h>
#include "NodeAssignment.h"

namespace vpl {

/*! 
	Exact tree matching algorithm
*/
class ExactTreeMatcher : public GraphMatcher
{
protected:
	NodeAssignmentPtr m_ptrRootAssignment;

	void ComputeRootedTreeDistance(NodeAssignmentPtr ptrNodeAss);

public:
	virtual double FindNodeCorrespondences(const double& maxCost);
	
	/*!
		Gets the correspondence map from the nodes in the first graph 
		to the modes in the second graph.
	*/
	virtual void GetF2SNodeMap(NodeMatchMap& nodeMap) const
	{
		ASSERT(m_ptrRootAssignment != NULL);
		ASSERT(m_ptrRootAssignment->Distance() != UNKNOWN_GRAPH_MATCH_COST);

		nodeMap.Clear();
		nodeMap.Init(*m_pG1); //, NodeAssignmentInfo(nil)

		m_ptrRootAssignment->CopyF2SAssignments(nodeMap);

		//m_ptrRootAssignment->CopyAssignment(nodeMap);
		//m_ptrRootAssignment->CopyChildAssignments(nodeMap);
	}

	/*!
		Adds up the attribute distances from the nodes in the second
		graph to the nodes in the first graph.
	*/
	virtual double GetGraphDistanceS2F() const
	{
		ASSERT(m_ptrRootAssignment != NULL);
		ASSERT(m_ptrRootAssignment->Distance() != UNKNOWN_GRAPH_MATCH_COST);

		double dist = 0;

		m_ptrRootAssignment->AddUpAttributeDistancesS2F(&dist);

		return dist;
	}
};

} //namespace vpl
