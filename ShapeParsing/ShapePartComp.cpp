/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapePartComp.h"
#include <ShapeRepresentation/NaiveDescriptorComp.h>
#include <ShapeRepresentation/ShapeContextComp.h>
#include <ShapeRepresentation/ContourTreeComp.h>
#include <ShapeRepresentation/HuMomentsComp.h>

using namespace vpl;

void ShapePartComp::LoadShapeSimilarityMeasurer(SHAPE_DESCRIPTOR_TYPE type)
{
	// There might be a descriptor already loaded
	if (m_pSDC)
	{
		delete m_pSDC;
		m_pSDC = NULL;
	}

	m_shapeDescriptorType = type;

	if (type == NAIVE_DESCRIPTOR)
	{
		m_pSDC = new NaiveDescriptorComp();
	}
	else if (type == SHAPE_CONTEXT)
	{
		m_pSDC = new ShapeContextComp();
	}
	else if (type == CONTOUR_TREE)
	{
		m_pSDC = new ContourTreeComp();
	}
	else if (type == HU_MOMENTS)
	{
		m_pSDC = new HuMomentsComp();
	}
}


