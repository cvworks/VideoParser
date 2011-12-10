/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "FSMBackgroundSubtractor.h"
#include <Tools/cv.h>
#include <Tools/CvMatView.h>
#include <Tools/IplImageView.h>
#include <Tools/UserArguments.h>


//Defines for FSMBM method
#define FSMBM_DEFAULT_BACKGROUND_MODEL_ADAPTATION_RATE 0.02
#define FSMBM_DEFAULT_NEGATIVE_LEARNING_STEP 1
#define FSMBM_DEFAULT_POSITIVE_LEARNING_STEP 3
#define FSMBM_DEFAULT_MAX_CONFIDENCE_LEVEL 300
#define FSMBM_DEFAULT_ERODE 1
#define FSMBM_DEFAULT_DILATE 2

using namespace vpl;

extern UserArguments g_userArgs;

FSMBackgroundSubtractor::FSMBackgroundSubtractor()
{
	m_backgroundModel = NULL;
	m_confidenceLevelImage = NULL;

	m_params.prevDifferencePixelValueThreshold = 20;
	m_params.backgroundModelAdaptationRate = FSMBM_DEFAULT_BACKGROUND_MODEL_ADAPTATION_RATE;
	m_params.negativeLearningStep = FSMBM_DEFAULT_NEGATIVE_LEARNING_STEP;
	m_params.positiveLearningStep = FSMBM_DEFAULT_POSITIVE_LEARNING_STEP;
	m_params.maxConfidenceLevel = FSMBM_DEFAULT_MAX_CONFIDENCE_LEVEL;

	m_params.erodeIterations = FSMBM_DEFAULT_ERODE;
    m_params.dilateIterations = FSMBM_DEFAULT_DILATE;
}

///////////////////////////////////////////////////////
// begin required functions from parent VisSysComponent
void FSMBackgroundSubtractor::ReadParamsFromUserArguments()
{
	BackgroundSubtractor::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "always_update", 
		"Whether to (a) always update model, or (b) only do it on first N frames "
		"(set N using trainingLength) and when motion is not detected.", 
		true, &m_params.always_update);

	g_userArgs.ReadArg(Name(), "pixelDiffThreshold", 
		"Minimum intensity difference valid for background pixels", 
		25.0, &m_params.pixelDiffThreshold);

	g_userArgs.ReadArg(Name(), "staticSceneThreshold", 
		"Minimum number of consecutive frames without motion required "
		" to consider the scene static and update the model", 
		20u, &m_params.staticSceneThreshold);

	g_userArgs.ReadArg(Name(), "pixelValueThreshold", 
		"Threshod that determines whether two pixel values are different", 
		25, &m_params.prevDifferencePixelValueThreshold);
}

void FSMBackgroundSubtractor::Initialize(graph::node v)
{
	BackgroundSubtractor::Initialize(v);

}

void FSMBackgroundSubtractor::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{	
	ASSERT(dii.outputIdx >= 0 && dii.outputIdx < 3);
		
	if (dii.outputIdx == 0) 
	{
		dio.imageType = BYTE_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_foreground);
	}
	else if (dii.outputIdx == 1) 
	{
		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_backgroundModel);
	}
	else
	{
		dio.imageType = INT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_confidenceLevelImage);
	}

	std::ostringstream oss;

	oss << ", foreground pixels " << m_stats.numberForegroundPixels 
		<< ", change ratio " << m_stats.ChangeRatio()
		<< ".";

	dio.message = oss.str();
}

void FSMBackgroundSubtractor::Clear()
{
	BackgroundSubtractor::Clear();
}

void FSMBackgroundSubtractor::InitializeBackgroundModel(unsigned width, unsigned height)
{
	m_backgroundModel.set_size(width, height);
	m_confidenceLevelImage.set_size(width, height);

	m_backgroundModel.fill(0);
	m_confidenceLevelImage.fill(0);

	m_background.set_size(width, height);
	m_background.fill(RGBColor(0, 0, 0));

	// Prepare the foreground mask in the base class
	m_foreground.set_size(width, height);
	m_foreground.fill(0);

	m_static_frame_count = 0;
}

/*!
	Assumes that there are no foreground pixels in the image. ie,
	The frame is known to be entirely background.
*/
void FSMBackgroundSubtractor::ProcessBackgroundFrame(
	RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex)
{
	ShowStatus("Learning background model");

	processImage(rgbImg, greyImg, true);
}

/*!
	Assumes that it is not known whether there are foreground pixels 
	in the input image.
*/
void FSMBackgroundSubtractor::FindForeground(RGBImg rgbImg, FloatImg greyImg)
{
	// See if we detected motion in the last frame
	if (ChangeDetected())
	{
		//do not updatde the background
		processImage(rgbImg, greyImg, false);
		m_static_frame_count = 0;
	}
	else
	{
		m_static_frame_count++;

		if(m_static_frame_count > m_params.staticSceneThreshold)  
		{
			//image is static, update the background
			processImage(rgbImg, greyImg, true);
		}
		else
		{
			// image is static, but has not been for long, do not update the background
			processImage(rgbImg, greyImg, false);
		}
	}
}

///////////////////////////////////////////////////////
// end required functions from parent VisSysComponent

void FSMBackgroundSubtractor::ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage)
{
	m_stats.changeDetected = m_stats.numberForegroundPixels >= 
		m_basicParams.minForegroundPixelsForChangeDetection;
}

/*!
	This function does all the background subtraction work. It is designed 
	for fix cameras only. By default, it will learn the background in
	10 seconds. It uses Finite State Machines to learn the background
	and as the model is used, small continual adaptation is made
	to the model. The FSMBM method uses 4 parameters (that can be accessed
	and set using the appropriate method):
	- Negative learning step: its value should always be 1. Greater than
	the positive learning step may create an unstable model.
	- Positive learning step: its value should be greater than the
	negative learning step. By default it is 3.
	- Maximum Confidence Level: this value reflects the maximum number of
	negative examples needed before changing opinion. The higher
	it is, the longer it will take to learn a new background, but
	at the same time, it will be more robust to moving entities.
	By default it is 300, which is around 10 seconds if a video is
	processed at 30 frames per seconds.
	- Background Model Adaptation Rate: this rate is used slowly adapt
	the value of the backgound model. This will take care of the slow
	changes that happens in the background. By default the rate is 0.02. It
	should lie in the range of 0.5 to 0.001.
*/
void FSMBackgroundSubtractor::processImage(RGBImg rgbImage, FloatImg grayImage,
	bool updateModel)
{
	int ls;

	m_stats.Clear();

	auto maskIt = m_foreground.begin();
	auto bmIt = m_backgroundModel.begin();
	auto confIt = m_confidenceLevelImage.begin();
	auto pixelIt = grayImage.begin();

	auto bmColorIt = m_background.begin();
	auto colorPixelIt = rgbImage.begin();

	// Iterate over each pixel
	while (pixelIt != grayImage.end())
	{
		if( ((*pixelIt - *bmIt) > m_params.pixelDiffThreshold) ||
			((*bmIt - *pixelIt) > m_params.pixelDiffThreshold) )
		{
			*maskIt = 255;
			m_stats.numberForegroundPixels++;
			ls = -m_params.negativeLearningStep;
		} 
		else 
		{
			*maskIt = 0;
			ls = m_params.positiveLearningStep;
			m_stats.numberBackgroundPixels++;
		}

		// Update the Confidence Level Image and optionally, the background model
		if (*confIt + ls <= 0)
		{
			*confIt = m_params.positiveLearningStep - m_params.negativeLearningStep;

			if (updateModel)
			{
				*bmIt = *pixelIt;
				*bmColorIt = *colorPixelIt;
			}
		} 
		else 
		{
			// Update the confidence
			*confIt += ls;

			// Cap the confidence s.t. it's not too high
			if(*confIt > m_params.maxConfidenceLevel)
				*confIt = m_params.maxConfidenceLevel;

			if(updateModel && ls > 0)
			{
				*bmIt = (float)(*bmIt * (1.0 - m_params.backgroundModelAdaptationRate)
					 + *pixelIt * m_params.backgroundModelAdaptationRate);
			}
		}

		maskIt++;
		pixelIt++;
		bmIt++;
		confIt++;
		colorPixelIt++;
	}

	//IplImageView img(m_foreground);
	//cv::Mat binMat(img);

	// Binary Image post processing
	CvMatView binMat(m_foreground);

    cv::erode(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.erodeIterations);
    cv::dilate(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.dilateIterations);
}


