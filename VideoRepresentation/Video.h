/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/InputImageInfo.h>

namespace vpl {

class Video
{
	friend class MultiVideo;

protected:
	fnum_t m_currentFrameNumber;  //!< Current frame number
	fnum_t m_frameCount;          //!< Number of frames in the video
	bool m_bFlipFrames;           //!< Forces the upside-down fliping of frames
	double m_scalingFactor;       //!< Image scaling factor. The default value is 1 (no scaling)
	std::string m_filename;       //!< File path used to load the video
	time_t m_startTime;           //!< Start of video recording (in seconds from 1/1/1970)
	double m_frameRate;           //!< Fixed frame-per-second value. Can be made valiable by overloading FrameRate()

	ROISequence m_roiSeq;          //!< Array of user defined regions of interest

	static double s_maxWidth;
	static double s_maxHeight;

protected:
	void ComputeScalingFactor(int width, int height)
	{
		double s_w = (s_maxWidth > 0 && width > s_maxWidth) 
			? s_maxWidth / width : 1;

		double s_h = (s_maxHeight > 0 && height > s_maxHeight) 
			? s_maxHeight / height : 1;

		m_scalingFactor = MIN(s_w, s_h);
	}

	/*!
		Helper function to set all the information that is required after 
		loading a video. It sets the following member variables:

		- m_filename = strFilename
		- m_frameCount = number of frames in the video
		- the video metadata (using the file name)

		Note: There are optional parameters, such as the ROI array, which
		are set using their own set functions.
	*/
	void SetVideoInfo(const std::string& strFilename, fnum_t frameCount, 
		time_t startTime, const double& fps = 5.0)
	{
		m_filename = strFilename;
		m_frameCount = frameCount;
		m_startTime = startTime;
		m_frameRate = fps;
	}

	//! Updates the timestamp initialized with SetVideoInfo()
	void SetTimestamp(time_t startTime)
	{
		m_startTime = startTime;
	}

public:
	enum { INVALID_FRAME = -1 };
	
	Video()  
	{ 
		m_currentFrameNumber = INVALID_FRAME; 
		m_frameCount = 0; 
		m_bFlipFrames = false; 
		m_scalingFactor = 1;
		m_startTime = 0;
		m_frameRate = 1;
	}

	void SetROISequence(const ROISequence& rs)
	{
		m_roiSeq = rs;
	}

	const ROISequence& GetROISequence() const
	{
		return m_roiSeq;
	}

	static void SetMaxFrameSize(const double& w, const double& h)
	{
		s_maxWidth = w;
		s_maxHeight = h;
	}

	virtual ~Video() { }

	virtual void Clear() 
	{ 
		m_currentFrameNumber = INVALID_FRAME; 
		m_frameCount = 0; 
		m_bFlipFrames = false; 
		m_scalingFactor = 1;
		m_startTime = 0;
		m_frameRate = 1;

		m_filename.clear();
	}

	time_t StartTime() const
	{
		return m_startTime;
	}

	virtual time_t CurrentFrameTime() const
	{
		if (StartTime() <= 0 || m_currentFrameNumber < 0)
			return 0;
		
		double d = m_currentFrameNumber / FrameRate();

		return StartTime() + int(d);
	}

	void SetScalingFactor(const double& s)
	{
		// Check that the scaling looks reasonable
		ASSERT(s > 0 && s < 10);

		m_scalingFactor = s;
	}

	const double& GetScalingFactor() const
	{
		return m_scalingFactor;
	}
	
	//!	Returns the file path used to load the video
	const std::string& Filename() const
	{
		return m_filename;
	}

	//!	By default, it simply returns the file path used to load the video
	virtual const std::string& FrameInfo() const
	{
		return m_filename;
	}

	virtual fnum_t FrameNumber() const { return m_currentFrameNumber; }
	virtual fnum_t FrameCount() const  { return m_frameCount; }

	/*!
		Some video formats don't number the frames according to
		a unit interval. This function allows the to report their own
		number for the current frame. 

		By default, it is equal to FrameNumber().
	*/
	virtual fnum_t NativeFrameNumber() const
	{
		return FrameNumber();
	}

	//! Film current position in milliseconds or video capture timestamp
	virtual double FramePosition() const 
	{
		// By default, use frame number, but let the derived classes
		// return the real timestamp of the frame if available
		return m_currentFrameNumber * 1000 / FrameRate(); 
	}

	//! Number of frames per second
	virtual double FrameRate() const { return m_frameRate; }

	//! Codec id
	virtual int CodecCode() const { return 0; }

	virtual bool Stream() { return false; }
	
	/*!
		The Load() function in a derived class must:

		- let m_filename = strFilename
		- set m_frameCount = number of frames in the video
		- set the video metadata

		All this can be accomplished by calling SetVideoInfo() after the
		video is loaded.
	*/
	virtual bool Load(std::string strFilename) = 0;

	virtual void ReadFirstFrame() = 0;
	virtual void ReadNextFrame() = 0;

	/*! 
		Reads the video frame with number n. 

		It also makes the frame the current	frame, so that a 
		following ReadNextFrame() operation returns the frame i + 1.
	*/
	virtual void ReadFrame(fnum_t n) = 0;

	/*! 
		Should be removed. It seems to be used only to give a choice to
		the derived classes about effcient vs inefficient methods. I'm not sure.
		It is only used in CvVideo.
	*/
	/*virtual void ReadIndexedFrame(unsigned int frameIndex)
	{
		ReadFrame((int)frameIndex);
	}*/
	
	virtual bool IsLastFrame() const = 0;
	virtual RGBImg GetCurrentRGBFrame() const = 0;
	virtual FloatImg GetCurrentGreyScaleFrame() const = 0;

	virtual void PrintFrameInfo() const
	{
	}

	//! Converts a time 't' to a frame number used the video timestamp.
	virtual fnum_t TimeToFrame(time_t t) const
	{
		ASSERT(m_startTime > 0);

		time_t diff = t - m_startTime;

		if (diff >= 0)
		{
			return fnum_t(diff * FrameRate());
		}
		else
		{
			WARNING(true, "Time is ealier than the start of the video");

			return 0;
		}
	}

	//! Gets the info associated with the current video frame
	void GetCurrentFrameInfo(InputImageInfo& imgInfo) const
	{
		imgInfo.rgbFrame  = GetCurrentRGBFrame();
		imgInfo.greyFrame = GetCurrentGreyScaleFrame();

		imgInfo.frameNumber = FrameNumber();
		imgInfo.framePos    = FramePosition();
	
		imgInfo.frameInfo = FrameInfo();
		imgInfo.timestamp = CurrentFrameTime();
		imgInfo.roiSequence  = GetROISequence();
	}

	//! Reads the frame at a given time wrt the video timestamp
	void ReadFrameAtTime(time_t t)
	{
		ReadFrame(TimeToFrame(t));
	}
};

} // namespace vpl


