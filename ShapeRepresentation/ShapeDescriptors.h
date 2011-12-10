/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <ShapeRepresentation/NaiveDescriptor.h>
#include <ShapeRepresentation/ShapeContext.h>
#include <ShapeRepresentation/ContourTree.h>
#include <ShapeRepresentation/HuMoments.h>
#include <Tools/Serialization.h>

namespace vpl {

/*!
	Any changes to this enum should be reflected in the following functions:

	1- StrArray ShapeDescriptorTypes()
	2- ShapeDescriptor* NewShapeDescriptor(SHAPE_DESCRIPTOR_TYPE type)
	3- ShapePartComp.cpp to load the comparison functions
*/
enum SHAPE_DESCRIPTOR_TYPE {VOID_DESCRIPTOR, NAIVE_DESCRIPTOR, 
	SHAPE_CONTEXT, CONTOUR_TREE, HU_MOMENTS};

inline StrArray ShapeDescriptorTypes()
{
	StrArray types;

	// The order of insertion MUST reflect the enum values
	// associated with the descriptors
	types.push_back("void");
	types.push_back("Naive");
	types.push_back("ShapeContext");
	types.push_back("ContourTree");
	types.push_back("HuMoments");

	return types;
}

inline ShapeDescriptor* NewShapeDescriptor(SHAPE_DESCRIPTOR_TYPE type)
{
	if (type == VOID_DESCRIPTOR)
		return NULL;
	else if (type == NAIVE_DESCRIPTOR)
		return new NaiveDescriptor;
	else if (type == SHAPE_CONTEXT)
		return new ShapeContext;
	else if (type == CONTOUR_TREE)
		return new ContourTree;
	else if (type == HU_MOMENTS)
		return new HuMoments;
	
	ASSERT(false);
	return NULL;
}

inline void ReadShapeDescriptorParams(SHAPE_DESCRIPTOR_TYPE type)
{
	ShapeDescriptor* pSD = NewShapeDescriptor(type);

	if (pSD)
	{
		pSD->ReadClassParameters();
		delete pSD;
	}
}

/*!
	// Read the appropriate shape descriptor params

	GenericParametersPtr ptrDescParams;

	if (s_params.descriptorType == ShapePart::SHAPE_CONTEXT)
		ptrDescParams = new ShapeContext::Params();
	else
		ASSERT(false);

	ptrShapeParams->ReadFromUserArguments();
*/

} // namespace vpl

//! Define how to serialize the SD type outside the vpl name space
DECLARE_BASIC_SERIALIZATION(vpl::SHAPE_DESCRIPTOR_TYPE)
