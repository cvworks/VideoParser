/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include "Video.h"
#include <Tools/cv.h>
#include <Tools/CvUtils.h>

namespace vpl {

/*!
	Wrapper for an OpenCV Video 
*/
class CvVideo : public Video
{
	cv::VideoCapture m_cap;
	cv::Mat  m_curFrame;

	inline void ReadCurrentFrame();

	double Get(int prop) const
	{
		return const_cast<cv::VideoCapture*>(&m_cap)->get(prop);
	}

	bool Set(int prop, double val)
	{
		return m_cap.set(prop, val);
	}

public:
	
	void Clear();

	bool Load(std::string strFilename);
	void ReadFirstFrame();
	void ReadNextFrame();
	void ReadFrame(fnum_t i);

	virtual fnum_t NativeFrameNumber() const;

	double FramePosition() const;

	double FrameRate() const;
	int CodecCode() const;
	
	bool IsLastFrame() const;
	RGBImg GetCurrentRGBFrame() const;
	FloatImg GetCurrentGreyScaleFrame() const;

	void PrintFrameInfo() const;
};

} // namespace vpl


