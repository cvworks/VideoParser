/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CvPeopleDetector.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <Tools/UserArguments.h>
#include <VideoParser/ImageProcessor.h>

using namespace vpl;
using namespace cv;

extern UserArguments g_userArgs;

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void CvPeopleDetector::ReadParamsFromUserArguments()
{
	PeopleDetector::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "windowSize", Tokenize("small medium large"),
		"Type of window used for detecting people", 0, &m_params.type);

	g_userArgs.ReadArg(Name(), "hitThreshold", 
		"To get a higher hit-rate (and more false alarms, respectively)"
		" decrease the hitThreshold", 0.0, &m_params.hitThreshold);

	g_userArgs.ReadArg(Name(), "groupThreshold", 
		"To get a higher hit-rate (and more false alarms, respectively)"
		" decrease the groupThreshold (0 turns off the grouping)", 
		2, &m_params.groupThreshold);

	g_userArgs.ReadArg(Name(), "scaleFactor", 
		"Specifies how much the image size is reduced at each image scale", 
		1.05, &m_params.scaleFactor);
}

void CvPeopleDetector::Initialize(graph::node v)
{
	PeopleDetector::Initialize(v);
	
}

void CvPeopleDetector::Run()
{
	HOGDescriptor hog;

    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

	// Get the new frame
	m_img = m_pImgProcessor->GetRGBImage();

	// Clear the previously detected people
	m_found_filtered.clear();
		
	std::vector<Rect> found;

	cv::Size winStride(8, 8);

	if (m_params.type == Params::SMALL_WIN)
	{
		winStride.width = 8;
		winStride.height = 8;
	}
	else if (m_params.type == Params::MEDIUM_WIN)
	{
		winStride.width = 16;
		winStride.height = 16;
	}
	else if (m_params.type == Params::LARGE_WIN)
	{
		winStride.width = 32;
		winStride.height = 32;
	}

	hog.detectMultiScale(m_img, found, m_params.hitThreshold, 
			winStride, Size(32,32), m_params.scaleFactor, m_params.groupThreshold);

	//hog.detectMultiScale(m_img, found, 0, Size(8,8), Size(32,32), 1.05, 2);

	size_t i, j;

	for( i = 0; i < found.size(); i++ )
	{
		Rect r = found[i];

		for( j = 0; j < found.size(); j++ )
			if( j != i && (r & found[j]) == r)
				break;

		if( j == found.size() )
			m_found_filtered.push_back(r);
	}
}

void CvPeopleDetector::GetParameterInfo(int i, DoubleArray* pMinVals, 
	DoubleArray* pMaxVals, DoubleArray* pSteps) const
{
	InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 0, 1);
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void CvPeopleDetector::Draw(const DisplayInfoIn& dii) const
{
	
}

/*!	
	Returns the basic information specifying the output of this component.
	It must provide an image, its type, and a text message. All of this parameters 
	are optional. For example, if there is no output image, the image type
	can be set to VOID_IMAGE.
*/
void CvPeopleDetector::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	if (m_img.empty())
		return;

	RGBImg img;

	img.deep_copy(m_img.ImgPtr());

	CvMatView mat(img);

	for(unsigned i = 0; i < m_found_filtered.size(); i++ )
	{
		Rect r = m_found_filtered[i];

		// the HOG detector returns slightly larger rectangles than the real objects.
		// so we slightly shrink the rectangles to get a nicer output.
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);

		cv::rectangle(mat, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
	}

	dio.imageType = RGB_IMAGE;
	dio.imagePtr = ConvertToBaseImgPtr(img);
}

