/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "GraphMatcher.h"

using namespace vpl;

/*void GraphMatcher::G1ToG2Map(NodeArray& nm2, DoubleArray& weights) const
{
	unsigned i, j;

	nm2.resize(m_nodes1.size());
	weights.resize(m_nodes1.size());

	for (i = 0; i < m_nodes1.size(); ++i)
	{
		j = m_row2colMap[i];

		if (j < m_nodes2.size())
		{
			nm2[i] = m_nodes2[j];
			weights[i] = m_nodeSimMat[i][j];
		}
		else
		{
			ASSERT(j == UINT_MAX);

			nm2[i] = nil;
		}
	}
}*/
