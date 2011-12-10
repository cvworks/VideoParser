/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>

namespace vpl {

// Declare prototypes of expected parent components
class ObjectLearner;
class ShapeMatcher;

/*!
	@brief Wrapper for a generic image segmentation algorithm.
*/
class FeatureEmbedder : public VisSysComponent
{
protected:
	//Parent components
	std::shared_ptr<const ObjectLearner> m_pObjectLearner;
	std::shared_ptr<const ShapeMatcher> m_pShapeMatcher;

public:	
	virtual void Initialize(graph::node v);

	virtual std::string GenericName() const
	{
		return "FeatureEmbedder";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ObjectLearner");
		deps.push_back("ShapeMatcher");

		return deps;
	}
};

} // namespace vpl


