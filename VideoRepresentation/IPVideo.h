/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include "Video.h"

struct CvCapture;

namespace vpl {

/*!
	Wrapper for an OpenCV Video 
*/
class IPVideo : public Video
{
	CvCapture* m_cap;
	IplImage*  m_curFrame;

	inline void ReadCurrentFrame();

public:
	IPVideo()
	{
		m_cap = NULL;
		m_curFrame = NULL;
	}

	~IPVideo();

	void Clear();

	bool Load(std::string strFilename);
	void ReadFirstFrame();
	void ReadNextFrame();
	void ReadFrame(fnum_t i);

	double FramePosition() const;

	double FrameRate() const;
	int CodecCode() const;
	
	bool IsLastFrame() const;
	RGBImg GetCurrentRGBFrame() const;
	FloatImg GetCurrentGreyScaleFrame() const;

	void PrintFrameInfo() const;
};

} // namespace vpl

