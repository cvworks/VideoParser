/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoWriter.h"

#include <Tools/CvUtils.h> // OpenCV

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h>  // gets CvImage class

using namespace vpl;

void VideoWriter::Open(const std::string& filename, 
					   int width, int height,
					   double fps, int fourcc)
{
	cv::Size frameSize(width, height);

	fps = 15;

	fourcc = CV_FOURCC_DEFAULT; //CV_FOURCC('P', 'I', 'M', '2');

	//DBG_PRINT5(filename, fourcc, fps, frameSize.width, frameSize.height);

	m_writer = cvCreateVideoWriter(filename.c_str(), fourcc, fps, frameSize);
}

void VideoWriter::Close()
{
	cvReleaseVideoWriter(&m_writer);
	m_writer = NULL;
}

int VideoWriter::WriteFrame(RGBImg vxlImg)
{
	CvImage cvImg;

	VXLImageToIplImage(vxlImg, cvImg);

	return cvWriteFrame(m_writer, cvImg);
}

