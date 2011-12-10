/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _CV_FGD_BACKGROUND_SUBTRACTOR_H_
#define _CV_FGD_BACKGROUND_SUBTRACTOR_H_

#include "../BackgroundSubtractor.h"

struct CvBGStatModel;

namespace vpl {

/*!
	We discriminate between foreground and background pixels
	by building and maintaining a model of the background.
	Any pixel which does not fit this model is then deemed
	to be foreground.

	The algorithm implemented is the latest and greatest algorithm
	in OpenCv, described in

	Foreground Object Detection from Videos Containing Complex Background.
	Liyuan Li, Weimin Huang, Irene Y.H. Gu, and Qi Tian. 
	ACM MM2003 9p
*/
class CvFGDBackgroundSubtractor : public BackgroundSubtractor
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
	CvFGDBackgroundSubtractor()
	{
		m_pBackgroundModel = NULL;
	}

	void ReadParamsFromUserArguments();

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "CvFGDBackgroundSubtractor";
	}
};

} // namespace vpl

#endif //_CV_FGD_BACKGROUND_SUBTRACTOR_H_