/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IMG_SEQ_VIDEO_H_
#define _IMG_SEQ_VIDEO_H_

#include "Video.h"
#include <Tools/UserArguments.h>
#include <Tools/STLUtils.h>

namespace vpl {

class ImgSeqVideo : public Video
{
	UserArguments m_params;
	BaseImgPtr m_curImg;
	unsigned m_frameWidth; //<! Used to ensure that all frames have the same size
	unsigned m_frameHeight;//<! Used to ensure that all frames have the same size
	double m_frameRate; 

	bool m_showScalingWarning;
	bool m_enforceSize;
	unsigned m_marginSize;
	RGBColor m_marginColor;

	StrList m_framePaths; //!< Sorted file paths to each video frame
	StrList::const_iterator m_pathIt; //!< Iterator pointing to the current frame path

protected:
	void ReadCurrentImageFile();

	template <typename T> void EnsureFrameSize(T& img) const;

public:
	void Clear() 
	{
		Video::Clear();

		m_frameWidth = 0;
		m_frameHeight = 0;

		m_framePaths.clear();
	}

	/*!	
		Returns the file path used to load the current frame if valid, 
		or the path to load the video otherwise.
	*/
	virtual const std::string& FrameInfo() const
	{
		return (!IsLastFrame()) ? *m_pathIt : m_filename;
	}

	virtual double FrameRate() const
	{ 
		return m_frameRate; 
	}

	bool Load(std::string strFilename);
	void ReadFirstFrame();
	void ReadNextFrame();
	void ReadFrame(fnum_t i);
	
	bool IsLastFrame() const;
	RGBImg GetCurrentRGBFrame() const;
	FloatImg GetCurrentGreyScaleFrame() const;
};

} // namespace vpl

#endif //_IMG_SEQ_VIDEO_H_