/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CvFGDBackgroundSubtractor.h"
#include <VideoParser/ImageProcessor.h>
#include <Tools/IplImageView.h>
#include <opencv2/video/background_segm.hpp>
#include <Tools/CvUtils.h> // gets cvShowImage

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h>

using namespace vpl;

CvFGDStatModelParams g_params;

/*!
	CV_BG_MODEL_FGD

	See the above-referenced Li/Huang/Gu/Tian paper
	for a full description of these background-model
	tuning parameters.

	Nomenclature:  'c'  == "color", a three-component red/green/blue vector.
							We use histograms of these to model the range of
							colors we've seen at a given background pixel.

				   'cc' == "color co-occurrence", a six-component vector giving
							RGB color for both this frame and preceding frame.
								We use histograms of these to model the range of
							color CHANGES we've seen at a given background pixel.

	CvFGDStatModelParams
	{
		int    Lc;			Quantized levels per 'color' component. Power of two, typically 32, 64 or 128.
		int    N1c;			Number of color vectors used to model normal background color variation at a given pixel.
		int    N2c;			Number of color vectors retained at given pixel.  Must be > N1c, typically ~ 5/3 of N1c.
							Used to allow the first N1c vectors to adapt over time to changing background.

		int    Lcc;			Quantized levels per 'color co-occurrence' component.  Power of two, typically 16, 32 or 64.
		int    N1cc;		Number of color co-occurrence vectors used to model normal background color variation at a given pixel.
		int    N2cc;		Number of color co-occurrence vectors retained at given pixel.  Must be > N1cc, typically ~ 5/3 of N1cc.
							Used to allow the first N1cc vectors to adapt over time to changing background.

		int    is_obj_without_holes;  If TRUE we ignore holes within foreground blobs. Defaults to TRUE.
		int    perform_morphing;	  Number of erode-dilate-erode foreground-blob cleanup iterations.
									  These erase one-pixel junk blobs and merge almost-touching blobs. Default value is 1.

		float  alpha1;		How quickly we forget old background pixel values seen.  Typically set to 0.1
		float  alpha2;		"Controls speed of feature learning". Depends on T. Typical value circa 0.005.
		float  alpha3;		Alternate to alpha2, used (e.g.) for quicker initial convergence. Typical value 0.1.

		float  delta;		Affects color and color co-occurrence quantization, typically set to 2.
		float  T;			"A percentage value which determines when new features can be recognized as new background." 
							(Typically 0.9).
		float  minArea;		Discard foreground blobs whose bounding box is smaller than this threshold.
	}

	Default parameters of foreground detection algorithm:
	#define  CV_BGFG_FGD_LC              128
	#define  CV_BGFG_FGD_N1C             15
	#define  CV_BGFG_FGD_N2C             25

	#define  CV_BGFG_FGD_LCC             64
	#define  CV_BGFG_FGD_N1CC            25
	#define  CV_BGFG_FGD_N2CC            40

	Background reference image update parameter:
	#define  CV_BGFG_FGD_ALPHA_1         0.1f

	stat model update parameter
	0.002f ~ 1K frame(~45sec), 0.005 ~ 18sec (if 25fps and absolutely static BG)

	#define  CV_BGFG_FGD_ALPHA_2         0.005f

	Start value for alpha parameter (to fast initiate statistic model)
	#define  CV_BGFG_FGD_ALPHA_3         0.1f

	#define  CV_BGFG_FGD_DELTA           2

	#define  CV_BGFG_FGD_T               0.9f

	#define  CV_BGFG_FGD_MINAREA         15.f

	#define  CV_BGFG_FGD_BG_UPDATE_TRESH 0.5f	
*/
void CvFGDBackgroundSubtractor::ReadParamsFromUserArguments()
{
	BackgroundSubtractor::ReadParamsFromUserArguments();

	// These constants are defined in cvaux/include/cvaux.h:
	g_params.Lc      = CV_BGFG_FGD_LC;
    g_params.N1c     = CV_BGFG_FGD_N1C;
    g_params.N2c     = CV_BGFG_FGD_N2C;

    g_params.Lcc     = CV_BGFG_FGD_LCC;
    g_params.N1cc    = CV_BGFG_FGD_N1CC;
    g_params.N2cc    = CV_BGFG_FGD_N2CC;

    g_params.delta   = CV_BGFG_FGD_DELTA;

    g_params.alpha1  = CV_BGFG_FGD_ALPHA_1;
    g_params.alpha2  = CV_BGFG_FGD_ALPHA_2;
    g_params.alpha3  = CV_BGFG_FGD_ALPHA_3;

    g_params.T       = CV_BGFG_FGD_T;
    g_params.minArea = CV_BGFG_FGD_MINAREA;

    g_params.is_obj_without_holes = 1;
    g_params.perform_morphing     = 1;
}

void CvFGDBackgroundSubtractor::Clear()
{
	BackgroundSubtractor::Clear();

	if (m_hasBackgroundModel)
	{
		cvReleaseBGStatModel(&m_pBackgroundModel);
		m_hasBackgroundModel = false;
	}
}

void CvFGDBackgroundSubtractor::ProcessBackgroundFrame(RGBImg sample, 
	FloatImg greyImg, fnum_t frameIndex)
{
	IplImageView iplView(sample, false);

	if (!m_pBackgroundModel)
		m_pBackgroundModel = cvCreateFGDStatModel(iplView, &g_params); 
	else
		cvUpdateBGStatModel(iplView, m_pBackgroundModel);
}

/*!
	We discriminate between foreground and background pixels
	by building and maintaining a model of the background.
	Any pixel which does not fit this model is then deemed
	to be foreground.
*/
void CvFGDBackgroundSubtractor::FindForeground(RGBImg newFrame, FloatImg greyImg)
{
	IplImageView iplView(newFrame, false);

	cvUpdateBGStatModel(iplView, m_pBackgroundModel);

	IplImageToVXLImage(m_pBackgroundModel->foreground, m_foreground, 1);
	IplImageToVXLImage(m_pBackgroundModel->background, m_background);
}