/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SPG_MATCHER_H_
#define _SPG_MATCHER_H_

#include "ShapeParsingModel.h"

namespace vpl {

class SPGMatcher
{
protected:
	const SPM& m_g1;
	const SPM& m_g2;

	typedef graph::node node;

public:
	SPGMatcher(const SPM& g1, const SPM& g2) 
		: m_g1(g1), m_g2(g2)
	{
	}

	virtual double Match() = 0;

	virtual double NodeMatchCost(node v1, node v2) = 0;

	/*static double Match(const BCG& g1, const BCG& g2)
	{
		return SPGMatcher(g1, g2).Match();
	}*/
};

} //namespace vpl

#endif //_SPG_MATCHER_H_