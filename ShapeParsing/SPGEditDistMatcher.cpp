/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "SPGEditDistMatcher.h"

using namespace vpl;

typedef SPGEditDistMatcher SPGEDM;

double SPGEDM::Match()
{
	// for each possible root
	// get a list of all nodes if DFS order
	// process the list recursively as spec by Kimia
	// return cost and leave enough data to create node mapping

	return 0;
}

double SPGEDM::NodeMatchCost(node v1, node v2)
{
	return 0;
}
