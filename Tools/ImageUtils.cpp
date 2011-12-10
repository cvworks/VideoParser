/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ImageUtils.h"

#include <Tools/CvUtils.h> // gets cvConvertImage() function

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h> // gets CvImage class

//#include <Tools/cv_legacy.h>  // gets CvImage class

/*! 
	Convert vxl images to OpenCV images
	The VXL image won't own the memory, so care must be taken if the
	IPL image goes out of scope and the VXL image is accessed later.
	The VXL image knows that is not supposed to free this shared memory,
	so async destruction is not a problem.
*/
void IplImageToVXLImage(IplImage* pIplImg, ByteImg& vxlImg, unsigned int nplanes)
{
	vxlImg.set_to_memory((const vxl_byte*) pIplImg->imageData, 
		pIplImg->width, pIplImg->height, nplanes, nplanes, 
		nplanes * pIplImg->width, 1); // step = 1 always

	// The member function memory_chunk() of the vxl image view returns a
	// smart pointer to the object holding the data for this view. 
	// Since the vxl image doesn't own the memory, there is not such an object.
	// Then, vxlImg.memory_chunk().ptr() == NULL. This can also be tested
	// by calling HasBorrowedMemory(vxlImg) in ImageUtils.h
}

void CvMatToVXLImage(const cv::Mat& mat, ByteImg& vxlImg, unsigned int nplanes)
{
	vxlImg.set_to_memory((const vxl_byte*) mat.data, 
		mat.cols, mat.rows, nplanes, nplanes, 
		nplanes * mat.cols, 1); // step = 1 always

	// The member function memory_chunk() of the vxl image view returns a
	// smart pointer to the object holding the data for this view. 
	// Since the vxl image doesn't own the memory, there is not such an object.
	// Then, vxlImg.memory_chunk().ptr() == NULL. This can also be tested
	// by calling HasBorrowedMemory(vxlImg) in ImageUtils.h
}

void IplImageToVXLImage(IplImage* pIplImg, RGBImg& vxlImg)
{
	ByteImg byteImg;

	IplImageToVXLImage(pIplImg, byteImg, 3);

	vxlImg = byteImg;
}

void CvMatToVXLImage(const cv::Mat& mat, RGBImg& vxlImg)
{
	ByteImg byteImg;

	CvMatToVXLImage(mat, byteImg, 3);

	vxlImg = byteImg;
}

/*!
	Converts the given VXL data to an IPL image wrapped by a CvImage
	object. It reuses the IPL image memory if size, depth, and channels
	do not vary.
*/
void VXLImageToIplImage(char* pVXLImgData, int width, int height, 
		int depth, int channels, CvImage& cvImg)
{
	CvSize sz;

	sz.height = height;
	sz.width = width;

	// Look at the VXL data with IPL eyes by creating a header for it
	IplImage* tempIplImg = cvCreateImageHeader(sz, depth, channels);
	
	tempIplImg->imageData = pVXLImgData;

	// Copy the image in IPL format (ie, with R and B swapped)
	cvImg.create(sz, depth, channels);

	cvConvertImage(tempIplImg, cvImg, CV_CVTIMG_SWAP_RB);

	// We are now done with the temporal header
	cvReleaseImageHeader(&tempIplImg);
}

void VXLImageToIplImage(RGBImg& vxlImg, CvImage& cvImg)
{
	VXLImageToIplImage((char*) vxlImg.top_left_ptr(), vxlImg.ni(), vxlImg.nj(), 
		IPL_DEPTH_8U, 3, cvImg);
}

void VXLImageToIplImage(ByteImg& vxlImg, CvImage& cvImg)
{
	VXLImageToIplImage((char*) vxlImg.top_left_ptr(), vxlImg.ni(), vxlImg.nj(), 
		IPL_DEPTH_8U, 1, cvImg);
}

void VXLImageToIplImage(FloatImg& vxlImg, CvImage& cvImg)
{
	VXLImageToIplImage((char*) vxlImg.top_left_ptr(), vxlImg.ni(), vxlImg.nj(), 
		IPL_DEPTH_32F, 1, cvImg);
}
