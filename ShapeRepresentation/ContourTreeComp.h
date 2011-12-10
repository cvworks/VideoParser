/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptorComp.h"
#include "ContourTree.h"

namespace vpl
{
class ContourTreeComp : public ShapeDescriptorComp
{
public:
	virtual double Match(const ShapeDescriptor& sd1, 
		const ShapeDescriptor& sd2);
};

} // namespace vpl

