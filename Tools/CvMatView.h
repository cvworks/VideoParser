/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "BasicUtils.h"
#include <Tools/cv.h>
#include "ImageUtils.h"

inline void DrawCenteredText(cv::Mat img, const std::string& text, const cv::Point& center, 
	int fontFace, double fontScale, cv::Scalar color, int thickness = 1)
{
	cv::Point text_placement;

    int ymin;

	cv::Size textSize = cv::getTextSize(text, fontFace, 
		fontScale, thickness, &ymin);

    text_placement.x = int(center.x - 0.5 * textSize.width);
    text_placement.y = int(center.y + 0.5 * textSize.height);

	cv::putText(img, text, text_placement, fontFace, 
		fontScale, color, thickness);
}

/*!
	Wrapper for an IplImage whose data is owned by a VXL image.

	It is similar to the CvImage class declared in cxcore.hpp. However,
	CvImage counts references to the wrapped IplImage, while ShallowIplImage
	is just a IplImage* and assumes that the *imageData is ownd by some
	VXL image. A ShallowIplImage object is only valid while the VXL image
	does not change.
*/
class CvMatView : public cv::Mat
{
protected:
	BaseImgPtr m_vxlView;  //! Needed to ensure that Mat data is valid

public:
	BaseImgPtr ImgPtr() const
	{
		return m_vxlView;
	}

	void operator=(const CvMatView& rhs)
	{
		// shallow copy of matrix
		cv::Mat::operator=(rhs);

		// shallow copy of data
		m_vxlView = rhs.m_vxlView;
	}

	CvMatView()
	{
	}

	CvMatView(FloatImg& img) : cv::Mat(cv::Size(img.ni(), img.nj()), 
		CV_32FC1, (void*) img.top_left_ptr())
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);
	}

	CvMatView(ByteImg& img) : cv::Mat(cv::Size(img.ni(), img.nj()), 
		CV_8UC1, (void*) img.top_left_ptr())
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);
	}

	CvMatView(IntImg& img) : cv::Mat(cv::Size(img.ni(), img.nj()), 
		CV_32SC1, (void*) img.top_left_ptr())
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);
	}

	CvMatView(RGBImg& img) : cv::Mat(cv::Size(img.ni(), img.nj()), 
		CV_8UC3, (void*) img.top_left_ptr())
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);
	}

	CvMatView(LabImg& img) : cv::Mat(cv::Size(img.ni(), img.nj()), 
		CV_32FC3, (void*) img.top_left_ptr())
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		ASSERT(img.jstep() > 0);
	}

	template<typename T> void deep_copy(const T& srcImg)
	{
		T aux;
		
		aux.deep_copy(srcImg);

		*this = aux;
	}

	/*template<typename T>
	void deep_copy(const vil_image_view<T>& src)
	{
		decltype(src) img;

		img.deep_copy(src);

		CvMatView aux(img);

		*this = aux;
	}*/
};

