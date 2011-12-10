/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "AdaptiveBackgroundSubtractor.h"
#include <Tools/cv.h>
#include <Tools/CvMatView.h>
#include <Tools/IplImageView.h>
#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

void AdaptiveBackgroundSubtractor::ReadParamsFromUserArguments()
{
	BackgroundSubtractor::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "minRelearnTime", 
		"Number of seconds that a candidate background pixel must be classified as "
		" 'not foreground' before it is learned as the model od background", 
		time_t(60 * 20), &m_params.minRelearnTime);

	g_userArgs.ReadArg(Name(), "minElapsedTimeForUpdate", 
		"Number of seconds needed to update a background pixel", 
		time_t(60), &m_params.minElapsedTimeForUpdate);

	g_userArgs.CheckMinValues(Name(), "minElapsedTimeForUpdate", 1);

	g_userArgs.ReadArg(Name(), "candidatePixelBufferSize", 
		"Size of the buffer used to hold samples of candidate background pixels", 
		unsigned(m_params.minElapsedTimeForUpdate * 15), 
		&m_params.candidatePixelBufferSize);

	g_userArgs.CheckMinValues(Name(), "candidatePixelBufferSize", 1);

	g_userArgs.ReadArg(Name(), "pixelDiffThreshold", 
		"Minimum intensity difference valid for background pixels", 
		25.0f, &m_params.pixelDiffThreshold);

	g_userArgs.ReadArg(Name(), "erodeIterations", 
		"Number of erode iteration to perform on foreground mask (before dilate)", 
		1, &m_params.erodeIterations);

	g_userArgs.ReadArg(Name(), "dilateIterations", 
		"Number of dilate iteration to perform on foreground mask (after erode)", 
		2, &m_params.dilateIterations);
}

void AdaptiveBackgroundSubtractor::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{		
	if (dii.outputIdx == FOREGROUND_IMG) 
	{
		dio.imageType = BYTE_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_foreground);
	}
	else if (dii.outputIdx == BACKGROUND_IMG) 
	{
		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_backgroundModel);
	}
	else if (dii.outputIdx == MODEL_VARIANCE)
	{
		FloatImg img(m_modelBuffer.ni(), m_modelBuffer.nj());

		for (unsigned i = 0; i < img.ni(); i++)
		{
			for (unsigned j = 0; j < img.nj(); j++)
			{
				img(i, j) = (float)m_modelBuffer(i, j).variance();
			}
		}

		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}
	else if (dii.outputIdx == CANDIDATE_VARIANCE)
	{
		FloatImg img(m_candidateBuffer.ni(), m_candidateBuffer.nj());

		for (unsigned i = 0; i < img.ni(); i++)
		{
			for (unsigned j = 0; j < img.nj(); j++)
			{
				img(i, j) = (float)m_candidateBuffer(i, j).variance();
			}
		}

		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}
	else if (dii.outputIdx == MODEL_THRESHOLD)
	{
		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(m_backgroundModelThreshold);
	}

	std::ostringstream oss;

	oss << ", foreground pixels " << m_stats.numberForegroundPixels 
		<< ", change ratio " << m_stats.ChangeRatio()
		<< ".";

	dio.message = oss.str();
}

void AdaptiveBackgroundSubtractor::InitializeBackgroundModel(unsigned width, unsigned height)
{
	m_backgroundModel.set_size(width, height);
	m_backgroundModel.fill(0);

	m_backgroundModelThreshold.set_size(width, height);
	m_backgroundModelThreshold.fill(m_params.pixelDiffThreshold);

	m_background.set_size(width, height);
	m_background.fill(RGBColor(0, 0, 0));

	// Prepare the foreground mask in the base class
	m_foreground.set_size(width, height);
	m_foreground.fill(0);

	m_modelBuffer.set_size(width, height, m_basicParams.trainingLength);
	m_candidateBuffer.set_size(width, height, m_params.candidatePixelBufferSize);
}

/*!
	Assumes that there are no foreground pixels in the image. ie,
	The frame is known to be entirely background.
*/
void AdaptiveBackgroundSubtractor::ProcessBackgroundFrame(
	RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex)
{
	ShowStatus("Learning background model");

	for (unsigned i = 0; i < m_backgroundModel.ni(); i++)
	{
		for (unsigned j = 0; j < m_backgroundModel.nj(); j++)
		{
			PixelBuffer& modelBuff = m_modelBuffer(i, j);

			modelBuff.push_back(greyImg(i, j), Timestamp());

			m_backgroundModel(i, j) = (float) modelBuff.mean();
		}
	}
}

/*!
	This function does all the background subtraction work. It is designed 
	for fix cameras only. 
*/
void AdaptiveBackgroundSubtractor::FindForeground(RGBImg rgbImg, FloatImg greyImg)
{
	double imgModDiff, modDiff;

	const int NI = m_backgroundModel.ni();
	const int NJ = m_backgroundModel.nj();

	// Iterate over each pixel
	//#pragma omp parallel for
	for (int i = 0; i < NI; i++)
	{
		for (int j = 0; j < NJ; j++)
		{
			PixelBuffer& modelBuff = m_modelBuffer(i, j);
			PixelBuffer& candBuff = m_candidateBuffer(i, j);

			imgModDiff = fabs(greyImg(i, j) - m_backgroundModel(i, j));
			modDiff = fabs(modelBuff.mean() - greyImg(i, j));

			// revert to original model as soon as possible
			if (modDiff < imgModDiff && modDiff <= m_params.pixelDiffThreshold)
			{
				m_backgroundModel(i, j) = (float) modelBuff.mean();
				m_backgroundModelThreshold(i, j) = m_params.pixelDiffThreshold;

				imgModDiff = modDiff;
			}

			if (imgModDiff <= m_backgroundModelThreshold(i, j))
			{
				// It is a background pixel
				m_foreground(i, j) = 0;

				m_stats.numberBackgroundPixels++;
				
				// Reset the color candidates of the current pixel
				if (candBuff.is_background() && 
					candBuff.elapsed_time(Timestamp()) >= m_params.minRelearnTime)
				{
					// the candidate seems to be a good "new" model
					modelBuff = candBuff;
				}
				else if (!candBuff.is_background() && !candBuff.empty())
				{
					// start over collecting candidates for this pixel
					candBuff.clear();
				}
			}
			else
			{
				// If the candidate buffer is a tryout, reset it because it
				// has failed
				if (candBuff.is_background())
				{
					candBuff.clear();
				}

				// It is a foreground pixel
				m_foreground(i, j) = 255;

				m_stats.numberForegroundPixels++;

				// Add a new color candidate for the current pixel
				candBuff.push_back(greyImg(i, j), Timestamp());

				// See if the candidate model is better
				if (candBuff.elapsed_time() >= m_params.minElapsedTimeForUpdate &&
					candBuff.std_dev() <= m_backgroundModelThreshold(i, j))
				{
					double canDiff = fabs(candBuff.mean() - greyImg(i, j));

					if (canDiff <= m_params.pixelDiffThreshold)
					{
						m_backgroundModel(i, j) = (float) candBuff.mean();
						//m_backgroundModelThreshold(i, j) = (float) (2.5 * candBuff.std_dev());

						candBuff.set_background_state(true);
					}
				}
			}
		}
	}

	// Binary Image post processing
	CvMatView binMat(m_foreground);

    cv::erode(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.erodeIterations);
    cv::dilate(binMat, binMat, cv::Mat(), cv::Point(-1,-1), m_params.dilateIterations);
}




