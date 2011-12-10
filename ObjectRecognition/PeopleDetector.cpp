/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "PeopleDetector.h"
#include <Tools/UserArguments.h>
#include <VideoParser/ImageProcessor.h>

using namespace vpl;

extern UserArguments g_userArgs;

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void PeopleDetector::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();
}

void PeopleDetector::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);
	
	m_pImgProcessor = FindParentComponent(ImageProcessor);
}

