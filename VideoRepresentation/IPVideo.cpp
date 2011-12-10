/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "IPVideo.h"

#include <Tools/cv.h> // cvResize
#include <Tools/CvUtils.h> // OpenCV

#include <Tools/DirWalker.h>

using namespace vpl;

IPVideo::~IPVideo()
{
	if (m_cap)
		cvReleaseCapture(&m_cap);

	//if (m_curFrame)
	//	cvReleaseImage(m_curFrame);
}

void IPVideo::Clear()
{
	Video::Clear();

	if (m_cap)
	{
		cvReleaseCapture(&m_cap);
		m_cap = NULL;
	}

	if (m_curFrame)
	{
		cvReleaseImage(&m_curFrame);
		ASSERT(!m_curFrame);
		//m_curFrame = NULL;
	}
}

/*!
	Sets m_curFrame to the current frame in the CvCapture object *m_cap. 
	If there is no available frame, m_curFrame is set to NULL.

	The returned image should not be released or modified by the user.

	Note: m_cap cannot be NULL.
*/
void IPVideo::ReadCurrentFrame()
{
	// Note about cvQueryFrame: The returned image should not be released 
	// or modified by the user. In the event of an error, the return 
	// value may be NULL.


	/*m_curFrame = cvQueryFrame(m_cap);

	if (m_curFrame)
	{
		int flipFlag = (m_curFrame->origin != 0) ? CV_CVTIMG_FLIP : 0;
		cvConvertImage(m_curFrame, m_curFrame, flipFlag + CV_CVTIMG_SWAP_RB);
	}*/

	IplImage* origImg = cvQueryFrame(m_cap);

	if (!origImg) // we couln't read an image
	{
		cvReleaseImage(&m_curFrame);
		ASSERT(!m_curFrame);
		return;
	}

	// If necessary, create an image to hold a copy of the video frame
	// ie, if the ptr to the image is null
	if (!m_curFrame)
	{
		CvSize sz;

		ComputeScalingFactor(origImg->width, origImg->height);

		if (m_scalingFactor == 1) // there is no scaling to do
		{
			sz.width = origImg->width;
			sz.height = origImg->height;
		}
		else // we WILL scale the "input" image
		{
			StreamStatus("The current input frame has been scaled down by " 
				<< m_scalingFactor);

			sz.width = cvRound(origImg->width * m_scalingFactor);
			sz.height = cvRound(origImg->height * m_scalingFactor);
		}

		m_curFrame = cvCreateImage(sz, origImg->depth, origImg->nChannels);
	}

	if (origImg)
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
	}
}

/*!
	Loads video and reads its first frame
*/
bool IPVideo::Load(std::string strFilename)
{
	Clear();

	if (!DirWalker::CheckFileExist(strFilename.c_str()))
	{
		ShowOpenFileError(strFilename);
		return false;
	}
	
	m_cap = cvCreateFileCapture("visualcortek-17.site.uottawa.ca");
	//m_cap = cvCaptureFromFile(strFilename.c_str());
	
	// Read the first frame
	ReadCurrentFrame();
	
	// if there is no first frame, then video isn't loaded
	if (!m_curFrame)
	{
		ShowStatus1("Cannot load OpenCV video:", strFilename);
		return false;
	}

	// Set the current frame number
	m_currentFrameNumber = (fnum_t) cvGetCaptureProperty(m_cap, 
		CV_CAP_PROP_POS_FRAMES);

	if (m_currentFrameNumber < 0)
		m_currentFrameNumber = 0;

	// Set the frame count in the video
	int frameCount = (int) cvGetCaptureProperty(m_cap, 
		CV_CAP_PROP_FRAME_COUNT);

	// Set the required video information in the base class
	SetVideoInfo(strFilename, frameCount,
		DirWalker::ReadCreationTime(strFilename));

	ShowStatus("OpenCV video loaded");

	//DBG_PRINT5(m_curFrame->depth, m_curFrame->dataOrder, m_curFrame->nChannels,
	//	m_curFrame->origin, m_curFrame->align)

	return true;
}

bool IPVideo::IsLastFrame() const
{
	return (m_curFrame == NULL);
}

void IPVideo::ReadFirstFrame()
{
	ASSERT(m_currentFrameNumber != INVALID_FRAME);

	// If the current frame is not already the first frame
	if (m_currentFrameNumber > 0)
	{
		cvSetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES, 0);
		ReadCurrentFrame();
		m_currentFrameNumber = 0;
	}
}

void IPVideo::ReadNextFrame()
{
	ASSERT(m_currentFrameNumber != INVALID_FRAME);

	ReadCurrentFrame();

	// Get 0-based index of the frame to be decoded/captured next
	m_currentFrameNumber = (int) cvGetCaptureProperty(m_cap, 
		CV_CAP_PROP_POS_FRAMES);
}

RGBImg IPVideo::GetCurrentRGBFrame() const
{
	if (!m_curFrame)
		return RGBImg();

	ASSERT(m_curFrame->nChannels == 3);

	RGBImg curRGBFrame;

	IplImageToVXLImage(m_curFrame, curRGBFrame);

	// We return a copy of the image, so that we can
	// reuse the ipl image storage to read a new image while
	// the previous image is still in use.
	return vil_copy_deep(curRGBFrame);
}

FloatImg IPVideo::GetCurrentGreyScaleFrame() const
{
	if (!m_curFrame)
		return FloatImg();

	vil_image_view<vxl_byte> vxlImg;

	IplImageToVXLImage(m_curFrame, vxlImg, 3);

	FloatImg curGreyFrame;

	// Copy and convert to gray
	vil_convert_planes_to_grey(vxlImg, curGreyFrame);

	return curGreyFrame;
}

double IPVideo::FrameRate() const
{
	// Get frame rate
	return cvGetCaptureProperty(m_cap, CV_CAP_PROP_FPS);
}

int IPVideo::CodecCode() const
{
	// Get 4-character code of codec
	return (int) cvGetCaptureProperty(m_cap, CV_CAP_PROP_FOURCC);
}

double IPVideo::FramePosition() const
{
	// Get film current position in milliseconds or video capture timestamp
	return cvGetCaptureProperty(m_cap, CV_CAP_PROP_POS_MSEC);
}

void IPVideo::PrintFrameInfo() const
{
	DBG_PRINT5(
		cvGetCaptureProperty(m_cap, CV_CAP_PROP_POS_MSEC),
		cvGetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES),
		cvGetCaptureProperty(m_cap, CV_CAP_PROP_POS_AVI_RATIO),
		cvGetCaptureProperty(m_cap, CV_CAP_PROP_FPS),
		cvGetCaptureProperty(m_cap, CV_CAP_PROP_FRAME_COUNT))
}

void IPVideo::ReadFrame(fnum_t i)
{
	ASSERT(false);
	ASSERT(m_currentFrameNumber != INVALID_FRAME);

	// NOTE: must always go to the requested frame, even if 
	// m_currentFrameNumber is equal to i, because m_currentFrameNumber may not be correct

	cvSetCaptureProperty(m_cap, CV_CAP_PROP_POS_FRAMES, (double)i);
	ReadCurrentFrame();
	m_currentFrameNumber = i;
}
