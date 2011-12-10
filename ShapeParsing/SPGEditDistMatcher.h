/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SPG_EDIT_DIST_MATCHER_H_
#define _SPG_EDIT_DIST_MATCHER_H_

#include "SPGMatcher.h"

namespace vpl {

class SPGEditDistMatcher : public SPGMatcher
{
public: 
	virtual double Match();

	virtual double NodeMatchCost(node v1, node v2);
};

} //namespace vpl

#endif //_SPG_EDIT_DIST_MATCHER_H_