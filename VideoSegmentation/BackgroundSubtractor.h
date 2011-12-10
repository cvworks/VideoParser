/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <VideoRepresentation/GenericVideo.h>
#include "BackgroundStats.h"

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;

/*!
*/
class BackgroundSubtractor : public VisSysComponent
{
protected:
	RGBImg m_background;
	ByteImg m_foreground;

	struct BasicParams {
		std::string trainingVideoFilename;
		unsigned firstTrainingFrame;
		unsigned trainingLength;
		unsigned minForegroundPixelsForChangeDetection;
		bool trainOffline;
	};

	BasicParams m_basicParams;
	GenericVideo m_trainingVideo;

	bool m_hasBackgroundModel;
	BackgroundStats m_stats;
	fnum_t m_relativeFrameNumber; //! Frame count per Run() from the last call the Clear().

	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;

protected:

	fnum_t RelativeFrameNumber() const
	{
		return m_relativeFrameNumber;
	}

	virtual void LearnBackgroundModelOffline();

	virtual void InitializeBackgroundModel(unsigned width, unsigned height) = 0;
	virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex) = 0;
	virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg) = 0;

	/*! 
		Called after all background frames have been processed and before any
		foreground frame is seen.
	*/
	virtual void FinalizeBackgroundModel()
	{
		// noithing to do by default
	}

	//! Called after a foreground frame is processed.
	virtual void ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage) = 0;

	/*! 
		Called after the foreground stats have been computed.
	*/
	virtual void FinalizeForegroundProcessing()
	{
		// noithing to do by default
	}

public:
	BackgroundSubtractor(BackgroundStats::Type statsType = BackgroundStats::PIXEL_BASED)
		: m_stats(statsType)
	{
		m_hasBackgroundModel = false;
	}

	bool HasBackgroundModel() const
	{
		return m_hasBackgroundModel;
	}

	bool ChangeDetected() const
	{
		return m_stats.changeDetected;
	}

	//! Returns a copy of the foreground.
	ByteImg Foreground() const
	{
		ByteImg img;

		img.deep_copy(m_foreground);

		return img;
	}

	//! Returns a copy of the background.
	RGBImg Background() const
	{
		RGBImg img;

		img.deep_copy(m_background);

		return img;
	}

	virtual void Initialize(graph::node v);

	virtual void ReadParamsFromUserArguments();

	virtual void Clear()
	{
		m_background.clear();
		m_foreground.clear();
		m_stats.Clear();

		m_hasBackgroundModel = false;
		m_relativeFrameNumber = 0;
	}

	virtual StrArray Dependencies() const
	{
		return StrArray(1, "ImageProcessor");
	}

	virtual std::string GenericName() const
	{
		return "BackgroundSubtractor";
	}

	virtual void Run();

	virtual int NumOutputImages() const
	{
		return 2;
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		ASSERT(i >= 0 && i < 2);

		return (i) ? "Background" : "Foreground";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{	
		ASSERT(dii.outputIdx >= 0 && dii.outputIdx < 2);
		
		if (dii.outputIdx) 
		{
			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(m_background);
		}
		else
		{
			dio.imageType = BYTE_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(m_foreground);
		}
	}
};

} // namespace vpl
