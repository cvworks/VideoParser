/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptor.h"
#include <Tools/LinearAlgebra.h>

namespace vpl {

/*!
	Abstract class represeting an algorithm for matching
	two shape descriptors.

	The ShapeDescriptorComp derived class used must be compatible
	with the actual derived classes of the shape descriptors
	to match.
*/
class ShapeDescriptorComp
{
protected:
	//! Helper function to cast shape descriptor references
	template <typename T>
	const T& CastDescriptor(const ShapeDescriptor& sd)
	{
		// Throws a std::bad_cast if casting is not valid
		return dynamic_cast<const T&>(sd);
	}

public:
	//! The necessary virtual destructor
	virtual ~ShapeDescriptorComp()
	{
	}

	/*!
		For some descriptors, this function can be called AFTER Match()
		in order to retrieve the transformation parameters.
	*/
	virtual void GetTransformationParams(const ShapeDescriptor& sd1, 
		const ShapeDescriptor& sd2, PointTransform* pPT)
	{
		pPT->Clear();
	}

	virtual double Match(const ShapeDescriptor& sd1, const ShapeDescriptor& sd2) = 0;
};

} // namespace vpl