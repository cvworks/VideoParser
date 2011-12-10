/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeParsingModel.h"

namespace vpl {

/*!
	@brief Wrapper for a generic shape parser algorithm
*/
struct Params
{
	SHAPE_DESCRIPTOR_TYPE descriptorType;
	unsigned boundarySubsamplingValue;
	SAMPLING_TYPE boundarySubsamplingType;
	SAMPLING_SCOPE boundarySubsamplingScope;

	DECLARE_BASIC_MEMBER_SERIALIZATION
};

} // namespace vpl

