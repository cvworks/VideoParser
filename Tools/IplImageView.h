/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IPL_IMAGE_VIEW_H_
#define _IPL_IMAGE_VIEW_H_

#include "BasicUtils.h"
#include <Tools/cv.h>
#include "ImageUtils.h"

/*!
	Wrapper for an IplImage whose data is owned by a VXL image.

	It is similar to the CvImage class declared in cxcore.hpp. However,
	CvImage counts references to the wrapped IplImage, while ShallowIplImage
	is just a IplImage* and assumes that the *imageData is ownd by some
	VXL image. A ShallowIplImage object is only valid while the VXL image
	does not change.
*/
class IplImageView
{
protected:
	IplImage* m_pIplImage; //! It is valid as long as m_vxlView is
	BaseImgPtr m_vxlView;  //! Needed to ensure that IPL image is valid

	void CreateHeader(int width, int height, int depth, int channels,
		char* imageData)
	{
		CvSize sz;
		
		sz.height = height;
		sz.width = width;

		m_pIplImage = cvCreateImageHeader(sz, depth, channels);

		m_pIplImage->imageData = imageData;
	}

public:
	IplImageView() { m_pIplImage = NULL; }

	IplImageView(FloatImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32F, 1,
			(char*) img.top_left_ptr());
	}

	IplImageView(ByteImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_8U, 1,
			(char*) img.top_left_ptr());
	}

	IplImageView(IntImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32S, 1,
			(char*) img.top_left_ptr());
	}

	IplImageView(RGBImg& rgbImg, bool swapRB);

	IplImageView(LabImg& img)
	{
		m_vxlView = ConvertToBaseImgPtr(img);

		CreateHeader(img.ni(), img.nj(), IPL_DEPTH_32F, 3,
			(char*) img.top_left_ptr());
	}

	~IplImageView() { cvReleaseImageHeader(&m_pIplImage); }

	operator const IplImage*() const { return m_pIplImage; }

    operator IplImage*() { return m_pIplImage; }
};

#endif // _IPL_IMAGE_VIEW_H_