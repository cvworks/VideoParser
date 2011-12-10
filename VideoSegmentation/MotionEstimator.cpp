/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "MotionEstimator.h"
#include <VideoParser/ImageProcessor.h>

using namespace vpl;

void MotionEstimator::Run()
{
	// Declare pIP using macro that takes care of casting
	std::shared_ptr<const ImageProcessor> pIP = FindParentComponent(ImageProcessor);
	
	if (!pIP)
	{
		ShowMissingDependencyError(ImageProcessor);
		return;
	}

	m_hasOutput = (pIP->BufferSize() >= 2);

	// If there are current and previous frames, estimate motion
	if (m_hasOutput)	
		Estimate(pIP->GetGreyImage(0), pIP->GetGreyImage(1));
}
