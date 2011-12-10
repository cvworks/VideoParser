/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Graph.h"
#include "NodeMap.h"

namespace vpl {

struct NodeMatch
{
	graph::node mappedNode;
	double rootedTreeDist;  //!< Rooted tree distance
	double nodeAttDist;     //!< Node attribute distance
	unsigned srcNodeIdx;    //!< Index of source node or UINT_MAX if node is nil
	unsigned tgtNodeIdx;    //!< Index of target node or UINT_MAX if node is nil

	NodeMatch(graph::node v = nil)
	{
		mappedNode = v;
	}

	//! graphs nodes can't be copied by default
	void operator=(const NodeMatch& rhs)
	{
		mappedNode = rhs.mappedNode;
		rootedTreeDist = rhs.rootedTreeDist;
		nodeAttDist = rhs.nodeAttDist;
		srcNodeIdx = rhs.srcNodeIdx;
		tgtNodeIdx = rhs.tgtNodeIdx;
	}

	void Set(graph::node v, const double& nad, const double& rtd,
		unsigned srcIdx, unsigned tgtIdx)
	{
		mappedNode = v;
		nodeAttDist = nad;
		rootedTreeDist = rtd;
		srcNodeIdx = srcIdx;
		tgtNodeIdx = tgtIdx;
	}
};

typedef NodeMap<NodeMatch> NodeMatchMap;

} // namespace vpl
