/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include "MatrixUtils.h"
#include "BasicUtils.h"

//#include <cxtypes.h> to declare class CV_EXPORTS CvImage;

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "CIELabColor.h"

// @TODO: undef Show "Error" and use this:
//vgui_error_dialog("Failed to save movie");
#include <vil/vil_image_view.h>
#include <vil/vil_convert.h>
#include <vil/vil_new.h> // gets vil_new_image_view_base_sptr()
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_math.h>

// Simple image size struct to avoid declaring the CvSize
struct ImgSize
{
	int width;
    int height;

	void Set(int w, int h) { width = w; height = h; }
};

// We use VXL images, but sometimes may construct them from
// an OpenCV image. So we declare the OpenCV image type here
struct _IplImage;
typedef _IplImage IplImage;

namespace cv {
	class Mat;
}

typedef vil_rgb<float> LabColor;

// Define handy typedefs for common image types
typedef vil_image_view_base_sptr BaseImgPtr;

typedef vil_image_view<bool> BoolImg;
typedef vil_image_view< vxl_byte > ByteImg;
typedef vil_image_view< vxl_sbyte > CharImg;
typedef vil_image_view<int> IntImg;
typedef vil_image_view<float> FloatImg;
typedef vil_image_view< vil_rgb<vxl_byte> > RGBImg;
typedef vil_image_view< LabColor > LabImg;

typedef vil_rgb<vxl_byte> RGBColor;

typedef vil_rgba<vxl_byte> RGBAColor;

enum ImageType {VOID_IMAGE, BOOL_IMAGE, BYTE_IMAGE, CHAR_IMAGE, 
	INT_IMAGE, FLOAT_IMAGE, RGB_IMAGE};

typedef std::vector<ByteImg> ByteImgArray;
typedef std::vector<RGBImg> RGBImgArray;
typedef std::vector<FloatImg> FloatImgArray;
typedef std::vector<IntImg> IntImgArray;

#define rgb2ind(C) (C.R() * (255 * 255) + C.G() * 255 + C.B())
#define ind2rgb(I) (vil_rgb<vxl_byte>(I / (255 * 255), (I % (255 * 255)) / 255, (I % (255 * 255)) % 255))

#define ConvertToGreyImage(I) vil_convert_cast(float(), vil_convert_to_grey_using_average(I))

//! Converts the given image view to a BaseImgPtr
#define ConvertToBaseImgPtr(I) vil_new_image_view_base_sptr(I)

///////////////////////////////////////////////////////////////////////////
// Functions that convert IPL images to VXL images

void IplImageToVXLImage(IplImage* pIplImg, ByteImg& vxlImg, unsigned int nplanes);

void IplImageToVXLImage(IplImage* pIplImg, RGBImg& vxlImg);

void CvMatToVXLImage(const cv::Mat& mat, ByteImg& vxlImg, unsigned int nplanes);

void CvMatToVXLImage(const cv::Mat& mat, RGBImg& vxlImg);

///////////////////////////////////////////////////////////////////////////
// Functions that convert VXL images to IPL images (wrapped by CvImage)

class CvImage;

void VXLImageToIplImage(char* pVXLImgData, int width, int height, 
						int depth, int channels, CvImage& cvImg);

void VXLImageToIplImage(RGBImg& vxlImg, CvImage& cvImg);
void VXLImageToIplImage(ByteImg& vxlImg, CvImage& cvImg);
void VXLImageToIplImage(FloatImg& vxlImg, CvImage& cvImg);

///////////////////////////////////////////////////////////////////////////
// Functions that should be part of vil_image_view<bool> but aren't

//! Returns true of the view looks at `third-party' data
template<class T> bool HasBorrowedMemory(const vil_image_view<bool>& img)
{
	// The function vil_image_view::memory_chunk() returns a
	// smart pointer to the object holding the data for the view. 
	// If the viewlooks at `third-party' data, there is not such an object.
	return (vxlImg.memory_chunk().ptr() == NULL);
}

///////////////////////////////////////////////////////////////////////////
// Random color functions

inline unsigned char RandomColor() 
{ 
	return (unsigned char)(rand() / (RAND_MAX + 1.0) * 255.0); 
}

inline RGBColor RandomRGBColor()
{ 
	return RGBColor(RandomColor(), RandomColor(), RandomColor());
}

///////////////////////////////////////////////////////////////////////////
// Handy functions



///////////////////////////////////////////////////////////////////////////
// Declare handy template functions. NOTE: Now using MatrixUtils.h

//! Element-wise multiplication
/*template<class T> vnl_matrix<T> emul(const vnl_matrix<T>& a, const vnl_matrix<T>& b)
{
	vnl_matrix<T> lhs(a.rows(), a.cols());
	
	for (unsigned int i = 0; i < a.rows(); i++)
		for (unsigned int j = 0; j < a.cols(); j++)
			lhs(i,j) = a(i,j) * b(i,j);
			
	return lhs;
}

//! Element-wise division
template<class T> vnl_matrix<T> ediv(const vnl_matrix<T>& a, const vnl_matrix<T>& b)
{
	vnl_matrix<T> lhs(a.rows(), a.cols());
	
	for (unsigned int i = 0; i < a.rows(); i++)
		for (unsigned int j = 0; j < a.cols(); j++)
			lhs(i,j) = a(i,j) / b(i,j);
			
	return lhs;
}

template<class T> vnl_matrix<T> sign(const vnl_matrix<T>& rhs)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = (rhs(i,j) > 0) ? 1:((rhs(i,j) < 0) ? -1:0);
			
	return lhs;
}

template<class T> vnl_matrix<T> absval(const vnl_matrix<T>& rhs)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = fabs(rhs(i,j));
			
	return lhs;
}

template<class T> vnl_matrix<T> sqrt(const vnl_matrix<T>& rhs)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = sqrt(rhs(i,j));
			
	return lhs;
}

template<class T> vnl_matrix<T> sin(const vnl_matrix<T>& rhs)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = std::sin(rhs(i,j));
			
	return lhs;
}

template<class T> vnl_matrix<T> cos(const vnl_matrix<T>& rhs)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = std::cos(rhs(i,j));
			
	return lhs;
}

template<class T> vnl_matrix<T> pow(const vnl_matrix<T>& rhs, double e)
{
	vnl_matrix<T> lhs(rhs.rows(), rhs.cols());
	
	for (unsigned int i = 0; i < rhs.rows(); i++)
		for (unsigned int j = 0; j < rhs.cols(); j++)
			lhs(i,j) = pow(rhs(i,j), e);
			
	return lhs;
}*/


