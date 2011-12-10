/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CvVideo.h"

#include <Tools/cv.h> // cvResize
#include <Tools/CvUtils.h> // OpenCV

#include <Tools/DirWalker.h>

using namespace vpl;

void CvVideo::Clear()
{
	Video::Clear();

	m_cap.release();
}

/*!
	Loads video and reads its first frame
*/
bool CvVideo::Load(std::string strFilename)
{
	Clear();

	if (!DirWalker::CheckFileExist(strFilename.c_str()))
	{
		ShowOpenFileError(strFilename);
		return false;
	}

	if (!m_cap.open(strFilename))
	{
		ShowStatus1("Cannot load OpenCV video:", strFilename);
		return false;
	}

	//m_cap.set(CV_CAP_PROP_CONVERT_RGB, 1);

	ReadCurrentFrame();
	
	// if there is no first frame, then video isn't loaded
	if (m_curFrame.empty())
	{
		ShowStatus1("Cannot load OpenCV video:", strFilename);
		return false;
	}

	// Set the current frame number
	m_currentFrameNumber = 0;

	// Set the frame count in the video
	int frameCount = (int) m_cap.get(CV_CAP_PROP_FRAME_COUNT);

	double fps = m_cap.get(CV_CAP_PROP_FPS);

	// Set the required video information in the base class
	SetVideoInfo(strFilename, frameCount, 
		DirWalker::ReadCreationTime(strFilename), fps);

	ShowStatus("OpenCV video loaded");

	return true;
}

/*!
	Sets m_curFrame to the current frame in the CvCapture object *m_cap. 
	If there is no available frame, m_curFrame is set to NULL.

	The returned image should not be released or modified by the user.

	Note: m_cap cannot be NULL.
*/
void CvVideo::ReadCurrentFrame()
{
	m_cap >> m_curFrame;

	/*if (!m_curFrame.empty())
	{
		CvSize sz;

		ComputeScalingFactor(m_curFrame.cols, m_curFrame.rows);
		
		if (m_scalingFactor == 1) // there is no scaling to do
		{
			sz.width = m_curFrame.cols;
			sz.height = m_curFrame.rows;
		}
		else // we WILL scale the "input" image
		{
			StreamStatus("The current input frame has been scaled down by " 
				<< m_scalingFactor);

			sz.width = cvRound(m_curFrame.cols * m_scalingFactor);
			sz.height = cvRound(m_curFrame.rows * m_scalingFactor);
		}

		m_curFrame = cvCreateImage(sz, origImg->depth, origImg->nChannels);
	}*/

	/*if (origImg)
	{
		int flipFlag = (origImg->origin != 0) ? CV_CVTIMG_FLIP : 0;

		// Convert the frame to RGB order and flip it if necessary.
		// Also, see if we need to resize the input frame
		if (m_curFrame->width != origImg->width || m_curFrame->height != origImg->height)
		{
			// Use CV_INTER_AREA interpolation, which is the preferred method 
			// for image decimation
			cvResize(origImg, m_curFrame, CV_INTER_AREA);

			// It seems that src and dst can be the same, so we reuse it here
			cvConvertImage(m_curFrame, m_curFrame, flipFlag + CV_CVTIMG_SWAP_RB);
		}
		else
		{
			cvConvertImage(origImg, m_curFrame, flipFlag + CV_CVTIMG_SWAP_RB);
		}
	}*/
}

bool CvVideo::IsLastFrame() const
{
	return m_curFrame.empty();
}

void CvVideo::ReadFirstFrame()
{
	// If the current frame is not already the first frame
	if (m_currentFrameNumber != 0)
	{
		// also see CV_CAP_PROP_AVI_RATIO and use -1 to go to start
		// of the video
		m_cap.set(CV_CAP_PROP_POS_FRAMES, 0);
		ReadCurrentFrame();
		m_currentFrameNumber = 0;
	}
}

void CvVideo::ReadNextFrame()
{
	ASSERT(m_currentFrameNumber != INVALID_FRAME);

	ReadCurrentFrame();

	m_currentFrameNumber++;
}

/*!
	Return a copy of the Mat image in vxl and RGB format.
*/
RGBImg CvVideo::GetCurrentRGBFrame() const
{
	if (m_curFrame.empty())
		return RGBImg();

	ASSERT(m_curFrame.channels() == 3);

	RGBImg curBGRFrame;

	CvMatToVXLImage(m_curFrame, curBGRFrame);

	RGBImg rbgImg(curBGRFrame.ni(), curBGRFrame.nj());

	auto srcIt = curBGRFrame.begin();
	auto tgtIt = rbgImg.begin();

	for (; srcIt != curBGRFrame.end(); ++srcIt, ++tgtIt)
	{
		tgtIt->r = srcIt->b;
		tgtIt->g = srcIt->g;
		tgtIt->b = srcIt->r;
	}

	return rbgImg;
}

/*!
	Converts from linear RGB to CIE luminance assuming a modern monitor.
	
	Weights are: rw=0.2125, double gw=0.7154, double bw=0.0721)
*/
FloatImg CvVideo::GetCurrentGreyScaleFrame() const
{
	if (m_curFrame.empty())
		return FloatImg();

	RGBImg curBGRFrame;

	CvMatToVXLImage(m_curFrame, curBGRFrame);

	FloatImg curGreyFrame;

	// Note that we have a BGR image, and so we must invert the weights
	// of the red and blue channels
	vil_convert_rgb_to_grey(curBGRFrame, curGreyFrame, 0.0721, 0.7154, 0.2125);

	return curGreyFrame;
}

/*!
	Some video formats don't number the frames according to
	a unit interval. This function allows the to report their own
	number for the current frame. 
*/
fnum_t CvVideo::NativeFrameNumber() const
{
	return (fnum_t) Get(CV_CAP_PROP_POS_FRAMES);
}

double CvVideo::FrameRate() const
{
	// Get frame rate
	return Get(CV_CAP_PROP_FPS);
}

int CvVideo::CodecCode() const
{
	// Get 4-character code of codec
	return (int) Get(CV_CAP_PROP_FOURCC);
}

double CvVideo::FramePosition() const
{
	// Get film current position in milliseconds or video capture timestamp
	return Get(CV_CAP_PROP_POS_MSEC);
}

void CvVideo::PrintFrameInfo() const
{
	DBG_PRINT5(
		Get(CV_CAP_PROP_POS_MSEC),
		Get(CV_CAP_PROP_POS_FRAMES),
		Get(CV_CAP_PROP_POS_AVI_RATIO),
		Get(CV_CAP_PROP_FPS),
		Get(CV_CAP_PROP_FRAME_COUNT))
}

void CvVideo::ReadFrame(fnum_t frameIndex)
{
	ShowStatus2("Seeking frame number ", frameIndex, "...");

	if (Set(CV_CAP_PROP_POS_FRAMES, (double)frameIndex))
	{
		ShowStatus("Found the frame quickly!");
		ReadCurrentFrame();
		m_currentFrameNumber = frameIndex;
	}
	else // Use a slow method to seek the requested frame
	{
		// If we are passed the requested frame, go to the beginning
		if (m_currentFrameNumber < 0 || frameIndex < m_currentFrameNumber)
		{
			ReadFirstFrame(); // it also sets m_currentFrameNumber = 0
		}

		// Move to a later frame if necessary
		if (frameIndex > m_currentFrameNumber)
		{
			// Note that m_cap points to the *next* frame. So, eg, if 
			// frameIndex == m_currentFrameNumber + 1. Then, we don't
			// need to grab any intermediate frames.
			for (; m_currentFrameNumber + 1 < frameIndex; m_currentFrameNumber++)
			{
				if (!m_cap.grab())
				{
					WARNING(true, "The frame number does not exist");
					break;
				}
			}

			// Reads the current frame in m_cap
			ReadNextFrame();
		}
	}
}

