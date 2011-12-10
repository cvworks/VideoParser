/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BackgroundSubtractor.h"
#include <VideoParser/ImageProcessor.h>
#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

void BackgroundSubtractor::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);

	m_relativeFrameNumber = 0;
	m_hasBackgroundModel = false;
}

void BackgroundSubtractor::ReadParamsFromUserArguments()
{
	// Read the parameters in for the base class first
	VisSysComponent::ReadParamsFromUserArguments();

	// The default value for the training video is the current video
	m_basicParams.trainingVideoFilename = VideoFilename();

	g_userArgs.ReadBoolArg(Name(), "trainOffline", 
		"File name of the background training video for offline training", 
		false, &m_basicParams.trainOffline);

	g_userArgs.ReadArg(Name(), "trainingVideo", 
		"File name of the background training video for offline learning", 
		m_basicParams.trainingVideoFilename, &m_basicParams.trainingVideoFilename);

	g_userArgs.ReadArg(Name(), "firstTrainingFrame", "First frame of the background training video", 
		0u, &m_basicParams.firstTrainingFrame);

	g_userArgs.ReadArg(Name(), "trainingLength", "Number of frames to read from the background training video", 
		15u, &m_basicParams.trainingLength);

	g_userArgs.ReadArg(Name(), "minForegroundPixelsForChangeDetection", 
		"Maximum number of white pixels not detect as motion", 
		10u, &m_basicParams.minForegroundPixelsForChangeDetection);
}

/*!
	Opens a video file and learns the background model from trainingLength
	number of frames, starting at frame firstTrainingFrame.
*/
void BackgroundSubtractor::LearnBackgroundModelOffline()
{
	ASSERT(!m_hasBackgroundModel);

	ShowStatus("Creating background model");

	if (m_basicParams.trainingVideoFilename.empty())
	{
		ShowError("There is no specified training background video");
		return;
	}

	if (!m_trainingVideo.Load(m_basicParams.trainingVideoFilename))
	{
		ShowError("Cannot load training background video");
		return;
	}

	m_trainingVideo.ReadFrame(m_basicParams.firstTrainingFrame);

	for (fnum_t i = 0; i < m_basicParams.trainingLength; i++, 
		m_trainingVideo.ReadNextFrame())
	{
		ShowStatus2("Training backround with frame", 
			m_trainingVideo.FrameNumber(), "...");

		ProcessBackgroundFrame(m_trainingVideo.GetCurrentRGBFrame(), 
			m_trainingVideo.GetCurrentGreyScaleFrame(), i);
	}

	FinalizeBackgroundModel();
	m_hasBackgroundModel = true;
}

void BackgroundSubtractor::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(ImageProcessor);
		return;
	}

	if (m_pImgProcessor->IsTooDark())
	{
		ShowStatus("Image is too dark. Background subtraction is suspended.");
		return;
	}

	RGBImg img = m_pImgProcessor->GetRGBImage();

	m_stats.Clear();

	if (m_hasBackgroundModel || m_basicParams.trainOffline)
	{
		if (!m_hasBackgroundModel)
			LearnBackgroundModelOffline();

		FindForeground(img,	m_pImgProcessor->GetGreyImage());

		ComputeForegroundStats(img,	m_pImgProcessor->GetGreyImage());

		FinalizeForegroundProcessing();
	}
	else
	{
		if (m_relativeFrameNumber == 0)
			InitializeBackgroundModel(img.ni(), img.nj());

		ProcessBackgroundFrame(img, m_pImgProcessor->GetGreyImage(), 
			m_relativeFrameNumber);

		if (m_relativeFrameNumber >= m_basicParams.trainingLength)
		{
			FinalizeBackgroundModel();
			m_hasBackgroundModel = true;
		}
	}

	m_relativeFrameNumber++;	
}

