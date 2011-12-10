/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "LemonGraph.h"
#include <lemon/matching.h>
#include <lemon/connectivity.h>
#include <limits>

using namespace vpl;

/*template <typename N_TYPE, typename E_TYPE, typename G_TYPE>
int LemonGraph<N_TYPE, E_TYPE, G_TYPE>::connectedComponents(typename G_TYPE::NodeMap<int>& compMap)
{
	return lemon::connectedComponents(*this, compMap);
}*/

int LemonGraph<>::connectedComponents(lemon::SmartGraph::NodeMap<int>& compMap)
{
	return lemon::connectedComponents(*this, compMap);
}

