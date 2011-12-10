/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptorComp.h"
#include "NaiveDescriptor.h"

namespace vpl
{
class NaiveDescriptorComp : public ShapeDescriptorComp
{
public:
	virtual double Match(const ShapeDescriptor& sd1, const ShapeDescriptor& sd2)
	{
		const NaiveDescriptor& nd1 = CastDescriptor<NaiveDescriptor>(sd1);
		const NaiveDescriptor& nd2 = CastDescriptor<NaiveDescriptor>(sd2);
			
		double diff = abs((int)nd1.Points().size() - (int)nd2.Points().size());
		//double maxDiff = MAX(nd1.Points().size(), nd2.Points().size());
			
		//return 1 - diff / maxDiff;
		return diff;
	}
};

} // namespace vpl
