/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BackgroundFeatureSubtractor.h"

namespace vpl {

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class FeatureGridSubtractor : public BackgroundFeatureSubtractor
{
public:	
	virtual std::string ClassName() const
	{
		return "FeatureGridSubtractor";
	}
};

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class HybridFeatureGridSubtractor : public FeatureGridSubtractor
{
	std::shared_ptr<BackgroundFeatureSubtractor> m_pFeatureSubtractor;

public:	
	virtual std::string ClassName() const
	{
		return "HybridFeatureGridSubtractor";
	}

	virtual void AddSubordinateComponents()
	{
		FeatureGridSubtractor::AddSubordinateComponents();

		m_pFeatureSubtractor.reset(new BackgroundFeatureSubtractor);

		AddSubordinateComponent(m_pFeatureSubtractor);
	}

	virtual void Run()
	{
		// Only run if there is no background model build yet or 
		// if changed has been detected
		if (!HasBackgroundModel() || m_pFeatureSubtractor->ChangeDetected())
			FeatureGridSubtractor::Run();
	}
};

} // namespace vpl

