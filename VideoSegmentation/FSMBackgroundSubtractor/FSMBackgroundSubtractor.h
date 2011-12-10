/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "../BackgroundSubtractor.h"

namespace vpl {
	/*!
	The goal of this component is to separate foreground pixels from background
	pixels in an image. The component is given a frame, and identifies which pixels
	in this frame belong to a moving object.

	This algorithm is designed for fix camera only. It uses a model
	of the background (which it continually learns) to segment between background and
	foreground objects. This method only uses one frame.
	*/
	class FSMBackgroundSubtractor : public BackgroundSubtractor
	{
	protected:
		struct Params 
		{
			bool always_update;
			double pixelDiffThreshold;
			unsigned staticSceneThreshold;
			
			int prevDifferencePixelValueThreshold;
			double backgroundModelAdaptationRate;
			int negativeLearningStep;
			int positiveLearningStep;
			int maxConfidenceLevel;

			int erodeIterations;
			int dilateIterations;
		};

	private:
		Params m_params;
		FloatImg m_backgroundModel;
		IntImg m_confidenceLevelImage;
		unsigned m_static_frame_count;

	protected:
		void processImage(RGBImg rgbImage, FloatImg grayImage, bool updateModel);
		virtual void ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage);

	public:
		FSMBackgroundSubtractor();

		/*!
		It returns an image that represents	the confidence level 
		of the background model. A white pixel means that the 
		confidence about the value of the pixel in the background 
		model is high; a black pixel means that the confidence is low.
		*/
		IntImg GetConfidenceLevelImage() const
		{
			return m_confidenceLevelImage;
		}

	public: // Required functions from parent VisSysComponent
		virtual void ReadParamsFromUserArguments();

		virtual void Initialize(graph::node v);

		virtual void Clear();

		virtual std::string ClassName() const
		{
			return "FSMBackgroundSubtractor";
		}

		void InitializeBackgroundModel(unsigned width, unsigned height);
		virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex);
		virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg);

		virtual int NumOutputImages() const
		{
			return 3;
		}

		virtual std::string GetOutputImageLabel(int i) const
		{
			ASSERT(i >= 0 && i < 3);

			switch (i)
			{
				case 0: return "Foreground";
				case 1: return "Model";
				case 2: return "Confidence";
			}

			return "error";
		}

		virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;
	};

} //namespace vpl
