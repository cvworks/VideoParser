/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CvGaussBackgroundSubtractor.h"
#include <VideoParser/ImageProcessor.h>
#include <Tools/IplImageView.h>
#include <opencv2/video/background_segm.hpp>
#include <Tools/CvUtils.h> // gets cvShowImage

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h>

using namespace vpl;

CvGaussBGStatModelParams g_params;

/*!
	CV_BG_MODEL_MOG

	Interface of Gaussian mixture algorithm

	   "An improved adaptive background mixture model for real-time tracking with shadow detection"
	   P. KadewTraKuPong and R. Bowden,
	   Proc. 2nd European Workshp on Advanced Video-Based Surveillance Systems, 2001."
	   http://personal.ee.surrey.ac.uk/Personal/R.Bowden/publications/avbs01/avbs01.pdf

	CvGaussBGStatModelParams
	{    
		int     win_size;   Is equal to 1/alpha
		int     n_gauss;
		double  bg_threshold, std_threshold, minArea;
		double  weight_init, variance_init;
	}

	Default parameters of gaussian background detection algorithm

	#define CV_BGFG_MOG_BACKGROUND_THRESHOLD     0.7     threshold sum of weights for background test
	#define CV_BGFG_MOG_STD_THRESHOLD            2.5     lambda=2.5 is 99%
	#define CV_BGFG_MOG_WINDOW_SIZE              200     Learning rate; alpha = 1/CV_GBG_WINDOW_SIZE
	#define CV_BGFG_MOG_NGAUSSIANS               5       = K = number of Gaussians in mixture
	#define CV_BGFG_MOG_WEIGHT_INIT              0.05
	#define CV_BGFG_MOG_SIGMA_INIT               30
	#define CV_BGFG_MOG_MINAREA                  15.f
*/
void CvGaussBackgroundSubtractor::ReadParamsFromUserArguments()
{
	BackgroundSubtractor::ReadParamsFromUserArguments();

	g_params.win_size      = CV_BGFG_MOG_WINDOW_SIZE;
	g_params.bg_threshold  = CV_BGFG_MOG_BACKGROUND_THRESHOLD;

	g_params.std_threshold = CV_BGFG_MOG_STD_THRESHOLD;
	g_params.weight_init   = CV_BGFG_MOG_WEIGHT_INIT;

	g_params.variance_init = CV_BGFG_MOG_SIGMA_INIT*CV_BGFG_MOG_SIGMA_INIT;
	g_params.minArea       = CV_BGFG_MOG_MINAREA;
	g_params.n_gauss       = CV_BGFG_MOG_NGAUSSIANS;
}

void CvGaussBackgroundSubtractor::Clear()
{
	BackgroundSubtractor::Clear();

	if (m_hasBackgroundModel)
	{
		cvReleaseBGStatModel(&m_pBackgroundModel);
		m_hasBackgroundModel = false;
	}
}

void CvGaussBackgroundSubtractor::ProcessBackgroundFrame(RGBImg sample, 
	FloatImg greyImg, fnum_t frameIndex)
{
	IplImageView iplView(sample, false);

	if (!m_pBackgroundModel)
		m_pBackgroundModel = cvCreateGaussianBGModel(iplView, &g_params);
	else
		cvUpdateBGStatModel(iplView, m_pBackgroundModel);

}

/*!
	We discriminate between foreground and background pixels
	by building and maintaining a model of the background.
	Any pixel which does not fit this model is then deemed
	to be foreground.
*/
void CvGaussBackgroundSubtractor::FindForeground(RGBImg newFrame, FloatImg greyImg)
{
	IplImageView iplView(newFrame, false);

	cvUpdateBGStatModel(iplView, m_pBackgroundModel);

	IplImageToVXLImage(m_pBackgroundModel->foreground, m_foreground, 1);
	IplImageToVXLImage(m_pBackgroundModel->background, m_background);
}