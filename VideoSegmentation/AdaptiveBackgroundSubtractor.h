/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BackgroundSubtractor.h"
#include <Tools/PixelwiseFrameBuffer.h>

namespace vpl {
/*!
	The goal of this component is to separate foreground pixels from background
	pixels in an image. The component is given a frame, and identifies which pixels
	in this frame belong to a moving object.

	This algorithm is designed for fix camera only. It uses a model
	of the background (which it continually learns) to segment between background and
	foreground objects. This method only uses one frame.
*/
class AdaptiveBackgroundSubtractor : public BackgroundSubtractor
{
	enum {FOREGROUND_IMG, BACKGROUND_IMG, MODEL_VARIANCE, 
		CANDIDATE_VARIANCE, MODEL_THRESHOLD};

protected:
	struct Params 
	{
		float pixelDiffThreshold;
		time_t minElapsedTimeForUpdate;
		time_t minRelearnTime;
		unsigned candidatePixelBufferSize;

		int erodeIterations;
		int dilateIterations;
	};

private:
	Params m_params;
	FloatImg m_backgroundModel;
	FloatImg m_backgroundModelThreshold;

	PixelwiseFrameBuffer m_modelBuffer;
	PixelwiseFrameBuffer m_candidateBuffer;

protected:
	//! Called after a foreground frame is processed.
	virtual void ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage)
	{
		m_stats.changeDetected = m_stats.numberForegroundPixels >= 
			m_basicParams.minForegroundPixelsForChangeDetection;
	}

public: // Required functions from parent VisSysComponent
	virtual void ReadParamsFromUserArguments();	

	virtual std::string ClassName() const
	{
		return "AdaptiveBackgroundSubtractor";
	}

	void InitializeBackgroundModel(unsigned width, unsigned height);
	virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex);
	virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg);

	virtual int NumOutputImages() const
	{
		return 5;
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case FOREGROUND_IMG: return "Foreground";
			case BACKGROUND_IMG: return "Background Model";
			case MODEL_VARIANCE: return "Model variance";
			case CANDIDATE_VARIANCE: return "Candidate variance";
			case MODEL_THRESHOLD: return "Model threshold";
		}

		return "error";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;
};

} //namespace vpl
