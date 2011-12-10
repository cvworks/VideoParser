/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _CV_GAUSS_BACKGROUND_SUBTRACTOR_H_
#define _CV_GAUSS_BACKGROUND_SUBTRACTOR_H_

#include "../BackgroundSubtractor.h"

struct CvBGStatModel;

namespace vpl {

/*!
	We discriminate between foreground and background pixels
	by building and maintaining a model of the background.
	Any pixel which does not fit this model is then deemed
	to be foreground.

	The algorithm implemented is the "Mixture of Gaussians", which
	is an older algorithm, described in 
	
	"Moving target classification and tracking from real-time video". 
	A Lipton, H Fujijoshi, R Patil.	Proceedings IEEE Workshop on 
	Application of Computer Vision pp 8-14 1998

	Learning patterns of activity using real-time tracking
	C Stauffer and W Grimson  August 2000
	IEEE Transactions on Pattern Analysis and Machine Intelligence 22(8):747-757
*/
class CvGaussBackgroundSubtractor : public BackgroundSubtractor
{
protected:
	CvBGStatModel* m_pBackgroundModel;

protected:
	virtual void InitializeBackgroundModel(unsigned width, unsigned height)
	{
		// nothing to do
	}

	virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex);
	virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg);

	virtual void ComputeForegroundStats(RGBImg rgbImage, FloatImg grayImage)
	{
		ASSERT(false);
	}

public:
	CvGaussBackgroundSubtractor()
	{
		m_pBackgroundModel = NULL;
	}

	void ReadParamsFromUserArguments();

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "CvGaussBackgroundSubtractor";
	}
};

} // namespace vpl

#endif //_CV_GAUSS_BACKGROUND_SUBTRACTOR_H_