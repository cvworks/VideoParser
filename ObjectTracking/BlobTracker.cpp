/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BlobTracker.h"
#include <VideoParser/ImageProcessor.h>
#include <VideoSegmentation/BackgroundSubtractor.h>
#include <ImageSegmentation/BlobFinder.h>
#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

void BlobTracker::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);
	m_pBlobFinder = FindParentComponent(BlobFinder);

	m_dbm.init(m_pSQLDatabase);
}

void BlobTracker::ReadParamsFromUserArguments()
{
	// Read the parameters of the base class first
	VisSysComponent::ReadParamsFromUserArguments();

}

